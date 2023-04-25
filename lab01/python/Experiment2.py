import numpy as np
import matplotlib.pyplot as plt
import subprocess
from datetime import datetime
# import argparse

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

def CleanFile(file_path):
    fd = open(file_path, "w")
    fd.write("")
    fd.close()

def main():
    n_proc_list = [2, 4, 5, 8]
    points_list = [8000, 10000, 12000]
    # points_list = [16000, 18000, 20000]

    fig = plt.figure()

    for points in points_list:
        speedup_list = []

        for n_proc in n_proc_list:
            for i in range(5):
                subprocess.run(["../a.out", f'{points}', f'{points}'])
                CleanFile("./validate.txt")

            res = ImportDataTimeFileContent("time.txt")
            consist_time = np.mean(res)
            CleanFile("./time.txt")

            for i in range(5):
                subprocess.run(["mpiexec", "-n", f'{n_proc}', "../main", f'{points}', f'{points}'])
                CleanFile("./data.txt")
                
            res = ImportDataTimeFileContent("time.txt")
            parallel_time = np.mean(res)
            CleanFile("./time.txt")

            speedup_list.append(consist_time/parallel_time)

        plt.scatter(np.array(n_proc_list), np.array(speedup_list))
        plt.plot(np.array(n_proc_list), np.array(speedup_list), label=f'K = M = {points}')

    date = datetime.strftime(datetime.now(), "%d.%m.%Y-%H.%M.%S")
    save_file_name = r"../images/acceleration/" + 'acceleration' + date + r".jpg"

    plt.xlabel("proces amount")
    plt.ylabel("speedup")
    plt.legend()
    plt.grid()
    fig.savefig(save_file_name)

if __name__ == '__main__':
    main()