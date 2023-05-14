#ifndef _LIB_HPP_
#define _LIB_HPP_

#include <iostream>
#include <unordered_map>
#include <stack>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

/*
От значения данных полей зависит частота обмена данными между глобальным
и локальным стеком. Так же, как следствие время работы программы.

STACK_LIMIT обязательно должен быть больше TRANSMIT_SIZE, иначе можно словить sigmentation fault
*/
#define STACK_LIMIT 4   // верхняя граница на количество записей в стеке
#define TRANSMIT_SIZE 3 // столько записей переносим за раз из локального стека в глобальный

// стуктура аргумента, передаваемого, как параметр для каждого потока
typedef struct {
    double eps;               // точность вычисления интеграла
    size_t id;                // id потока (надо для отладки)
    pthread_mutex_t* g_mutex; // мьютекс (нужен для обращения к глобальной переменной res)
    volatile double* res;     // глобальная (для процессов) переменная, хранящая результат работы программы

    sem_t* glob_sem;          // семафор (нужен для отслживания окончания работы программы)
    std::stack<std::unordered_map<std::string, double>>* glob_stack; // указатель на глобальный стек (нужен для делигирования работы на другие проессы в случае необходимости)

    double A; // начало участка, обрабатываемого процессом
    double B; // конец участка, обрабатываемого процессом
} arg_t;

double f(double x);

double Trapez(double A, double B, double fa, double fb);

std::unordered_map<std::string, double> MakeNode(
    double A, double B, double fa, double fb, double Sab
);

void TransmitOneNode(
    std::stack<std::unordered_map<std::string, double>>* dst_stack,
    std::stack<std::unordered_map<std::string, double>>* src_stack
);

void Global2Local(
    std::stack<std::unordered_map<std::string, double>>* local_stack,
    std::stack<std::unordered_map<std::string, double>>* global_stack,
    sem_t* sem
);

void Local2Global(
    std::stack<std::unordered_map<std::string, double>>* local_stack,
    std::stack<std::unordered_map<std::string, double>>* global_stack,
    sem_t* sem
);

void Calculate(
    size_t id, double eps,
    std::stack<std::unordered_map<std::string, double>>* local_stack, 
    std::stack<std::unordered_map<std::string, double>>* global_stack,
    std::unordered_map<std::string, double> node,
    volatile double* local_res, sem_t* sem, pthread_mutex_t* mutex
);

void* ThreadFunction(void* arg);

#endif //_LIB_HPP_