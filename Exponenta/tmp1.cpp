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

    if (rank == 0) {
        mpz_t var; // заводим целоисленную переменную длинной арифметики
        mpz_init_set_ui(var, 124097102); // инициализируем эту переменную
        // mpz_init_set_str(var, "124097102");
        std::cout << var << std::endl;
        char* str_var = mpz_get_str(NULL, 10, var); // конвертируем переменную длинной арифметики в строку
        std::cout << str_var << std::endl;
        int str_len = strlen(str_var); // длина строки
        MPI_Send(&str_len, 1, MPI_INT, 1, 0, MPI_COMM_WORLD); // сначала отсылаем длину строки
        MPI_Send(str_var, str_len, MPI_CHAR, 1, 0, MPI_COMM_WORLD); // отсылаем строку
        mpz_clear(var);
    }
    else if (rank == 1) {
        int str_len = 0;
        MPI_Recv(&str_len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // принимаем длину строки
        std::cout << str_len << std::endl;
        char* str_var = (char*) calloc(str_len, sizeof(char)); // выделяем буффер, чтобы принять строку
        MPI_Recv(str_var, str_len, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  //прнимаем строку
        std::cout << str_var << std::endl;
        
        mpz_t var; // заводим новую переменную длинной арифметики
        mpz_init(var); // инициализируем ее
        mpz_set_str(var, str_var, 10); // присваем ей значение через строку
        std::cout << var << std::endl;
        mpz_t addition;
        mpz_init_set_ui(addition, 20);
        mpz_add(var, var, addition);
        std::cout << var << std::endl;
    }

    MPI_Finalize();

    return 0;
}