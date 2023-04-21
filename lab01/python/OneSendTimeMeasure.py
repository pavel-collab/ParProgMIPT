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

def main():
    N_list = np.array([
        10000, 50000, 100000, 500000, 1000000,
        5000000, 10000000, 50000000, 100000000,
        500000000, 1000000000, 5000000000, 10000000000   
    ])

    for N in N_list:
        subprocess.run(["mpiexec", "-n", "2", "../prework", f'{N}'])
    times = ImportDataTimeFileContent("time.txt")

    date = datetime.strftime(datetime.now(), "%d.%m.%Y-%H.%M.%S")
    save_file_name = r"../images/" + "OneSendExec_" + date + r".jpg"

    fig = plt.figure()
    plt.scatter(np.array(N_list), np.array(times))
    plt.plot(np.array(N_list), np.array(times))
    plt.xlabel("Execs")
    plt.ylabel("time")
    plt.grid()
    # fig.savefig(save_file_name)

if __name__ == '__main__':
    main()