#include <stdio.h>
#include <stdlib.h>

// includes for UNIX pthread
#include <pthread.h>
#include <errno.h>

// includes for POSIX semapores
#include <fcntl.h> /* Defines O_* constants */
#include <sys/stat.h> /* Defines mode constants */
#include <semaphore.h>

// стуктура аргумента, передаваемого, как параметр для каждого потока
typedef struct {
    size_t id;                // id потока (надо для отладки)
    sem_t *g_sem;    
} arg_t;

void* ThreadFunction(void* arg) {
    arg_t* args = (arg_t*) arg;
    size_t rank = args->id;
    int sem_value;
    if (sem_getvalue(args->g_sem, &sem_value) == -1) {
        perror("sem_getvalue");
        exit(1);
    }

    while (sem_value != rank) {
        if (sem_getvalue(args->g_sem, &sem_value) == -1) {
            perror("sem_getvalue");
            exit(1);
        }
    }

    printf("Thread [%ld] global var value is %d\n", rank, sem_value);
    if (sem_post(args->g_sem) == -1) {
        perror("sem_post");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "[-] Usage %s n_of_threads N\n", argv[0]);
        return -1;
    }
    // количество потоков
    unsigned int thread_amount = atoi(argv[1]);

    pthread_t thread_id[thread_amount]; 
    const char* glob_sem_name = "glob_sem";
    unsigned int glob_sem_init = 0;
    // Создаем глобальный именованный семафор
    sem_t* sem;
    sem = sem_open(glob_sem_name, O_CREAT | O_EXCL, O_RDWR, glob_sem_init);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return -1;
    }

    arg_t thread_args[thread_amount];
    // в цикле задаем аргументы каждому процессу
    for (size_t i = 0; i < thread_amount; ++i) {
        thread_args[i].id = i;
        thread_args[i].g_sem = sem;
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
    return 0;
}