#include <stdio.h>
#include <math.h>
#include "mpi.h"

int Sum(int start, int end) {
    int res = 0;
    for (unsigned i = start; i <= end; ++i) {
        res += i;
    }
    return res;
}

//TODO: make an error checking, use MPI_Abort
int main(int argc, char* argv[]) {
    
    int size = 0;
    int rank = 0;
    int res = 0;

    MPI_Status status;

    //TODO: refactor names
    uint64_t n_range = 0;
    uint64_t mod_size = 0;

    if (argc != 2) {
        fprintf(stderr, "[-] Usage %s N\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // ATTENTION! floor return value is double
    if (floor(N/2) < size) {
        perror("[-] Every process have to sum at least two numbers.\nSo, total number of processes must be at least 2 times less then N.\n");
        return 1;
    }

    n_range = N / size;
    mod_size = N % size;

    int rank_res = 0;
    int rank_start = rank*n_range + 1;
    int rank_end = rank_start + n_range - 1;
    
    if (rank != size-1) {
        rank_res = Sum(rank_start, rank_end);
    }
    else {
        rank_res = Sum(rank_start, rank_end + mod_size);
    }

    if (rank != 0) {
        printf("Process %d, size %d, rank sum %d\n", rank, size, rank_res);
        MPI_Send(&rank_res, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    else {
        printf("Process %d, size %d, rank sum %d\n", rank, size, rank_res);
        res += rank_res;

        int tmp_res = 0;
        for (unsigned proc_rank = 1; proc_rank < size; proc_rank++) {
            MPI_Recv(&tmp_res, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            res += tmp_res;
        }
    }

    MPI_Finalize();

    printf("Total sum: %d\n", res);
    
    return 0;
}

// |1 2 3|  |4 5 6|  |7 8 9|  10 11