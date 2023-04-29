#include <stdio.h>
#include <stdlib.h>

// includes for UNIX pthread
#include <pthread.h>
#include <errno.h>

void* ThreadFunction(void* rank) {
    printf("Thread [%d] Hello World!\n", *(unsigned int*)(rank));
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[-] Usage %s n_of_threads\n", argv[0]);
        return -1;
    }
    // количество потоков
    unsigned int thread_amount = atoi(argv[1]);

    pthread_t thread_id[thread_amount]; 
    unsigned int thread_rank[thread_amount];
    for (unsigned int i = 0; i < thread_amount; ++i) {
        thread_rank[i] = i;
    }

    for (unsigned int i = 0; i < thread_amount; ++i) {
        if (errno = pthread_create(&thread_id[i], NULL, ThreadFunction, &thread_rank[i])) {
            perror("pthread_create");
            return 1;
        }
    }

    // wait for a thread[thread_id]
    for (unsigned int i = 0; i < thread_amount; ++i) {
        pthread_join(thread_id[i], NULL);
    }

    return 0;
}