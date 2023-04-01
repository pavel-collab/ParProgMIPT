#include <iostream>
#include <gmpxx.h>
#include <cstring>
#include <cmath>
// #include <fstream>
#include "mpi.h"

#define DEBUG
#define LEN_TAG 11
#define MESSAGE_TAG 12
#define SPECIAL_TAG 1

// Релизация численного решения нелинейного уравнения N*ln(N) - N - K*ln(10) = 0
// методом Ньютона для определения количества слагаемых N в ряде 
// для достижения заданной точности K (количество знаков после десятичной точки)
long long Newtons_M(unsigned long K, double init_x) {
    unsigned steps = 0;
    double accuracy = 1e-6;
    double x_cur = init_x;
    double x_prev = 0;

    while(abs(x_cur - x_prev) > accuracy) {
        x_prev = x_cur;
        steps+=1;
        x_cur = x_prev - (x_prev*log(x_prev) - x_prev - K*log(10)) / (log(x_prev));
    }

    return static_cast<long long>(std::floor(x_cur));
}

// Функция сопостовляем величине rank процесса количество слагаемых ряда, которое процесс
// должен обработать. Слагаемые нумеруются, начиная с 1. Процесс с номером rank
// обрабатывает слагаемые начиная с rank_start и заканчивая rank_end
void ItemDestribution(int rank, int size, long long N, int* rank_start, int* rank_end) {
    int64_t n_range = N / size;
    uint64_t mod_size = N % size;

    // Задаем, с какого по какой элемент ряда суммирует процесс
    if (rank < mod_size) {
        *rank_start = rank*(n_range + 1) + 1;
        *rank_end = *rank_start + n_range;
    }
    else {
        *rank_start = rank*n_range + 1 + mod_size;
        *rank_end = *rank_start + n_range - 1;
    }    
}

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

void MPI_RECV_MPZ(char* buf, int source, MPI_Comm comm, MPI_Status* status) {
    // принимаем длинну сообщения
    int str_len = 0;
    MPI_Recv(&str_len, 1, MPI_INT, source, LEN_TAG, MPI_COMM_WORLD, status);
    buf = (char*) realloc(buf, str_len);
    // принимаем строку
    MPI_Recv(buf, str_len, MPI_CHAR, source, MESSAGE_TAG, MPI_COMM_WORLD, status);
}

int main(int argc, char* argv[]) {

    int size = 0;
    int rank = 0;
    mpf_t total_res; // результат работы программы -- число e с заданной точностью (получаем делением res на last_fector)
    mpf_t last_factor; // n! который последний процесс отправляет первому для деления
    mpz_t res; // результат сложения ВСЕХ слагаемых, но целочисленный (без деления на n!)

    MPI_Status status;

    if (argc != 2) {
        fprintf(stderr, "[-] Usage %s K\n", argv[0]);
        return 1;
    }
    // количество знаков после запятой
    unsigned long K = atoi(argv[1]);
    long long N = Newtons_M(K, 2.0);

    #ifdef DEBUG
    printf("To get %lu accuracy we have to have %lld items\n", K, N);
    #endif //DEBUG

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (int(floor(N/2)) < size) {
        perror("[-] Every process have to sum at least two numbers.\nSo, total number of processes must be at least 2 times less then N.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    int rank_start, rank_end = 0;

    ItemDestribution(rank, size, N, &rank_start, &rank_end);

    mpz_t cur_item; // начальный элемент ряда
    mpz_t cur_num; // текущий множитель в ряде
    mpz_t rank_res; // частичная сумма каждого поцесса

    // инициализируем переменные длинной арифметики
    mpz_init_set_ui(cur_item, N - rank_start + 1);
    mpz_init_set_ui(cur_num, N - rank_start + 1);
    mpz_init_set_ui(rank_res, N - rank_start + 1);

    for (int i = rank_start; i <= rank_end-1; ++i) {
        mpz_t decriment;
        mpz_init_set_ui(decriment, 1);

        mpz_sub(cur_num, cur_num, decriment);
        mpz_mul(cur_item, cur_item, cur_num);
        mpz_add(rank_res, rank_res, cur_item);
    }

    if (rank != 0) {
        char* str_factor = (char*) calloc(10, sizeof(char));
        MPI_RECV_MPZ(str_factor, rank-1, MPI_COMM_WORLD, &status);

        mpz_t factor;
        mpz_init_set_str(factor, str_factor, 10);
        free(str_factor);

        mpz_mul(rank_res, rank_res, factor);
        mpz_mul(cur_item, cur_item, factor);

        MPI_SEND_MPZ(rank_res, 0, MPI_COMM_WORLD);

        if (rank == size-1) {
            char* cur_item_str = mpz_get_str(NULL, 10, cur_item);
            int cur_item_str_len = strlen(cur_item_str);
            MPI_Send(&cur_item_str_len, 1, MPI_INT, 0, SPECIAL_TAG, MPI_COMM_WORLD);
            MPI_Send(cur_item_str, cur_item_str_len, MPI_CHAR, 0, SPECIAL_TAG, MPI_COMM_WORLD);
        }
    }

    if (rank != size-1) {
        MPI_SEND_MPZ(cur_item, rank+1, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        mpz_t tmp_res;
        
        for (int proc = 1; proc < size; ++proc) {
            char* tmp_res_str = (char*) calloc(10, sizeof(char));
            MPI_RECV_MPZ(tmp_res_str, proc, MPI_COMM_WORLD, &status);
            mpz_init_set_str(tmp_res, tmp_res_str, 10);
            free(tmp_res_str);
            mpz_add(rank_res, rank_res, tmp_res);
        }
        char* res_str = mpz_get_str(NULL, 10, rank_res);
        printf("Process [%d], res is %s\n", rank, res_str);
        mpz_init_set_str(res, res_str, 10);

        int last_factor_str_len = 0;
        MPI_Recv(&last_factor_str_len, 1, MPI_INT, size-1, SPECIAL_TAG, MPI_COMM_WORLD, &status);
        char* last_factor_str = (char*) calloc(last_factor_str_len, sizeof(char));
        MPI_Recv(last_factor_str, last_factor_str_len, MPI_CHAR, size-1, SPECIAL_TAG, MPI_COMM_WORLD, &status);
        mpf_init_set_str(last_factor, last_factor_str, 10);
        printf("Process [%d], last factor is %s\n", rank, last_factor_str);
        free(last_factor_str);

        mpf_init_set_str(total_res, res_str, 10);
        mpf_div(total_res, total_res, last_factor);

        FILE* out;
        const char* file_name = "result.txt";
        if ((out = fopen(file_name, "w")) == NULL) {
            perror("[-] File open error.");
            return 1;
        
        }
        mpf_out_str(out, 10, 0, total_res);

        fclose(out);

        // mpf_out_str(stdout, 10, 0, total_res);
    }

    MPI_Finalize();

    return 0;
}