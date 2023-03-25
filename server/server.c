#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

    // Open port
    char port_name[MPI_MAX_PORT_NAME];
    MPI_Open_port(MPI_INFO_NULL, port_name);
    
    printf("[SERVER] The port name is %s\n", port_name);

    // Publish name and accept client
    MPI_Comm client;
    MPI_Publish_name("server_name", MPI_INFO_NULL, port_name);
    MPI_Comm_accept(port_name, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &client);

    // Recive data from client
    int recv_buf;
    MPI_Recv(&recv_buf, 1, MPI_INT, 0, 0, client, MPI_STATUS_IGNORE);
    printf("[SERVER] the data recv from client %d\n", recv_buf);

    MPI_Unpublish_name("server_name", MPI_INFO_NULL, port_name);
    MPI_Close_port(port_name);

    MPI_Finalize();

    return 0;
}