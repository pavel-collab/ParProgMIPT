#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mpi.h"

/*Предполагаем, что будем работать на 2 процессах*/
// #define DEBUG

int main(int argc, char* argv[]) {

    int size = 0;
    int rank = 0;

    const char* send_file_name = "send.txt";
    const char* resv_file_name = "resv.txt";

    if (argc != 3) {
        fprintf(stderr, "[-] Usage %s N mode\nFor mode you can use:\n\tstandart\n\tbuf\n\tsynch\n\tready\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    /*
    There could be 4 mode:
    "-" -- standart MPI_Send
    "b" -- MPI_Bsend
    "s" -- MPI_Ssend
    "r" -- MPI_Rsend
    */    
    char* mode = argv[2];

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 1) {
        int* data_for_send = (int*) malloc(N*sizeof(int));
        memset(data_for_send, 1, N*sizeof(int));
        // отправляем длинну массива
        MPI_Send(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        double start = MPI_Wtime();
        if (!strcmp(mode, "standart"))
            MPI_Send(data_for_send, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
        else if (!strcmp(mode, "buf"))
            MPI_Bsend(data_for_send, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
        else if (!strcmp(mode, "synch"))
            MPI_Ssend(data_for_send, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
        else if (!strcmp(mode, "ready"))
            MPI_Rsend(data_for_send, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
        else {
            perror("[-] There is not such mode\n");
            MPI_Abort(MPI_COMM_WORLD, rc);
        }
        double end = MPI_Wtime();
        FILE* fd = fopen(send_file_name, "a");
        if (fd) {
            fprintf(fd, "%lf\n", end-start);
            fclose(fd);
        }
        #ifdef DEBUG
        printf("proc [%d] size %d total send time is %f\n", rank, size, end-start);
        #endif // DEBUG
        free(data_for_send);
    }

    if (rank == 0) {
        int len = 0;
        // отправляем длинну массива
        MPI_Recv(&len, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // here we should to allocate memory dynamic
        int* data_for_resv = (int*) malloc(len*sizeof(int));
        sleep(2);
        double start = MPI_Wtime();
        MPI_Recv(data_for_resv, len, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        double end = MPI_Wtime();
        FILE* fd = fopen(resv_file_name, "a");
        if (fd) {
            fprintf(fd, "%lf\n", end-start);
            fclose(fd);
        }
        #ifdef DEBUG
        printf("proc [%d] size %d total resv time is %f\n", rank, size, end-start);
        #endif // DEBUG
        free(data_for_resv);
    }

    MPI_Finalize();

    return 0;
}