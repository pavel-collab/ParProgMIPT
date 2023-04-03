import numpy as np
import matplotlib.pyplot as plt

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
    res = ImportDataTimeFileContent("time.txt")
    print(res)
    print(np.mean(res))

if __name__ == '__main__':
    main()