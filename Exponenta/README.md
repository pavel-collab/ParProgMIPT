Установка пакета gmp с длинной арифметикой
```consol
sudo apt-get install libgmp3-dev
sudo apt-get install libmpc-dev 
```

Подключение пакета в качестве библиотеки
```C++
#include <gmpxx.h>
```

Дополнительные флаги компиляции
```consol
g++ main.cpp -lgmpxx -lgmp
```