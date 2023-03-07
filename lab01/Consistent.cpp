#include <iostream>
#include <cmath>

#define pi 3.14159265

// Заготовка под функцию
double f(double t, double x) {
    return x + t;
}

// Заготовка под функции начальных значений
double phi(double x) {
    return cos(pi*x);
}

double psi(double t) {
    return exp(-t);
}

int main() {
    
    // пределы сетки по времени и по пространству
    double T, X = 0;
    // количество узлов сетки по времени и по пространству
    int K, M = 0;

    /*
    Задаем T, X, K, M
    */
    T = 1;
    X = 1;
    K = 100;
    M = 100;

    // шаг сетки по времени и по пространству
    double tau = T / K;
    double h = X / M;

    double f_arr[K][M];
    double u[K][M];
    /*Заполнить массив для решения нулями*/
    /*Заполнить массив значений функции в узлах сетки*/
    for (int k = 0; k < K; ++k) {
        for (int m = 1; m <= M; ++m) {
            f_arr[k][m] = f(k*tau, m*h);
        }
    }

    // начальные условия задачи
    double phi_arr[K];
    double psi_arr[M];
    /*
    Задаем начальные условия
    */
    for (int k = 0; k < K; ++k) {
        psi_arr[k] = exp(-k*tau);
    }
    for (int m = 0; m < M; ++m) {
        phi_arr[m] = phi(m*h);
    }


    for (int i = 0; i < M; ++i) {
        u[0][i] = phi_arr[i];
    }
    for (int i = 0; i < K; ++i) {
        u[i][0] = psi_arr[i];
    }

    // применение разностной схемы
    for (int k = 0; k < K; ++k) {
        for (int m = 1; m <= M; ++m) {
            u[k+1][m] = (f_arr[k][m] + (u[k][m-1] - u[k][m]) / h)*tau + u[k][m];
        }
    }

    const char* data_file_name = "data.txt";
    FILE* fd = fopen(data_file_name, "a");
    if (fd) {
        for (int k = 0; k < K; ++k) {
            for (int m = 1; m <= M; ++m) {
                fprintf(fd, "%lf ", u[k][m]);
            }
            fprintf(fd, "\n");
        }
        fclose(fd);
    }

    return 0;
}