import numpy as np
import matplotlib.pyplot as plt

def ImportDataFileContent(file_name: str) -> list:
    res = []
    fd = open(file_name, "r")
    content = fd.read()
    # the last symbol is \n
    # so, the last split str will be ""
    for s1 in content.split("\n")[:-1]:
        sub_res = []
        for s2 in s1.split(" ")[:-1]:
            sub_res.append(float(s2))
        res.append(sub_res)
    fd.close()    
    return res

t = np.linspace(0, 1, 100)
x = np.linspace(0, 1, 100)

u = np.full((100, 100), 0.0)
data = np.array(ImportDataFileContent("data.txt"))

plt.plot(x[:-1], data[0][:-1])
plt.plot(x, data[1])
plt.plot(x, data[2])
plt.plot(x, data[3])
plt.show()