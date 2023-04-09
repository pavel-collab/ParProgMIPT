import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import subprocess
from datetime import datetime

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
    n_proc = 2
    points_list = [4, 10, 50, 100, 500, 1000, 2000]

    for points in points_list:
        subprocess.run(["../a.out", f'{points}', f'{points}'])
        validate = ImportDataFileContent("./validate.txt")
        fd = open("./validate.txt", "w")
        fd.write("")
        fd.close()

        subprocess.run(["mpiexec", "-n", f'{n_proc}', "../main", f'{points}', f'{points}'])
        data = ImportDataFileContent("./data.txt")
        fd = open("./data.txt", "w")
        fd.write("")
        fd.close()

        # размеры сетки
        K = np.shape(data)[1]
        M = np.shape(data)[0]
        t = np.linspace(0, 1, np.shape(data)[1])
        x = np.linspace(0, 1, np.shape(data)[0])

        x_grid, t_grid = np.meshgrid(x, t)

        date = datetime.strftime(datetime.now(), "%d.%m.%Y-%H.%M.%S")
        save_file_name = r"../images/validation/" + date + f'_{K*M}_' + r".jpg"

        fig = plt.figure()
        ax_3d = fig.add_subplot(projection="3d")
        ax_3d.plot_surface(x_grid, t_grid, data)
        ax_3d.plot_wireframe(x_grid, t_grid, data, linestyle='-', color='blue', label='Consistent')
        ax_3d.plot_wireframe(x_grid, t_grid, data, linestyle='--', color='red', label='Parallel')
        ax_3d.set_xlabel("x")
        ax_3d.set_ylabel("t")
        ax_3d.set_zlabel("u")
        ax_3d.legend()
        fig.savefig(save_file_name)

if __name__ == '__main__':
    main()