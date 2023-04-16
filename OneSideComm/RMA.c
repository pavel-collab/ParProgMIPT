#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mpi.h"

#define DEBUG

/*
In the programm we create a chunk of memory, that "shared" between two processes
And each of two prosess could work with it
*/

double Sum(int start, int end) {
    double res = 0;
    for (double i = start; i <= end; ++i) {
        res += 1/i;
    }
    return res;
}

/*Функция вычисляет частичную сумму 1/n для указанного процесса*/
double GetRankSumRes(int rank, int size, int N) {
    int n_range = N / size;
    int mod_size = N % size;
    int rank_start, rank_end = 0;

    if (rank < mod_size) {
        rank_start = rank*(n_range + 1) + 1;
        rank_end = rank_start + n_range;
    }
    else {
        rank_start = rank*n_range + 1 + mod_size;
        rank_end = rank_start + n_range - 1;
    }

    return Sum(rank_start, rank_end);
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: mpiexec -n 3 %s N\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);

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

    /*window creation*/
    int win_sz = 0;
    double res = 0;

    // окно создается только в главном процессе
    // все остальные процессы только обращаются к нему
    if (rank == 0)
        win_sz = 1;

    int dbl_sz = 0;
    MPI_Type_size(MPI_DOUBLE, &dbl_sz);
    MPI_Win win;
    MPI_Win_create(&res, win_sz*dbl_sz, dbl_sz, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    double rank_res = GetRankSumRes(rank, size, N);

    /*Accumulate rank_res in window*/
    // Прежде, чем обращаться к окну всегда надо вызывать функцию синхронизации
    MPI_Win_fence(0, win);
    MPI_Accumulate(&rank_res, 1, MPI_DOUBLE, 0, 0, 1, MPI_DOUBLE, MPI_SUM, win);
    MPI_Win_fence(0, win);

    if (rank == 0) {
        sleep(5);
        printf("res is %lf\n", res);
    }


    MPI_Win_free(&win);
    MPI_Finalize();

    return 0;
}