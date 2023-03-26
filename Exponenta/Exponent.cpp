#include <iostream>
#include <cmath>
#include "mpi.h"

// #define DEBUG

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

int main(int argc, char* argv[]) {

    int size = 0;
    int rank = 0;
    int res = 0;
    double total_res = 0.0;
    int last_factor = 0;

    MPI_Status status;

    if (argc != 2) {
        fprintf(stderr, "[-] Usage %s K\n", argv[0]);
        return 1;
    }
    // количество знаков после запятой
    unsigned long K = atoi(argv[1]);
    long long N = Newtons_M(K, 2.0);

    printf("To get %lu accuracy we have to have %lld items\n", K, N);

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

    int cur_item = N - rank_start + 1; // начальный элемент ряда
    int cur_num = N - rank_start + 1;
    int rank_res = N - rank_start + 1; // частичная сумма каждого поцесса

    for (int i = rank_start; i <= rank_end-1; ++i) {
        cur_num = cur_num - 1;
        cur_item = cur_item * cur_num;
        rank_res += cur_item;
    }

    int factor = 0;
    if (rank != 0) {
        MPI_Recv(&factor, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        rank_res *= factor;
        cur_item *= factor;
        MPI_Send(&rank_res, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        if (rank == size-1) {
            MPI_Send(&cur_item, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    }

    #ifdef DEBUG
    printf("proc [%d] size %d -- in range [%d, %d] rank res is %d\n", rank, size, rank_start, rank_end, rank_res);
    #endif //DEBUG

    if (rank != size-1) {
        MPI_Send(&cur_item, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        int tmp_res;
        res = rank_res;
        for (int proc = 1; proc < size; ++proc) {
            MPI_Recv(&tmp_res, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            res += tmp_res;
        }

        MPI_Recv(&last_factor, 1, MPI_INT, size-1, 1, MPI_COMM_WORLD, &status);
        total_res = static_cast<double>(res) / static_cast<double>(last_factor);
    }

    MPI_Finalize();

    printf("last factor is %d\n", last_factor);
    printf("result is %d\n", res);
    printf("result is %f\n", total_res);

    return 0;
}