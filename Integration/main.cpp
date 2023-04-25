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

#define DEBUG
#define STACK_LIMIT 10 // верхняя граница на количество записей в стеке
#define TRANSMIT_SIZE 6 // столько записей переносим за раз из локального стека в глобальный

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
    pthread_mutex_t* g_mutex; // мьютекс (нужен для обращения к глобальной переменной res)
    volatile double* res; // глобальная (для процессов) переменная, хранящая результат работы программы

    sem_t* glob_sem; // семафор (нужен для отслживания окончания работы программы)
    std::stack<std::unordered_map<std::string, double>>* glob_stack; // указатель на глобальный стек (нужен для делигирования работы на другие проессы в случае необходимости)

    double A; // начало участка, обрабатываемого процессом
    double B; // конец участка, обрабатываемого процессом
} arg_t;

// Функция перемещает ОДНУ запись из одного стека в другой
void TransmitOneNode(
    std::stack<std::unordered_map<std::string, double>>* dst_stack,
    std::stack<std::unordered_map<std::string, double>>* src_stack
)
{
    std::unordered_map<std::string, double> node = src_stack->top();
    src_stack->pop();
    dst_stack->push(node);
}

void Global2Local(
    std::stack<std::unordered_map<std::string, double>>* local_stack,
    std::stack<std::unordered_map<std::string, double>>* global_stack,
    pthread_mutex_t* mutex, sem_t* sem
)
{
    if (global_stack->size() < STACK_LIMIT) {
        while(global_stack->size() != 0) {
            pthread_mutex_lock(mutex);
            TransmitOneNode(local_stack, global_stack);
            pthread_mutex_unlock(mutex);
        }
        /*
        В этом случае нам не надо менять семафор, так как мы очистили глобальный стек и 
        заполнили локальный
        */
    }
    else {
        for (size_t i = 0; i < STACK_LIMIT; ++i) {
            pthread_mutex_lock(mutex);
            TransmitOneNode(local_stack, global_stack);
            pthread_mutex_unlock(mutex);
        }
        /*
        В этом случае нам надо инкрементировать семафор, так как мы заполнили локальный стек,
        при этом в глобальном стеке у нас все еще что-то есть.
        */
        if (sem_post(sem) == -1) {
            perror("sem_post");
            exit(1);
        }
    }
}

void Local2Global(
    std::stack<std::unordered_map<std::string, double>>* local_stack,
    std::stack<std::unordered_map<std::string, double>>* global_stack,
    pthread_mutex_t* mutex, sem_t* sem
)
{
    for (size_t i = 0; i < TRANSMIT_SIZE; ++i) {
        pthread_mutex_lock(mutex);
        TransmitOneNode(global_stack, local_stack);
        pthread_mutex_unlock(mutex);
    }
    if (global_stack->size() - TRANSMIT_SIZE == 0) {
        // если глобальный стек был пуст, то надо инкрементировать семафор
        if (sem_post(sem) == -1) {
            perror("sem_post");
            exit(1);
        }
    }
}

void Calculate(
    std::stack<std::unordered_map<std::string, double>>* local_stack, 
    double A, double B, double fa, double fb, double Sab,
    volatile double* local_res, sem_t* global_sem,
    std::stack<std::unordered_map<std::string, double>>* global_stack, pthread_mutex_t* mutex
) {
    double eps = 1e-3; // точность вычисления
    double C = (A + B) / 2;
    double fc = f(C);
    double Sac = Trapez(A, C, fa, fc);
    double Scb = Trapez(C, B, fc, fb);

    if (abs(Sac+Scb - Sab) < eps) {
        *local_res += Sab;

        if (local_stack->size() != 0) {
            // если в локальном стеке есть записи, берем запись с вершины
            std::unordered_map<std::string, double> cur_top = local_stack->top();
            local_stack->pop();
            // производим выычисление промежутка
            Calculate(
                local_stack,   cur_top["A"],  cur_top["B"], 
                cur_top["fa"], cur_top["fb"], cur_top["Sab"], local_res, global_sem, global_stack,
                mutex
            );
        }
        else {
            /*
            Eсли локальный стек пуст, декрементируем глобальный семафор (индикатор того, что процесс выполнил свою работу) 
            и начинаем мониторить глобальный стек.
            */
            int sem_value;
            size_t counter = 0;
            if (sem_wait(global_sem) == -1) {
                perror("sem_wait");
                exit(1);
            }
            if (sem_getvalue(global_sem, &sem_value) == -1) {
                perror("sem_getvalue");
                exit(1);
            }

            /*
            Значение семафора уменьшается на единицу каждый раз, когда какой либо из стеков программы
            (глобальный или локальный становится пустым). Значение семафора равное 0 означает, что
            все стеки пусты и больше нет необработанных диапазонов. Это значит, что программу можно завершать.
            */
            while (sem_value != 0) {
                // чекаем глобальный стек раз в 3 такта
                if (counter != 3) {
                    counter++;
                    continue;
                }

                counter = 0;
                if (global_stack->size() != 0) {
                    // если в глобальном стеке есть записи, то переносим их в локальный стек и начинаем обрабатывать
                    Global2Local(local_stack, global_stack, mutex, global_sem);
                    // берем первую запись с локального стека и начинаем ее обрабатывать
                    std::unordered_map<std::string, double> cur_top = local_stack->top();
                    Calculate(
                        local_stack,   cur_top["A"],  cur_top["B"], 
                        cur_top["fa"], cur_top["fb"], cur_top["Sab"], local_res, global_sem, global_stack,
                        mutex
                    );
                }
            }
        }
    } else {
        // правую часть промежутка кладем в стек
        local_stack->push(MakeNode(C, B, fc, fb, Scb));

        // Случай переполнения локального стека
        if (local_stack->size() > STACK_LIMIT) {
            // переносим часть записей из локального стека в глобальный
            Local2Global(local_stack, global_stack, mutex, global_sem);
        }

        // с левой частью продолжаем работать
        Calculate(local_stack, A, C, fa, fc, Sac, local_res, global_sem, global_stack, mutex);
    }
}

void* ThreadFunction(void* arg) {
    arg_t* args = (arg_t*) arg;
    std::stack<std::unordered_map<std::string, double>> local_stack; // локальный стек
    volatile double local_res = 0; // частичная сумма потока
    #ifdef DEBUG
    printf("thread works on space [%lf, %lf]\n", args->A, args->B);
    #endif //DEBUG

    double fa = f(args->A);
    double fb = f(args->B);

    Calculate(
        &local_stack, args->A, args->B, fa, fb, 
        Trapez(args->A, args->B, fa, fb), &local_res, args->glob_sem, args->glob_stack, args->g_mutex
    );

    pthread_mutex_lock(args->g_mutex);
    *(args->res) += local_res;
    pthread_mutex_unlock(args->g_mutex);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "[-] Usage %s n_of_threads\n", argv[0]);
        return -1;
    }

    // начало и конец участка интегрирования
    double A = 0;
    double B = 5;

    // количество потоков
    unsigned int thread_amount = atoi(argv[1]);

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