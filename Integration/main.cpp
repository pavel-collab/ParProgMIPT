#include <iostream>
#include <cmath>
#include <stack>
#include <unordered_map>

// includes for UNIX pthread
#include <pthread.h>
#include <errno.h>

// includes for POSIX semapores
#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <semaphore.h>

// man pthread_create
//! Линковать с динамической библиотекой -pthread

double f(double x) {
    return x*x;
}

std::unordered_map<std::string, double> MakeNode(double A, double B, double fa, double fb, double Sab) {
    std::unordered_map<std::string, double> node;
    node.insert({"A", A});
    node.insert({"B", B});
    node.insert({"fa", fa});
    node.insert({"fb", fb});
    node.insert({"Sab", Sab});
    return node;
}

double Trapez(double A, double B, double fa, double fb) {
    return (fa + fb)*(B - A) / 2;
}

// стуктура аргумента, передаваемого, как параметр для каждого потока
typedef struct {
    pthread_mutex_t g_mutex; // мьютекс (нужен для обращения к глобальной переменной res)
    volatile double* res; // глобальная (для процессов) переменная, хранящая результат работы программы

    sem_t* glob_sem; // семафор (нужен для отслживания окончания работы программы)
    std::stack<std::unordered_map<std::string, double>>* glob_stack; // указатель на глобальный стек (нужен для делигирования работы на другие проессы в случае необходимости)
} arg_t;

void* ThreadFunction(void* arg) {
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "[-] Usage %s n_of_threads\n", argv[0]);
        return -1;
    }

    // количество потоков
    unsigned int thread_amount = atoi(argv[1]);

    const char* glob_sem_name = "glob_sem";
    /*
    Начальное значение семафора равно количеству стеков. 
    Каждый поток имеет локальный стек + один глобальный стек.
    */
    unsigned int glob_sem_init = 1 + thread_amount;
    // Создаем глобальный именованный семафор
    sem_t* sem;
    sem = sem_open(glob_sem_name, O_CREAT | O_EXCL, O_RDWR, glob_sem_init);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return -1;
    }

    volatile double res = 0; // глобальная (для процессов) переменная, хранящая результат работы программы
    std::stack<std::unordered_map<std::string, double>> global_stack;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // mutex init

    arg_t thread_args[thread_amount];
    // в цикле задаем аргументы каждому процессу
    for (size_t i = 0; i < thread_amount; ++i) {
        thread_args[i].g_mutex = mutex;
        thread_args[i].glob_sem = sem;
        thread_args[i].res = &res;

        thread_args[i].glob_stack = &global_stack;
    }

    pthread_t thread_id[thread_amount]; 

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

    // ===========================================================================================
    if (sem_close(sem) == -1) {
        perror("sem_close");
        return -1;
    }
    /*
    When we close the semaphore, we don't delet it.
    So, we have to use a special method to unlink it.
    */
    if (sem_unlink(glob_sem_name) == -1) {
        perror("sem_unlink");
        return -1;
    }
    // ===========================================================================================
}