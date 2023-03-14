#include <iostream>
#include <cmath>
#include "mpi.h"

#define DEBUG

int main(int argc, char* argv[]) {

    int size = 0;
    int rank = 0;
    double res = 0;

    MPI_Status status;

    uint64_t n_range = 0;
    uint64_t mod_size = 0;

    if (argc != 2) {
        fprintf(stderr, "[-] Usage %s K\n", argv[0]);
        return 1;
    }
    // количество знаков после запятой
    int K = atoi(argv[1]);
    int N = 4 + K;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (int(floor(N/2)) < size) {
        perror("[-] Every process have to sum at least two numbers.\nSo, total number of processes must be at least 2 times less then N.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    n_range = N / size;
    mod_size = N % size;

    double cur_item = 1; // начальный элемент ряда
    double rank_res = 0; // частичная сумма каждого поцесса

    int rank_start, rank_end = 0;

    // Задаем, с какого по какой элемент ряда суммирует процесс
    if (rank < mod_size) {
        rank_start = rank*(n_range + 1) + 1;
        rank_end = rank_start + n_range;
    }
    else {
        rank_start = rank*n_range + 1 + mod_size;
        rank_end = rank_start + n_range - 1;
    }

    for (int i = rank_start; i <= rank_end; ++i) {
        cur_item = cur_item / i;
        rank_res += cur_item;
    }

    #ifdef DEBUG
    printf("proc [%d] size %d -- in range [%d, %d] rank res is %lf\n", rank, size, rank_start, rank_end, rank_res);
    #endif //DEBUG

    double factor = 0;
    if (rank != 0) {
        for (int proc = 0; proc < rank; ++proc) {
            MPI_Recv(&factor, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            rank_res *= factor;
            cur_item *= factor;
        }
        MPI_Send(&rank_res, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    #ifdef DEBUG
    printf("proc [%d] size %d -- in range [%d, %d] rank res is %lf\n", rank, size, rank_start, rank_end, rank_res);
    #endif //DEBUG

    if (rank != size) {
        for (int proc = rank+1; proc < size; ++proc) {
            MPI_Send(&cur_item, 1, MPI_DOUBLE, proc, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0) {
        double tmp_res;
        // need to explain, why +1
        res = rank_res + 1;
        for (int proc = 1; proc < size; ++proc) {
            MPI_Recv(&tmp_res, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            res += tmp_res;
        }
    }

    MPI_Finalize();

    printf("result is %lf\n", res);

    return 0;
}