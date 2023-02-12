#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[]) {
    
    // define variables:
    //  rank -- the number of current process
    //  size -- total amount of process
    int rank = 0;
    int size = 0;

    // Initilize parallel part of program
    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    // Get the total number of pocess (save in variable size)
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Get the number of current pocess (save in variable rank)
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Hello, world! Process %d, size %d\n", rank, size);

    // The end of parallel part of program
    MPI_Finalize();

    return 0;
}