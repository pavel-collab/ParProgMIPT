#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

    // Look up for server's name
    char port_name[MPI_MAX_PORT_NAME];
    MPI_Lookup_name("server_name", MPI_INFO_NULL, port_name);

    // Connect to server
    MPI_Comm server;
    MPI_Comm_connect(port_name, MPI_INFO_NULL, 0, MPI_COMM_SELF, &server);

    // Send data to server
    int send_buf = 42;
    MPI_Send(&send_buf, 1, MPI_INT, 0, 0, server);
    printf("[CLIENT] send data %d to the server\n", send_buf);

    MPI_Finalize();

    return 0;
}