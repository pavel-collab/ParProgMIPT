/*
Цели создания новых коммуникаторов более эффективноя работа с пересылками.
Нампример нам нужно, чтобы общались между собой только процессы с четными ранками.
Или нам нужно переупорядочить процессы (потому что коммуникатор это набор упорядоченных ранков).
*/
#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[]) {

    int size = 0;
    int rank = 0;

    int new_size = 0;
    int new_rank = 0;

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm new_comm;
    // группа в отличие от коммуникатора не может обеспечивать пересылки
    MPI_Group base_group;
    MPI_Group new_group;

    MPI_Comm_group(MPI_COMM_WORLD, &base_group);
    int exclude_rank = size-1;
    MPI_Group_excl(base_group, 1, &exclude_rank, &new_group);
    MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm);

    MPI_Comm_size(new_comm, &new_size);
    MPI_Comm_rank(new_comm, &new_rank);

    printf("proc [%d] size %d\n", new_rank, new_size);

    MPI_Finalize();

    return 0;
}