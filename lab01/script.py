import numpy as np
import matplotlib.pyplot as plt
import subprocess

def ImportDataTimeFileContent(file_name: str) -> list:
    res_list = []
    fd = open(file_name, "r")
    content = fd.read()
    # the last symbol is \n
    # so, the last split str will be ""
    for s in content.split(" ")[:-1]:
        res_list.append(float(s))
    fd.close()  
    res = np.array(res_list)
    return res

def main():
    n_proces = 4
    # points_list = [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000]
    points_list = [1000, 2000]

    consistent_prog_time = []
    parallel_prog_time = []

    # for points in points_list:
    #     for i in range(10):
    #         subprocess.run(["./a.out", f'{points}', f'{points}'])
    #         fd = open("data.txt", "w")
    #         fd.write("")
    #         fd.close()
    #     res = ImportDataTimeFileContent("time.txt")
    #     consistent_prog_time.append(np.mean(res))
    #     fd = open("time.txt", "w")
    #     fd.write("")
    #     fd.close()

    for points in points_list:
        for i in range(10):
            subprocess.run(["mpiexec", "-n", f'{n_proces}', "./main", f'{points}', f'{points}'])
            fd = open("data.txt", "w")
            fd.write("")
            fd.close()
        res = ImportDataTimeFileContent("time.txt")
        parallel_prog_time.append(np.mean(res))
        fd = open("time.txt", "w")
        fd.write("")
        fd.close()

    # fig = plt.figure()
    # plt.scatter(np.array(points_list), np.array(consistent_prog_time))
    # plt.plot(np.array(points_list), np.array(consistent_prog_time), label='Consistent')
    # plt.scatter(np.array(points_list), np.array(parallel_prog_time))
    # plt.plot(np.array(points_list), np.array(parallel_prog_time), label='Consistent')
    # plt.xlabel("points amount")
    # plt.ylabel("time")
    # plt.show()
    # fig.savefig('./images/time.jpg')

if __name__ == '__main__':
    main()