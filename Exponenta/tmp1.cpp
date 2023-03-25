/*
This is an example of using gmp library
*/
#include <iostream>
#include <gmpxx.h>
#include <cstring>
// #include "mpi.h"

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

    // int size = 0;
    // int rank = 0;
    // int accuracy = 8;

    // MPI_Status status;

    // int rc = MPI_Init(&argc, &argv);
    // if (rc != MPI_SUCCESS) {
    //     perror("[-] Error starting MPI program. Programm was terminated.\n");
    //     MPI_Abort(MPI_COMM_WORLD, rc);
    // }

    // MPI_Comm_size(MPI_COMM_WORLD, &size);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    
    // MPI_Finalize();

    mpf_t var;
    mpf_init_set_str(var, "3.14159265", 10);
    gmp_printf("%Ff\n", var);
    mp_exp_t exppt;

    char* val_str = mpf_get_str(nullptr, &exppt, 10, 0, var);
    // получили массив символов без точки
    std::cout << val_str << std::endl;


    mpf_t new_val;
    mpf_init_set_str(new_val, val_str, 10);
    std::cout << exppt << std::endl;
    new_val->_mp_exp = exppt;

    gmp_printf("%Ff\n", new_val);
    return 0;
}