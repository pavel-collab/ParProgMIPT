import numpy as np
import matplotlib.pyplot as plt
import subprocess
from datetime import datetime
import argparse

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
    parser = argparse.ArgumentParser()
    parser.add_argument("-n", "--number_of_proc", help="*description*")
    args = parser.parse_args()

    if args.number_of_proc != None:
        n_proces = args.number_of_proc
    else:
        n_proces = 2

    points_list = [
        100,   500,   1000,  2000,  3000,  4000, 5000, 6000, 7000, 8000,
        10000, 12000, 14000, 16000, 18000, 20000
    ]

    consistent_prog_time = []
    parallel_prog_time = []

    for points in points_list:
        for i in range(10):
            subprocess.run(["../a.out", f'{points}', f'{points}'])
            CleanFile("./data.txt")
            
        res = ImportDataTimeFileContent("time.txt")
        consistent_prog_time.append(np.mean(res))
        CleanFile("./time.txt")

    for points in points_list:
        for i in range(10):
            subprocess.run(["mpiexec", "-n", f'{n_proces}', "../main", f'{points}', f'{points}'])
            CleanFile("./data.txt")

        res = ImportDataTimeFileContent("time.txt")
        parallel_prog_time.append(np.mean(res))
        CleanFile("./time.txt")

    date = datetime.strftime(datetime.now(), "%d.%m.%Y-%H.%M.%S")
    save_file_name = r"../images/acceleration/" + f'{n_proces}_proc_' + date + r".jpg"

    fig = plt.figure()
    plt.scatter(np.array(points_list), np.array(consistent_prog_time))
    plt.plot(np.array(points_list), np.array(consistent_prog_time), label='Consistent')
    plt.scatter(np.array(points_list), np.array(parallel_prog_time))
    plt.plot(np.array(points_list), np.array(parallel_prog_time), label='Parallel')
    plt.xlabel("points amount")
    plt.ylabel("time")
    plt.legend()
    plt.grid()
    fig.savefig(save_file_name)

if __name__ == '__main__':
    main()