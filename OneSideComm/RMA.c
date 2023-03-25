#include <stdio.h>
#include <math.h>
#include "mpi.h"

// #define COMPARE
#define DEBUG

/*
In the programm we create a chunk of memory, that "shared" between two processes
And each of two prosess could work with it
*/

int main(int argc, char* argv[]) {

    int size = 0;
    int rank = 0;
    int window_buffer_size = 4;

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (size != 2) {
        perror("[-] Too many processes. Expected 2 only");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    int window_buffer[window_buffer_size];
    if (rank == 1) {
        window_buffer[0] = 1;
        window_buffer[1] = 11;
        window_buffer[2] = 111;
        window_buffer[3] = 1111;
    }    

    // Create a window object that will let us to share allocated memory between rank = 1 and rank = 0
    MPI_Win win;
    MPI_Win_create(&window_buffer, /* pre-allocated buffer */
               (MPI_Aint)window_buffer_size * sizeof(int), /* size in bytes */
               sizeof(int), /* displacement units */
               MPI_INFO_NULL, /* info object */
               MPI_COMM_WORLD, /* communicator */
               &win /* window object */);

    /*
    Every rank has now a window, but only the window on rank 1 has values different from 0.
    */
    
    // Before doing anything on the window, we need to start an access epoch
    MPI_Win_fence(0, /* assertion */
              win /* window object */);

    if (rank == 0) {
        int local_buf[window_buffer_size];
        MPI_Get(&local_buf, /* pre-allocated buffer on RMA origin process */
          window_buffer_size, /* count on RMA origin process */
          MPI_INT, /* type on RMA origin process */
          1, /* rank of RMA target process */
          0, /* displacement on RMA target process */
          window_buffer_size, /* count on RMA target process */
          MPI_INT, /* type on RMA target process */
          win /* window object */);

        printf("[rank %d] the data, we get from the shared window:\n", rank);
        for (int i = 0; i < window_buffer_size; ++i) {
            printf("local_buf[%d] = %d\n", i, local_buf[i]);
        }
    }

    // this access epoch is closed
    MPI_Win_fence(0, /* assertion */
              win /* window object */);

    MPI_Win_free(&win);
    MPI_Finalize();

    return 0;
}