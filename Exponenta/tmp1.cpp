/*
This is an example of using gmp library
*/
#include <iostream>
#include <gmpxx.h>
#include <cstring>
#include "mpi.h"

#define LEN_TAG 11
#define MESSAGE_TAG 12
#define SPECIAL_TAG 1

void MPI_SEND_MPZ(mpz_srcptr numb, int dest, MPI_Comm comm) {
    // полуаем строку из длинной арифметики
    char* str_numb = mpz_get_str(NULL, 10, numb);
    // вычисляем ее длинну
    int str_len = strlen(str_numb);
    // отправляем длинну строки
    MPI_Send(&str_len, 1, MPI_INT, dest, LEN_TAG, comm);
    // отправляем сообщение
    MPI_Send(str_numb, str_len, MPI_CHAR, dest, MESSAGE_TAG, comm);
}

void MPI_RCV_MPZ(char* buf, int source, MPI_Comm comm, MPI_Status* status) {
    // принимаем длинну сообщения
    int str_len = 0;
    MPI_Recv(&str_len, 1, MPI_INT, source, LEN_TAG, MPI_COMM_WORLD, status);
    buf = (char*) realloc(buf, str_len);
    // принимаем строку
    MPI_Recv(buf, str_len, MPI_CHAR, source, MESSAGE_TAG, MPI_COMM_WORLD, status);
}

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
        MPI_SEND_MPZ(var, 1, MPI_COMM_WORLD);
        mpz_clear(var);
    }
    else if (rank == 1) {
        char* str_var = (char*) calloc(10, sizeof(char));
        MPI_RCV_MPZ(str_var, 0, MPI_COMM_WORLD, &status);
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