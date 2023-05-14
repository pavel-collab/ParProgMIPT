import numpy as np
import matplotlib.pyplot as plt
import subprocess
from datetime import datetime

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
    accuracy = 0.00001

    # threads = [
    #     1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
    # ]
    threads = [
        1, 2, 3, 4, 5, 6, 7, 8
    ]

    exec_time = []

    for n in threads:
        print("-"*50)
        print(f"{n} threads")
        for i in range(10):
            subprocess.run(["../a.out", f'{n}', f'{accuracy}'])
            
        res = ImportDataTimeFileContent("time.txt")
        exec_time.append(np.mean(res))
        CleanFile("./time.txt")
        print("-"*50)


    date = datetime.strftime(datetime.now(), "%d.%m.%Y-%H.%M.%S")
    save_file_name = r"../images/" + date + r".jpg"

    fig = plt.figure()
    plt.scatter(np.array(threads), np.array(exec_time))
    plt.plot(np.array(threads), np.array(exec_time))
    plt.xlabel("threads")
    plt.ylabel("time (ms)")
    plt.grid()
    fig.savefig(save_file_name)

if __name__ == '__main__':
    main()