import numpy as np
import matplotlib.pyplot as plt
import subprocess
from datetime import datetime
import argparse
from time import sleep

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

def GenArray(low, high, size, file_path):
    CleanFile(file_path)

    gen_array = np.random.randint(low, high, size)
    gen_array_size = np.size(gen_array)
    
    fd = open(file_path, "w")
    fd.write(str(gen_array_size) + "\n")
    for item in gen_array:
        fd.write(str(item) + " ")
    fd.close()

def GenBadCase(low, high, size, file_path):
    CleanFile(file_path)

    gen_array = np.random.randint(low, high, size)
    gen_array_size = np.size(gen_array)

    gen_array = list(gen_array)
    gen_array.sort(reverse=True) 
    
    fd = open(file_path, "w")
    fd.write(str(gen_array_size) + "\n")
    for item in gen_array:
        fd.write(str(item) + " ")
    fd.close()

def main():
    low = -2048
    high = 2048
    file_name = "../tests/test.dat"
    bad_case_file_name = "../tests/bad_case.dat"
    size_list = [50, 100, 200, 500, 1000, 2000, 3000, 5000, 8000, 10000, 12000, 14000, 16000, 18000, 20000]

    parser = argparse.ArgumentParser()
    parser.add_argument("-n", "--number_of_proc", help="*description*")
    args = parser.parse_args()

    if args.number_of_proc != None:
        n_proc = args.number_of_proc
    else:
        n_proc = 2

    consistent_prog_time = []
    parallel_prog_time = []
    for size in size_list:
        CleanFile(file_name)
        CleanFile(bad_case_file_name)
        GenArray(low, high, size, file_name)
        GenBadCase(low, high, size, bad_case_file_name)

        for i in range(10):
            subprocess.run(["../a.out", f'{file_name}'])
            CleanFile("./data.txt")
            sleep(0.5)
        res = ImportDataTimeFileContent("time.txt")
        consistent_prog_time.append(np.mean(res))
        CleanFile("./time.txt")

    date = datetime.strftime(datetime.now(), "%d.%m.%Y-%H.%M.%S")
    save_file_name = r"../images/" + f'{n_proc}_proc_' + date + r".jpg"

    fig = plt.figure()
    plt.scatter(np.array(size_list), np.array(consistent_prog_time))
    plt.plot(np.array(size_list), np.array(consistent_prog_time), label='Consistent')
    # plt.scatter(np.array(size_list), np.array(parallel_prog_time))
    # plt.plot(np.array(size_list), np.array(parallel_prog_time), label='Parallel')
    plt.xlabel("points amount")
    plt.ylabel("time (ms)")
    plt.legend()
    plt.grid()
    fig.savefig(save_file_name)



if __name__ == '__main__':
    main()