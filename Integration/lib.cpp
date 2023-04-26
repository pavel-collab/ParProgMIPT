#include <iostream>
#include <unordered_map>
#include <stack>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include <cmath>

#include "lib.hpp"

#define DEBUG
#define LOG

const char* thread0_file = "thread0.log";
const char* thread1_file = "thread1.log";
const char* global_file  = "global.log";

double f(double x) {
    // return x*x;
    return cos(20*x);
}

// --------------------------------------------------------------------------------------------
void PrintStackTop(FILE* stream, std::stack<std::unordered_map<std::string, double>>& stack) {
    fprintf(stream, "Stack top\n");
    fprintf(stream, "\tA   = %lf\n", stack.top()["A"]  );
    fprintf(stream, "\tB   = %lf\n", stack.top()["B"]  );
    fprintf(stream, "\tfa  = %lf\n", stack.top()["fa"] );
    fprintf(stream, "\tfb  = %lf\n", stack.top()["fb"] );
    fprintf(stream, "\tSab = %lf\n", stack.top()["Sab"]);
}

void PrintStack(FILE* stream, std::stack<std::unordered_map<std::string, double>>& stack) {
    //копируем в локальную переменную, чтобы не портить стек
    std::stack<std::unordered_map<std::string, double>> copy_stack = stack;
    while(copy_stack.size() != 0) {
        PrintStackTop(stream, copy_stack);
        copy_stack.pop();
    }
}

void Push2StackLog(
    size_t id, std::stack<std::unordered_map<std::string, double>>* stack, sem_t* sem
) {
    FILE* fd = NULL;
    if (id == 0)
        fd = fopen(thread0_file, "a");
    else if (id == 1)
        fd = fopen(thread1_file, "a");
    else if (id == 69) 
        fd = fopen(global_file, "a");
    if (fd) {
        fprintf(fd, "======================================================================================\n");
        fprintf(fd, "There was a push to stack.\n");
        fprintf(fd, "Stack size is %ld\n", stack->size());
        int tmp_sem_val = 0;
        sem_getvalue(sem, &tmp_sem_val);
        fprintf(fd, "Semaphor value is %d\n", tmp_sem_val);
        PrintStack(fd, *stack);
    }
    fclose(fd);
}
void PopStackLog( 
    size_t id, std::stack<std::unordered_map<std::string, double>>* stack, sem_t* sem
) {
    FILE* fd = NULL;
        if (id == 0)
        fd = fopen(thread0_file, "a");
    else if (id == 1)
        fd = fopen(thread1_file, "a");
    else if (id == 69) 
        fd = fopen(global_file, "a");
    if (fd) {
        fprintf(fd, "======================================================================================\n");
        fprintf(fd, "There was a pop from stack.\n");
        fprintf(fd, "Stack size is %ld\n", stack->size());
        int tmp_sem_val = 0;
        sem_getvalue(sem, &tmp_sem_val);
        fprintf(fd, "Semaphor value is %d\n", tmp_sem_val);
        PrintStack(fd, *stack);
    }
    fclose(fd);
}
// --------------------------------------------------------------------------------------------

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
    if (global_stack->size() <= STACK_LIMIT) {
        size_t curent_nodes_amount = global_stack->size();
        for (size_t i = 0; i < curent_nodes_amount; ++i) {
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

    if (std::abs(Sac+Scb - Sab) < eps) {
        *local_res += Sab;
        // printf("Thread [%ld] (accept range [%lf, %lf]) Sac = %lf, Scb = %lf, Sab = %lf\n", id, A, B, Sac, Scb, Sab);

        if (local_stack->size() != 0) {
            // если в локальном стеке есть записи, берем запись с вершины
            std::unordered_map<std::string, double> cur_top = local_stack->top();
            local_stack->pop();

            #ifdef LOG
            PopStackLog(id, local_stack, sem);
            #endif //LOG

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
            // локальный стек пуст -> начинаем мониторить глобальный стек
            size_t counter = 0;
            int sem_value;
            if (sem_getvalue(sem, &sem_value) == -1) {
                perror("sem_getvalue");
                exit(1);
            }

            /*
            Значение семафора уменьшается на единицу каждый раз, когда какой либо из стеков программы
            (глобальный или локальный становится пустым). Значение семафора равное 0 означает, что
            все стеки пусты и больше нет необработанных диапазонов. Это значит, что программу можно завершать.
            */
            while (sem_value != 0) {
                sem_getvalue(sem, &sem_value);
                // printf("thread [%ld] curent sem value is %d\n", id, sem_value);
                // чекаем глобальный стек раз в 3 такта
                if (counter != 3) {
                    counter++;
                    continue;
                }

                counter = 0;
                if (global_stack->size() != 0) {
                    // если в глобальном стеке есть записи, то переносим их в локальный стек и начинаем обрабатывать
                    Global2Local(local_stack, global_stack, mutex, sem);
                    // printf("Transmit data from global stack to local thread [%ld]\n", id);

                    #ifdef LOG
                    Push2StackLog(id, local_stack, sem);
                    PopStackLog(69, global_stack, sem);
                    #endif //LOG

                    // берем первую запись с локального стека и начинаем ее обрабатывать
                    std::unordered_map<std::string, double> cur_top = local_stack->top();
                    local_stack->pop();
                    std::unordered_map<std::string, double> nd = MakeNode(
                        cur_top["A"],  cur_top["B"], cur_top["fa"], cur_top["fb"], cur_top["Sab"]
                    );
                    Calculate(
                        id, eps, local_stack, global_stack, nd, local_res, sem, mutex 
                    );
                }
            }
            return;
        }
    } else {
        // правую часть промежутка кладем в стек
        local_stack->push(MakeNode(C, B, fc, fb, Scb));

        #ifdef LOG
        Push2StackLog(id, local_stack, sem);
        #endif //LOG

        // Случай переполнения локального стека
        if (local_stack->size() > STACK_LIMIT) {
            // переносим часть записей из локального стека в глобальный
            Local2Global(local_stack, global_stack, mutex, sem);
            
            #ifdef LOG
            PopStackLog(id, local_stack, sem);
            Push2StackLog(69, global_stack, sem);
            #endif //LOG
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
    Calculate(args->id, args->eps, &local_stack, args->glob_stack, initial_node, &local_res, args->glob_sem, args->g_mutex);

    printf("thread [%ld] local res is %lf\n", args->id, local_res);

    pthread_mutex_lock(args->g_mutex);
    *(args->res) += local_res;
    pthread_mutex_unlock(args->g_mutex);
    return NULL;
}