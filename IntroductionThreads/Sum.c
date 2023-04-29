#include <stdio.h>
#include <stdlib.h>

// includes for UNIX pthread
#include <pthread.h>
#include <errno.h>

// стуктура аргумента, передаваемого, как параметр для каждого потока
typedef struct {
    int N;
    size_t id;                // id потока (надо для отладки)
    pthread_mutex_t* g_mutex; // мьютекс (нужен для обращения к глобальной переменной res)
    volatile double* res;     // глобальная (для процессов) переменная, хранящая результат работы программы
    unsigned int threads_amount;
} arg_t;

double Sum(int start, int end) {
    double res = 0;
    for (double i = start; i <= end; ++i) {
        res += 1/i;
    }
    return res;
}

void* ThreadFunction(void* arg) {
    arg_t* args = (arg_t*) arg;
    int N = args->N;
    unsigned int size = args->threads_amount;
    size_t rank = args->id;

    int n_range = N / size;
    int mod_size = N % size;

    double rank_res = 0;

    int rank_start, rank_end = 0;

    if (rank < mod_size) {
        rank_start = rank*(n_range + 1) + 1;
        rank_end = rank_start + n_range;
    }
    else {
        rank_start = rank*n_range + 1 + mod_size;
        rank_end = rank_start + n_range - 1;
    }

    rank_res = Sum(rank_start, rank_end);
    printf("Thread [%ld] sum space [%d, %d]. Rank res is %lf\n", rank, rank_start, rank_end, rank_res);

    pthread_mutex_lock(args->g_mutex);
    *(args->res) += rank_res;
    pthread_mutex_unlock(args->g_mutex);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "[-] Usage %s n_of_threads N\n", argv[0]);
        return -1;
    }
    // количество потоков
    unsigned int thread_amount = atoi(argv[1]);
    int N = atoi(argv[2]);
    volatile double res;

    pthread_t thread_id[thread_amount]; 
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // mutex init

    arg_t thread_args[thread_amount];
    // в цикле задаем аргументы каждому процессу
    for (size_t i = 0; i < thread_amount; ++i) {
        thread_args[i].g_mutex = &mutex;
        thread_args[i].id = i;
        thread_args[i].N = N;
        thread_args[i].res = &res;
        thread_args[i].threads_amount = thread_amount;
    }

    for (unsigned int i = 0; i < thread_amount; ++i) {
        if (errno = pthread_create(&thread_id[i], NULL, ThreadFunction, &thread_args[i])) {
            perror("pthread_create");
            return 1;
        }
    }

    // wait for a thread[thread_id]
    for (unsigned int i = 0; i < thread_amount; ++i) {
        pthread_join(thread_id[i], NULL);
    }

    printf("Total result is %lf\n", res);
    return 0;
}