import numpy as np
import matplotlib.pyplot as plt
import subprocess
from datetime import datetime

array_len = [
        32, 64, 128, 256, 512, 1024, 2048, 4096
    ]

def CleanFile(file_name: str):
    fd = open(file_name, "w")
    fd.write("")
    fd.close()

def ImportDataFileContent(file_name: str) -> list:
    res = []
    fd = open(file_name, "r")
    content = fd.read()
    # the last symbol is \n
    # so, the last split str will be ""
    for s in content.split("\n")[:-1]:
        res.append(float(s))
    fd.close()    
    return res

def Test(mode: str):
    date = datetime.strftime(datetime.now(), "%d.%m.%Y-%H.%M.%S")
    send_plot_file = r"./images/SendTime_" + mode + "_" + date + r"_.jpg" 
    resv_plot_file = r"./images/ResvTime_" + mode + "_" + date + r"_.jpg" 
    CleanFile("send.txt")
    CleanFile("resv.txt")

    for i in array_len:
        subprocess.run(["mpiexec", "-n", "2", "./BlockingSendResearch", str(i), str(mode)])

    send_time = ImportDataFileContent("send.txt")
    resv_time = ImportDataFileContent("resv.txt")

    fig1 = plt.figure()
    plt.title("Send time")
    plt.xlabel("Длина целочисленного массива")
    plt.ylabel("Время")
    plt.plot(np.array(array_len), np.array(send_time))
    plt.scatter(np.array(array_len), np.array(send_time))
    fig1.savefig(send_plot_file)

    fig2 = plt.figure()
    plt.title("Resv time")
    plt.xlabel("Длина целочисленного массива")
    plt.ylabel("Время")
    plt.plot(np.array(array_len), np.array(resv_time))
    plt.scatter(np.array(array_len), np.array(resv_time))
    fig2.savefig(resv_plot_file)

def main():
    Test("standart")
    Test("ready")
    Test("synch")
    # Test("buf")

if __name__ == '__main__':
    main()