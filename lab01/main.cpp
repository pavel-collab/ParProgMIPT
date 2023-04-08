#include <iostream>
#include <cmath>
#include <cstring>
#include "mpi.h"

#define DEBUG
#define pi 3.14159265

// Функция, задающая ДУ
double f(double t, double x) {
    return x + t;
}
// Функция, задающая начальное условие на границе t = 0
double phi(double x) {
    return cos(pi*x);
}
// Функция, задающая начальное условие на границе x = 0
double psi(double t) {
    return exp(-t);
}

// Функция возвращает
// k -- первый индекс двумерного массива (номер строки)
// m -- второй индекс двумерного массива (номер столбца)
// M -- количество столбцов в двумерном массиве
// a[k][m]
int GetIdx(int k, int m, int M) {
    return k*M + m;
}

// Функция заполняет значения функции в узлах сетки
// f -- указатель на функцию ДУ
// f_arr -- указатель на массив значений функии в узлах сетки
// M -- количество точек на пространственном диапазоне
// K -- количество точек на временном диапазоне
// tau -- шаг сетки временного диапазона
// h -- шаг сетки пространственного диапазона
void FillFunctionValues(double(*f)(double, double), double* f_arr, int M, int K, double tau, double h) {
for (int k = 0; k < K; ++k) {
    for (int m = 0; m < M; ++m) {
            int idx = GetIdx(k, m, M); // индекс одномерного массива
            f_arr[idx] = f(k*tau, m*h);
        }
    }
}

// Функция заполняет узлы сетки на границе в соответствии с начальными условиями
// f -- указатель на функцию, задающую начальное значение на границе
// arr -- указатель на массив значений на границе
// N -- количество точек соответствующего диапазона (временного или пространственного)
// h -- шаг сетки 
void FillInitialValues(double(*f)(double), double* arr, int N, double h) {
    for (int i = 0; i < N; ++i) {
        arr[i] = f(i*h);
    }
}

int GetRankStart(int rank, int size, int N) {
    int n_range = N / size;
    int mod_size = N % size;

    int rank_start = 0;
    if (rank < mod_size) {
        rank_start = rank*(n_range + 1);
    } 
    else {
        rank_start = rank*n_range + mod_size;
    }

    return rank_start;
}

int GetRankEnd(int rank, int size, int N) {
    int n_range = N / size;
    int mod_size = N % size;

    int rank_end = 0;
    int rank_start = GetRankStart(rank, size, N);
    if (rank < mod_size) {
        rank_end = rank_start + n_range;
    } 
    else {
        rank_end = rank_start + n_range - 1;
    }

    return rank_end;
}

// Функция записывает двумерный массив значений в файл
// data_file_name -- название файла
// u -- указатель на массив значений размером M*K
// M -- количество столбцов в двумерном массиве
// K -- количество строк в двумерном массиве
void PutData2File(const char* data_file_name, double* u, int M, int K) {
    FILE* fd = fopen(data_file_name, "a");
    if (fd) {
        for (int m = 0; m < M; ++m) {
            for (int k = 0; k < K; ++k) {
                int idx = GetIdx(k, m, M);
                fprintf(fd, "%lf ", u[idx]);
            }
            fprintf(fd, "\n");
        }
        fclose(fd);
    }
}

void PutData2FileParallel(int rank, int size, const char* data_file_name, double* u, int M, int K) {
    if (rank != 0) {
        int special_signal = 0;
        MPI_Recv(&special_signal, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    PutData2File(data_file_name, u, M, K);
    if (rank != size-1) {
        int special_signal = 69;
        MPI_Send(&special_signal, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
    }
}

void PutTime2FileParallel(int rank, int size, const char* file_name, double time) {
    if (rank != 0) {
        int special_signal = 0;
        MPI_Recv(&special_signal, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    FILE* fd = fopen(file_name, "a");
    fprintf(fd, "%lf ", time);
    fclose(fd);
    if (rank != size-1) {
        int special_signal = 69;
        MPI_Send(&special_signal, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
    }
}

int main(int argc, char* argv[]) {
    /*
    Задаем T, X, K, M
    */
    double T = 1;
    double X = 1;

    /*Ввод количества точек на расчетной сетке через параметры командной строки*/
    if (argc != 3) {
        fprintf(stderr, "[-] Usage mpiexec -n 4 %s K N\n", argv[0]);
        return -1;
    }
    int K = atoi(argv[1]);
    int M = atoi(argv[2]);

    // шаг сетки по времени и по пространству
    double tau = T / K;
    double h = X / M;

    /*
    Двумерные массивы arr[K][M] будем хранить в виде одномерных массивов
    arr[K*M]. В этом случае будет удобнее передавать их в функции по указателям и,
    в случае надобности, выделять динамически.
    */
    double* f_arr = (double*) malloc(K*M*sizeof(double));
    /*Заполнить массив значений функции в узлах сетки*/
    FillFunctionValues(f, f_arr, M, K, tau, h);

    // начальные условия задачи индекс соответствующего одномерного массива по индексам двумерного
    double* phi_arr = (double*) malloc(K*sizeof(double));
    double* psi_arr = (double*) malloc(M*sizeof(double));
    /*
    Задаем начальные условия
    */
    FillInitialValues(psi, psi_arr, K, tau); // значения функции psi(t) в узлах сетки
    FillInitialValues(phi, phi_arr, M, h); // значения функции phi(t) в узлах сетки

    int size = 0;
    int rank = 0;

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    // ! замеры времени можно ставить в разных местах
    double start = MPI_Wtime();

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // устанавливаем начало и конец диапазона, который обрабатывает текущий процесс
    int rank_M_start = GetRankStart(rank, size, M);
    int rank_M_end = GetRankEnd(rank, size, M);
    int rank_M = abs(rank_M_end - rank_M_start + 1);

    #ifdef DEBUG
    printf("proc [%d] work on space [%d, %d] with %d cells\n", rank, rank_M_start, rank_M_end, rank_M);
    #endif //DEBUG

    double* u     = (double*) malloc(K*rank_M*sizeof(double));
    // u(0, x) = phi(x)
    for (int i = rank_M_start; i <= rank_M_end; ++i) {
        u[GetIdx(0, i-rank_M*rank, rank_M)] = phi_arr[i];
    }
    if (rank == 0) {
        // u(t, 0) = psi(t)
        for (int i = 0; i < K; ++i) {
            u[GetIdx(i, 0, rank_M)] = psi_arr[i];
        }
    }

    // ! замеры времени можно ставить в разных местах
    // double start = MPI_Wtime();

    for (int k = 0; k < K; ++k) {
        if (rank != 0) {
            double u_prev = 0.0;
            MPI_Recv(&u_prev, 1, MPI_DOUBLE, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            u[GetIdx(k+1, 0, rank_M)] = f_arr[GetIdx(k, 0, rank_M)] * tau + (h - tau)/h * u[GetIdx(k, 0, rank_M)] + tau/h * u_prev;
        }

        for (int m = 1; m < rank_M; ++m) 
            u[GetIdx(k+1, m, rank_M)] = f_arr[GetIdx(k, m, rank_M)] * tau + (h - tau)/h * u[GetIdx(k, m, rank_M)] + tau/h * u[GetIdx(k, m-1, rank_M)];    

        if (rank != size-1)
            MPI_Send(&u[GetIdx(k, rank_M-1, rank_M)], 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD);
    }

    //! замеры времени можно ставить в разных местах
    // double end = MPI_Wtime();

    const char* data_file_name = "data.txt";
    const char* time_file_name = "time.txt";
    PutData2FileParallel(rank, size, data_file_name, u, rank_M, K);
    
    // ! замеры времени можно ставить в разных местах
    double end = MPI_Wtime();

    std::cout << "Parallel (" << K << ") " << (end - start)*1000 << " ms \n";
    PutTime2FileParallel(rank, size, time_file_name, (end - start)*1000);

    // free(u);
    MPI_Finalize();

    // free(psi_arr);
    // free(phi_arr);
    // free(f_arr);
    return 0;
}

/*
MPI_Count <- сколько элементов принимает в строку
*/