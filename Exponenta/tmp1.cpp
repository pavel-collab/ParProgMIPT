/*
This is an example of using gmp library
*/
#include <iostream>
#include <gmpxx.h>
#include <cstring>
#include "mpi.h"

int main(int argc, char* argv[]) {
    /*
    Пример программы: 2 потока
    один формирует число из длинной арифметики и передает его второму потоку в качестве строки
    второй поток принимает строку, формирует из нее число длинной арифметики, производит метематические операции
    и выводит на экран
    */

    // int size = 0;
    // int rank = 0;
    // MPI_Status status;

    // int rc = MPI_Init(&argc, &argv);
    // if (rc != MPI_SUCCESS) {
    //     perror("[-] Error starting MPI program. Programm was terminated.\n");
    //     MPI_Abort(MPI_COMM_WORLD, rc);
    // }

    // MPI_Comm_size(MPI_COMM_WORLD, &size);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // if (rank == 0) {
    //     mpz_t var;
    //     mpz_init_set_ui(var, 12409710294);
    //     char* str_var = mpz_get_str(NULL, 10, var);
    //     MPI_Send(str_var, strlen(str_var), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    //     mpz_clear(var);
    // }
    // else if (rank == 1) {
    //     char* str_var;
    //     MPI_Recv(str_var, 20, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    //     mpz_t var;
    //     mpz_init(var);
    //     mpz_set_str(var, str_var, 10);
    //     mpz_t addition;
    //     mpz_init_set_ui(addition, 20);
    //     mpz_add(var, var, addition);
    //     std::cout << var << std::endl;
    // }

    // MPI_Finalize();

    int size = 0;
    int rank = 0;
    int accuracy = 8;

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        mpf_t var;
        mpf_init_set_d(var, 3.14159265);

        char *mpi_buf = (char*)calloc(accuracy + 8, sizeof(char));
        char *format_str = (char*)calloc(accuracy + 8, sizeof(char));

        size_t format_str_size = accuracy + 12;

        snprintf(format_str, format_str_size, "%%.%dFf", accuracy);
        gmp_snprintf(mpi_buf, accuracy + 8, format_str, var);
        MPI_Send(mpi_buf, accuracy + 8, MPI_CHAR, 1, 0, MPI_COMM_WORLD);

        mpf_clear(var);
    }
    else if (rank == 1) {
        char *rvc_buf = (char*)calloc(accuracy + 8, sizeof(char));
        MPI_Recv(rvc_buf, accuracy + 8, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        std::cout << rvc_buf << std::endl;
        
        mpf_t var;
        mpf_init(var);
        mpf_set_str(var, rvc_buf, 10);
        std::cout << var << std::endl;
        mpf_clear(var);
    }

    MPI_Finalize();

    return 0;
}