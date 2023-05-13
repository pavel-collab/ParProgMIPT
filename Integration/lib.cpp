#include <iostream>
#include <unordered_map>
#include <stack>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include <cmath>

#include "lib.hpp"

// #define DEBUG

double f(double x) {
    // return x*x;
    // return cos(20*x);
    return cos(1/x);
}

double Trapez(double A, double B, double fa, double fb) {
    return (fa + fb)*(B - A) / 2;
}

std::unordered_map<std::string, double> MakeNode(
    double A, double B, double fa, double fb, double Sab
) {
    std::unordered_map<std::string, double> node;
    node.insert({"A", A});
    node.insert({"B", B});
    node.insert({"fa", fa});
    node.insert({"fb", fb});
    node.insert({"Sab", Sab});
    return node;
}

// Функция перемещает ОДНУ запись из одного стека в другой
void TransmitOneNode(
    std::stack<std::unordered_map<std::string, double>>* dst_stack,
    std::stack<std::unordered_map<std::string, double>>* src_stack
)
{
    if (src_stack->empty()) {
        std::cout << "[ERROR] src stack is empty" << std::endl;
        return;
    }
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
    if (global_stack->size() == 0) {
        printf("[ERROR] stack is eampty\n");
        return;
    }

    pthread_mutex_lock(mutex);
    if (global_stack->size() <= STACK_LIMIT) {
        size_t curent_nodes_amount = global_stack->size();
        for (size_t i = 0; i < curent_nodes_amount; ++i) {
            TransmitOneNode(local_stack, global_stack);
        }
        /*
        В этом случае нам не надо менять семафор, так как мы очистили глобальный стек и 
        заполнили локальный
        */
    }
    else {
        for (size_t i = 0; i < STACK_LIMIT; ++i) {
            TransmitOneNode(local_stack, global_stack);
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
    pthread_mutex_unlock(mutex);
}

void Local2Global(
    std::stack<std::unordered_map<std::string, double>>* local_stack,
    std::stack<std::unordered_map<std::string, double>>* global_stack,
    pthread_mutex_t* mutex, sem_t* sem
)
{
    pthread_mutex_lock(mutex);
    for (size_t i = 0; i < TRANSMIT_SIZE; ++i) {
        TransmitOneNode(global_stack, local_stack);
    }
    pthread_mutex_unlock(mutex);
    if (global_stack->size() - TRANSMIT_SIZE == 0) {
        // если глобальный стек был пуст, то надо инкрементировать семафор
        if (sem_post(sem) == -1) {
            perror("sem_post");
            exit(1);
        }
    }
}

void Calculate(
    size_t id, double eps,
    std::stack<std::unordered_map<std::string, double>>* local_stack, 
    std::stack<std::unordered_map<std::string, double>>* global_stack,
    std::unordered_map<std::string, double> node,
    volatile double* local_res, sem_t* sem, pthread_mutex_t* mutex
) {
    double A   = node["A"];
    double B   = node["B"];
    double Sab = node["Sab"];
    double fa  = node["fa"];
    double fb  = node["fb"];
    double C = (A + B) / 2;
    double fc = f(C);
    double Sac = Trapez(A, C, fa, fc);
    double Scb = Trapez(C, B, fc, fb);
    // printf("Thread [%ld]. Entre to Calculate function.\n", id);

    if (std::abs((Sac+Scb) - Sab) < eps) {
        *local_res += Sab;
        printf("Thread [%ld] (accept range [%lf, %lf]) Sac = %lf, Scb = %lf, Sab = %lf\n", id, A, B, Sac, Scb, Sab);

        if (local_stack->size() != 0) {
            printf("Thread [%ld]. Local stack access\n", id);
            // если в локальном стеке есть записи, берем запись с вершины
            std::unordered_map<std::string, double> cur_top = local_stack->top();
            local_stack->pop();

            // производим выычисление промежутка
            std::unordered_map<std::string, double> nd = MakeNode(
                cur_top["A"],  cur_top["B"], cur_top["fa"], cur_top["fb"], cur_top["Sab"]
            );
            Calculate(
                id, eps, local_stack, global_stack, nd, local_res, sem, mutex 
            );
        }
        else {
            /*
            Eсли локальный стек пуст, декрементируем глобальный семафор (индикатор того, что процесс выполнил свою работу) 
            */
            if (sem_wait(sem) == -1) {
                perror("sem_wait");
                exit(1);
            }
            printf("Thread [%ld]. Semaphore access\n", id);
            // локальный стек пуст -> начинаем мониторить глобальный стек
            size_t counter = 0;
            int sem_value;
            if (sem_getvalue(sem, &sem_value) == -1) {
                perror("sem_getvalue");
                exit(1);
            }

            /*
            Значение семафор
            а уменьшается на единицу каждый раз, когда какой либо из стеков программы
            (глобальный или локальный становится пустым). Значение семафора равное 0 означает, что
            все стеки пусты и больше нет необработанных диапазонов. Это значит, что программу можно завершать.
            */
            while (sem_value != 0) {
                sem_getvalue(sem, &sem_value);
                printf("thread [%ld] curent sem value is %d\n", id, sem_value);
                // чекаем глобальный стек раз в 3 такта
                if (counter != 3) {
                    counter++;
                    continue;
                }

                counter = 0;
                if (global_stack->size() != 0) {
                    printf("Thread [%ld]. Global stack access\n", id);
                    // если в глобальном стеке есть записи, то переносим их в локальный стек и начинаем обрабатывать
                    Global2Local(local_stack, global_stack, mutex, sem);
                    printf("Transmit data from global stack to local thread [%ld]\n", id);

                    if (local_stack->size() != 0) {
                        // берем первую запись с локального стека и начинаем ее обрабатывать
                        std::unordered_map<std::string, double> cur_top = local_stack->top();
                        local_stack->pop();
                        printf("Thread [%ld]. Local stack pop.\n", id);
                        std::unordered_map<std::string, double> nd = MakeNode(
                            cur_top["A"],  cur_top["B"], cur_top["fa"], cur_top["fb"], cur_top["Sab"]
                        );
                        Calculate(
                            id, eps, local_stack, global_stack, nd, local_res, sem, mutex 
                        );
                    }
                }
            }
            return;
        }
    } else {
        // правую часть промежутка кладем в стек
        local_stack->push(MakeNode(C, B, fc, fb, Scb));
        printf("Thread [%ld]. Local stack push.\n", id);

        // Случай переполнения локального стека
        if (local_stack->size() > STACK_LIMIT) {
            // переносим часть записей из локального стека в глобальный
            printf("Thread [%ld]. Overflow local stack. Transmit to global\n", id);
            Local2Global(local_stack, global_stack, mutex, sem);
        }

        // с левой частью продолжаем работать
        std::unordered_map<std::string, double> nd = MakeNode(A, C, fa, fc, Sac);
        Calculate(id, eps, local_stack, global_stack, nd, local_res, sem, mutex);
    }
}

void* ThreadFunction(void* arg) {
    arg_t* args = (arg_t*) arg;
    std::stack<std::unordered_map<std::string, double>> local_stack; // локальный стек
    volatile double local_res = 0; // частичная сумма потока
    #ifdef DEBUG
    printf("thread [%zd] works on space [%lf, %lf]\n", args->id, args->A, args->B);
    #endif //DEBUG

    double fa = f(args->A);
    double fb = f(args->B);

    std::unordered_map<std::string, double> initial_node = MakeNode(
        args->A, args->B, fa, fb, Trapez(args->A, args->B, fa, fb)
    );
    printf("Thread [%ld]. Thread function. Start to calculate.\n", args->id);
    Calculate(args->id, args->eps, &local_stack, args->glob_stack, initial_node, &local_res, args->glob_sem, args->g_mutex);

    printf("thread [%ld] local res is %lf\n", args->id, local_res);

    pthread_mutex_lock(args->g_mutex);
    *(args->res) += local_res;
    pthread_mutex_unlock(args->g_mutex);
    return NULL;
}

//! Текущее поведение программы возможно объяснить следующим образом:
//! за время printf() инициализируются некоторые сущности, 
//! к которым идет обращение и программа работает корректно.

//TODO: Постараться отследить обращения к возможно неиницелизированным сущностям
//TODO: вниметальнее посмотреть операции с семафором. Если программа спит, значит скорее всего она ждет, пока отпустят семафор.