#include <iostream>
#include <cmath>
#include <stack>
#include <unordered_map>
#include <cmath>

// includes for UNIX pthread
#include <pthread.h>
#include <errno.h>

// includes for POSIX semapores
#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <semaphore.h>

#include "lib.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[-] Usage %s n_of_threads\n", argv[0]);
        return -1;
    }

    // начало и конец участка интегрирования
    double A = 0.005;
    double B = 5;

    // количество потоков
    unsigned int thread_amount = atoi(argv[1]);
    double eps = 1e-3; // точность вычисления интеграла
    if (argc == 3) 
        eps = atof(argv[2]);

    const char* glob_sem_name = "glob_sem";
    /*
    Начальное значение семафора равно количеству стеков. 
    Каждый поток имеет локальный стек + один глобальный стек.
    Но так как глобальный стек изначально пуст, начальное значение семафора равно
    количеству потоков.
    */
    unsigned int glob_sem_init = thread_amount;
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
        thread_args[i].eps = eps;
        thread_args[i].id = i;
        thread_args[i].g_mutex = &mutex;
        thread_args[i].glob_sem = sem;
        thread_args[i].res = &res;

        thread_args[i].glob_stack = &global_stack;

        thread_args[i].A = (B - A)*i / thread_amount;
        thread_args[i].B = (B - A)*(i+1) / thread_amount;
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

    std::cout << "total integration result is " << res << std::endl;

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