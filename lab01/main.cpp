#include <iostream>
#include "mpi.h"

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

int main(int argc, char* argv[]) {
    /*
    Задаем T, X, K, M
    */
    double T = 1;
    double X = 1;
    int K = 100; // по t
    int M = 100; // по x

    // шаг сетки по времени и по пространству
    double tau = T / K;
    double h = X / M;

    /*
    Двумерные массивы arr[K][M] будем хранить в виде одномерных массивов
    arr[K*M]. В этом случае будет удобнее передавать их в функции по указателям и,
    в случае надобности, выделять динамически.
    */
    double f_arr[K*M];
    double u[K*M];
    /*Заполнить массив значений функции в узлах сетки*/
    FillFunctionValues(f, f_arr, M, K, tau, h);

    // начальные условия задачи индекс соответствующего одномерного массива по индексам двумерного
    double phi_arr[K];
    double psi_arr[M];
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

    int size = 0;
    int rank = 0;

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //! Временная заглушка: тестировать будем с использованием 4 процессов
    if (size != 4) {
        perror("[-] Expected 4 processes. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    // начало и конец диапазона (пространственного или временного), котрый обрабатывает текущий процесс
    int rank_M_start = rank*N / 4;
    int rank_M_end = rank_start + N/4 - 1;
    int rank_K_start = rank*K / 4;
    int rank_K_end = rank_start + K/4 - 1;

    /*code here*/

    MPI_Finalize();

    return 0;
}