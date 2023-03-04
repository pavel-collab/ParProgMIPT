#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[]) {

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

    /*code here*/

    MPI_Finalize();

    return 0;
}