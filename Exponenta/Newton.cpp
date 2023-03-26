#include <iostream>
#include <cmath>

long long Newtons_M(unsigned long K, double init_x) {
    unsigned steps = 0;
    double accuracy = 1e-6;
    double x_cur = init_x;
    double x_prev = 0;

    while(abs(x_cur - x_prev) > accuracy) {
        x_prev = x_cur;
        steps+=1;
        x_cur = x_prev - (x_prev*log(x_prev) - x_prev - K*log(10)) / (log(x_prev));
    }

    return static_cast<long long>(std::floor(x_cur));
}

int main(int argc, char* argv[]) {
    unsigned long K = 1000000;
    double init_val = 2.0;
    std::cout << Newtons_M(K, init_val) << std::endl;
    return 0;
}