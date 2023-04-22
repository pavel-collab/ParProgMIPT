#ifndef _STACK_HPP_
#define _STACK_HPP_

#include <vector>
#include <iostream>
#include <unordered_map>

class Stack {
public:
    Stack() = default;
    // Инициализация с первым "элементом" стека
    Stack(double A, double B, double fa, double fb) {
        this->m_A[idx] = A;
        this->m_B[idx] = B;
        this->m_fa[idx] = fa;
        this->m_fb[idx] = fb;
        this->idx++;
    }
    ~Stack() = default;

    void Push(double A, double B, double fa, double fb) {
        this->m_A.emplace_back(A);
        this->m_B.emplace_back(B);
        this->m_fa.emplace_back(fa);
        this->m_fb.emplace_back(fb);
        this->idx++;
    }

    // void Top()

    void Pop(double* buf) {
        this->idx--;
        buf[0] = this->m_A[idx];
        buf[1] = this->m_B[idx];
        buf[2] = this->m_fa[idx];
        buf[3] = this->m_fb[idx];
        this->m_A.pop_back();
        this->m_B.pop_back();
        this->m_fa.pop_back();
        this->m_fb.pop_back();
    }

    unsigned int Size() {
        return this->idx;
    }

    void PrintContent() {
        for (unsigned int i = 0; i < this->idx; ++i) {
            this->PrintElement(i);
        }
    }

private:
    std::vector<double> m_A;
    std::vector<double> m_B;
    std::vector<double> m_fa;
    std::vector<double> m_fb;

    unsigned int idx = 0;

    void PrintElement(unsigned int i) {
        printf("Stack [%u]\n", i);
        printf("\tA[%u]  = %lf\n", i, this->m_A[i]);
        printf("\tB[%u]  = %lf\n", i, this->m_B[i]);
        printf("\tfa[%u] = %lf\n", i, this->m_fa[i]);
        printf("\tfb[%u] = %lf\n", i, this->m_fb[i]);
        printf("\n");
    }
};

#endif //_STACK_HPP_