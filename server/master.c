#include <stdio.h>
#include "mpi.h"

/*
Program start self child processes server and client
Make a communication between them
And return intercommunicator
*/

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

    // intercommunicator between server and client
    MPI_Comm intercom;

    // Spawn applications: server and client with a single frosesses for each of them
    // Server must be spawned before client
    MPI_Comm_spawn("server", MPI_ARGV_NULL, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercom, MPI_ERRCODES_IGNORE);
    MPI_Comm_spawn("client", MPI_ARGV_NULL, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &intercom, MPI_ERRCODES_IGNORE);

    MPI_Finalize();

    return 0;
}