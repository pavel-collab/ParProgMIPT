#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[]) {
    
    int size = 0;
    int rank = 0;
    int mod_size = 0;

    if (argc != 2) {
        fprintf(stderr, "[-] Usage %s N\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (floor(N/2) < size) {
        perror("[-] Every process have to sum at least two numbers.\nSo, total number of processes must be at least 2 times less then N.\n");
        return 1;
    }

    // остаток от деления колиства чисел на количество процессов
    mod_size = N % size;

    MPI_Finalize();
    
    return 0;
}