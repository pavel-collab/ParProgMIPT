#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[]) {
    
    int rank = 0;
    int size = 0;

    int target = 0;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        // ждем, пока к нам придут данные от size-1 процесса
        // другие действия
    }
    else {
        // ждем, пока к нам придет информация от rank-1 процесса
        // инкремент переменной
        if (rank+1 != size) {
            // отправляем данные rank+1 процессу
        }
        else {
            // отправляем данные нулевому процессу
        }
    }

    MPI_Finalize();
    
    return 0;
}