import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def ImportDataFileContent(file_name: str) -> list:
    res_list = []
    fd = open(file_name, "r")
    content = fd.read()
    # the last symbol is \n
    # so, the last split str will be ""
    for s1 in content.split("\n")[:-1]:
        sub_res = []
        for s2 in s1.split(" ")[:-1]:
            sub_res.append(float(s2))
        res_list.append(sub_res)
    fd.close()  
    res = np.array(res_list)
    return res.T

def main():
    data = ImportDataFileContent("../data.txt")

    # размеры сетки
    K = np.shape(data)[1]
    M = np.shape(data)[0]
    t = np.linspace(0, 1, K)
    x = np.linspace(0, 1, M)

    x_grid, t_grid = np.meshgrid(x, t)

    fig = plt.figure()
    ax_3d = fig.add_subplot(projection="3d")
    ax_3d.plot_surface(x_grid, t_grid, data)
    ax_3d.plot_wireframe(x_grid, t_grid, data)
    ax_3d.set_xlabel("x")
    ax_3d.set_ylabel("t")
    ax_3d.set_zlabel("u")
    plt.show()
    fig.savefig("../images/solution.jpg")

if __name__ == '__main__':
    main()