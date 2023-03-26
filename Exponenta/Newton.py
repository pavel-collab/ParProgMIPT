import math

def Newtons_M(K, init_x=2):
    steps = 0
    eps = 1e-6

    x_cur = init_x
    x_prev = 0

    while(abs(x_cur - x_prev) > eps):
        x_prev = x_cur
        steps+=1

        x_cur = x_prev - (x_prev*math.log(x_prev) - x_prev - K*math.log(10)) / (math.log(x_prev))

    return x_cur,  steps

def main():
    K = 1000000
    N, steps = Newtons_M(K)
    print(f"The target N is {round(N)} (achived for {steps} steps)")

if __name__ == '__main__':
    main()