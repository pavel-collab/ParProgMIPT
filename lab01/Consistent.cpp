#include <iostream>
#include <cmath>
#include <chrono>

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

// Функция производит численно решение задачи с применением схемы: явный левый уголок
// u -- указатель на массив значений размером M*K
// f_arr -- указатель на массив значений функии в узлах сетки
// M -- количество точек на пространственном диапазоне
// K -- количество точек на временном диапазоне
// tau -- шаг сетки временного диапазона
// h -- шаг сетки пространственного диапазона
void LeftCorner(double* u, double* f_arr, int M, int K, double tau, double h) {
    for (int k = 0; k < K; ++k) {
        for (int m = 1; m < M; ++m) {
            u[GetIdx(k+1, m, M)] = f_arr[GetIdx(k, m, M)] * tau + (h - tau)/h * u[GetIdx(k, m, M)] + tau/h * u[GetIdx(k, m-1, M)];
        }
    }
}

// Функция производит численно решение задачи с применением схемы: явная четырехточечная схема
void FourPointScheme(double* u, double* f_arr, int M, int K, double tau, double h) {
    for (int k = 0; k < K; ++k) {
        for (int m = 1; m < M; ++m) {
            if (m < M-1)
                u[GetIdx(k+1, m, M)] = f_arr[GetIdx(k, m, M)] * tau + (tau / (2*h) + tau*tau / (2*h*h)) * u[GetIdx(k, m-1, M)] + (tau*tau / (2*h*h) - tau / (2*h)) * u[GetIdx(k, m+1, M)] + (1 - tau*tau / (h*h)) * u[GetIdx(k, m, M)];
            else // точки на левой границе считаем схемой левого уголка
                u[GetIdx(k+1, m, M)] = f_arr[GetIdx(k, m, M)] * tau + (h - tau)/h * u[GetIdx(k, m, M)] + tau/h * u[GetIdx(k, m-1, M)];
        }
    }
}

// Функция производит численно решение задачи с применением схемы: крест
void CrossScheme(double* u, double* f_arr, int M, int K, double tau, double h) {
// обсчитываем первый слой схемой уголка
    for (int m = 1; m < M; ++m) {
        u[GetIdx(1, m, M)] = f_arr[GetIdx(0, m, M)] * tau + (h - tau)/h * u[GetIdx(0, m, M)] + tau/h * u[GetIdx(0, m-1, M)];
    }
    for (int k = 1; k < K; ++k) {
        for (int m = 1; m < M; ++m) {
            if (m < M-1) 
                u[GetIdx(k+1, m, M)] = f_arr[GetIdx(k, m, M)] * 2*tau + u[GetIdx(k-1, m, M)] + (u[GetIdx(k, m-1, M)] - u[GetIdx(k, m+1, M)]) * tau / h;
            else // точки на левой границе считаем схемой левого уголка
                u[GetIdx(k+1, m, M)] = f_arr[GetIdx(k, m, M)] * tau + (h - tau)/h * u[GetIdx(k, m, M)] + tau/h * u[GetIdx(k, m-1, M)];
        }
    }
}

int main(int argc, char* argv[]) {
    /*
    Задаем T, X, K, M
    */
    double T = 1;
    double X = 1;

    /*
    Пока что на 8000*8000 узлах на последовательной программе заметно замедление.
    Компьютеру прям плохо становится.
    */
    // int K = 1000; // по t
    // int M = 1000; // по x

    /*Ввод количества точек на расчетной сетке через параметры командной строки*/
    if (argc != 3) {
        fprintf(stderr, "[-] Usage %s K N\n", argv[0]);
        return -1;
    }
    int K = atoi(argv[1]);
    int M = atoi(argv[2]);
    // int K = 100;
    // int M = 100;

    // шаг сетки по времени и по пространству
    double tau = T / K;
    double h = X / M;

    //! замеры времени можно ставить в разных местах
    auto t_start = std::chrono::high_resolution_clock::now();

    /*
    Двумерные массивы arr[K][M] будем хранить в виде одномерных массивов
    arr[K*M]. В этом случае будет удобнее передавать их в функции по указателям и,
    в случае надобности, выделять динамически.
    */
    double* f_arr = (double*) malloc(K*M*sizeof(double));
    double* u     = (double*) malloc(K*M*sizeof(double));
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

    // u(0, x) = phi(x)
    for (int i = 0; i < M; ++i) {
        u[GetIdx(0, i, M)] = phi_arr[i];
    }
    // u(t, 0) = psi(t)
    for (int i = 0; i < K; ++i) {
        u[GetIdx(i, 0, M)] = psi_arr[i];
    }

    //! замеры времени можно ставить в разных местах
    // auto t_start = std::chrono::high_resolution_clock::now();

    //---------------------------------------Расчетная схема №1---------------------------------------
    //---------------------------------------Явный левый уголок---------------------------------------
    LeftCorner(u, f_arr, M, K, tau, h);
    //------------------------------------------------------------------------------------------------

    //---------------------------------------Расчетная схема №2---------------------------------------
    //----------------------------------Явная четырехточечная схема-----------------------------------
    // LeftCorner(u, f_arr, M, K, tau, h);
    //------------------------------------------------------------------------------------------------

    //---------------------------------------Расчетная схема №3---------------------------------------
    //---------------------------------------------Крест----------------------------------------------
    // CrossScheme(u, f_arr, M, K, tau, h);
    //------------------------------------------------------------------------------------------------
    
    //! замеры времени можно ставить в разных местах
    // auto t_end = std::chrono::high_resolution_clock::now();

    const char* data_file_name = "validate.txt";
    const char* file_name = "time.txt";
    PutData2File(data_file_name, u, M, K);

    //! замеры времени можно ставить в разных местах
    auto t_end = std::chrono::high_resolution_clock::now();
    
    std::cout << "consistent " << std::chrono::duration<double, std::milli>(t_end-t_start).count() << " ms\n";
    FILE* fd = fopen(file_name, "a");
    fprintf(fd, "%lf ", std::chrono::duration<double, std::milli>(t_end-t_start).count());
    fclose(fd);

    /*
    Данный кусок памяти вроде как надо чистить.
    Но при попытке поистить его, программа выкидывает double free or corruption.
    */
    free(psi_arr); //* psi_arr -- единственный массив, который освобождается без проблем
    psi_arr = NULL;
    // free(f_arr);
    // f_arr = NULL;
    // free(u);
    // u = NULL;
    // free(phi_arr);
    // phi_arr == NULL;
    return 0;
}