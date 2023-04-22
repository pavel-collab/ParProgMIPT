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
        this->Push(A, B, fa, fb);
    }
    ~Stack() = default;

    void Push(double A, double B, double fa, double fb) {
        std::unordered_map<std::string, double> node;
        node.insert({"A", A});
        node.insert({"B", B});
        node.insert({"fa", fa});
        node.insert({"fb", fb});
        this->m_stack.emplace_back(node);
        this->idx++;
    }

    std::unordered_map<std::string, double> Top() {
        return this->m_stack[this->idx-1];
    }

    void Pop() {
        this->idx--;
        this->m_stack.pop_back();
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
    std::vector<std::unordered_map<std::string, double>> m_stack;
    unsigned int idx = 0;

    void PrintElement(unsigned int i) {
        printf("Stack [%u]\n", i);
        std::unordered_map<std::string, double> node = this->Top();
        for (auto item : node) {
            std::cout << item.first << "[" << i << "] = " << item.second << std::endl;
        }
        printf("\n");
    }
};

#endif //_STACK_HPP_