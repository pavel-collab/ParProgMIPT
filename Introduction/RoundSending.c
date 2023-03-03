#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[]) {
    
    int rank = 0;
    int size = 0;

    int target = 0;
    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        MPI_Send(&target, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        MPI_Recv(&target, 1, MPI_INT, size-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("Proc %d; recv target value is %d\n", rank, target);
    }
    else {
        // ждем, пока к нам придет информация от rank-1 процесса
        MPI_Recv(&target, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("Proc %d; recv target value is %d\n", rank, target);
        // инкремент переменной
        target++;
        if (rank+1 != size) {
            // отправляем данные rank+1 процессу
            MPI_Send(&target, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        }
        else {
            // отправляем данные нулевому процессу
            MPI_Send(&target, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    
    return 0;
}