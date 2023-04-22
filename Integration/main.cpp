#include <iostream>
#include <stack>
#include <unordered_map>

double f(double x) {
    return x*x;
}

void PrintStackTop(std::stack<std::unordered_map<std::string, double>>& stack) {
    std::cout << "Stack top" << std::endl;
    std::cout << "\tA  = " << stack.top()["A"] << std::endl;
    std::cout << "\tB  = " << stack.top()["B"] << std::endl;
    std::cout << "\tfa = " << stack.top()["fa"] << std::endl;
    std::cout << "\tfb = " << stack.top()["fb"] << std::endl << std::endl;
}

void PrintStack(std::stack<std::unordered_map<std::string, double>>& stack) {
    //копируем в локальную переменную, чтобы не портить стек
    std::stack<std::unordered_map<std::string, double>> local_stack = stack;
    while(local_stack.size() != 0) {
        PrintStackTop(local_stack);
        local_stack.pop();
    }
}

std::unordered_map<std::string, double> MakeNode(double A, double B, double fa, double fb) {
    std::unordered_map<std::string, double> node;
    node.insert({"A", A});
    node.insert({"B", B});
    node.insert({"fa", fa});
    node.insert({"fb", fb});
    return node;
}

int main(int argc, char* argv[]) {
    std::stack<std::unordered_map<std::string, double>> stack;

    double A = 1;
    double B = 5;
    double C = 0;

    stack.push(MakeNode(A, B, f(A), f(B)));
    C = (A+B)/2;
    stack.push(MakeNode(A, C, f(A), f(C)));
    stack.push(MakeNode(C, B, f(C), f(B)));

    std::cout << "top:" << std::endl;
    PrintStackTop(stack);
    stack.pop();
    std::cout << "stack after pop:" << std::endl;
    PrintStack(stack);
    std::cout << "stack curent top:" << std::endl;
    PrintStackTop(stack);
}