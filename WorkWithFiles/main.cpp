#include <iostream>
#include <cstring>
#include "mpi.h"

int main(int argc, char* argv[]) {

    int size = 0;
    int rank = 0;
    int offset = 0;

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // descriptor
    MPI_File data_file;
    MPI_File_open(MPI_COMM_WORLD, "data.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY | MPI_MODE_APPEND, MPI_INFO_NULL, &data_file);

    const char* my_rank = (std::to_string(rank) + " ").c_str();
    // установка смещения
    MPI_File_set_view(data_file, (size-rank-1)*strlen(my_rank)*sizeof(char), MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
    MPI_File_write(data_file, my_rank, strlen(my_rank), MPI_CHAR, MPI_STATUS_IGNORE);
    
    MPI_File_close(&data_file);

    MPI_Finalize();

    return 0;
}