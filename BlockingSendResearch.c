#include <stdio.h>
#include <string.h>
#include "mpi.h"

/*Предполагаем, что будем работать на 2 процессах*/

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

    if (rank == 1) {
        const char* string_for_send = "string for send";
        int len = strlen(string_for_send);
        // отправляем длинну массива
        MPI_Send(&len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        double start = MPI_Wtime();
        MPI_Send(string_for_send, len, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        double end = MPI_Wtime();
        printf("proc [%d] size %d total send time is %f\n", rank, size, end-start);
    }

    if (rank == 0) {
        int len = 0;
        // отправляем длинну массива
        MPI_Recv(&len, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // here we should to allocate memory dynamic
        char* string_for_resv = (char*) malloc(len);

        double start = MPI_Wtime();
        MPI_Recv(string_for_resv, len, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        double end = MPI_Wtime();
        printf("proc [%d] size %d -- resv %s\n", rank, size, string_for_resv);
        printf("proc [%d] size %d total send time is %f\n", rank, size, end-start);
    }

    MPI_Finalize();

    return 0;
}