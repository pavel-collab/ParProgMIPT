### Способы определить количество ядер (на Linux).

```
cat /proc/cpuinfo|grep processor
lscpu
nproc
```

### Формат запуска программы

```
mpiexec -n 3 ./HelloWorld
```