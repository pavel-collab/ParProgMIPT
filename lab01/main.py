import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

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

x_grid, t_grid = np.meshgrid(x, t)

data = np.array(ImportDataFileContent("data.txt"))

fig = plt.figure()
ax_3d = fig.add_subplot(projection="3d")
# ax_3d.plot_surface(x_grid, t_grid, data)
ax_3d.plot_wireframe(x_grid, t_grid, data)
ax_3d.set_xlabel("x")
ax_3d.set_ylabel("t")
ax_3d.set_zlabel("u")
plt.show()
# fig.savefig("./test.jpg")