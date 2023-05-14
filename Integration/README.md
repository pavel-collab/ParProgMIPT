# README

Программа выполняет численное интегрирование функции
$$
\cos \frac{1}{x}
$$
на отрезке $[0.005, 5]$. Валидацию производили с использованием надежного источника WolframAlpha.
~~Потому что, если он ошибется, то вообще никому в мире нельзя будет верить~~

<p align="center">
<img src="https://github.com/pavel-collab/ParProgMIPT/blob/main/Integration/images/validation.jpg" alt="caption" width="300"/>
</p>

#### Компиляция и запуск

```
make
./a.out 3 0.00001
```

Так же есть возможность включить debug mode. В этом случае программа будет выводить в консоль
сообщения о промежуточной работе, что позволит более детально отследить работу программы.
Для этого откройте файл _lib.cpp_ и раскоментируйте строчку
```CPP
#define DEBUG
```
После чего пересоберите программу. Читать отладочные записи с консоли может быть неудобно, 
поэтому при использованиии данной опции предлагается перенаправлять вывод программы в отладочный 
файл:
```
make
./a.out 3 0.000001 > debug.txt
```

Первым параметром указываем количество потоков, вторым параметром указываем точность вычислений.

#### Python

Скрипт запускает эксперемент. Сравнивается время работы при фиксированной точности на 
разном числе потоков.

```
python3 main.py
```

__ATTENTION:__ после того, как скрипт отработает, не забудьте удалить из дериктории _python_ все
.txt файлы.

#### Результаты работы программы

Программа выдает корректный результат с заданной точностью. Многопоточная реализация выдает
вполне ожидаемое ускорение на небольшом количестве потоков.

<p align="center">
<img src="https://github.com/pavel-collab/ParProgMIPT/blob/main/Integration/images/14.05.2023-11.06.31.jpg" alt="caption" width="300"/>
</p>

На большом числе потоков программа начинает работать хуже. Это может быть связано, как с 
характеристиками компьютера (количество ядер CPU), так и с ростом накладных расходов на создание
и работу потоков.

<p align="center">
<img src="https://github.com/pavel-collab/ParProgMIPT/blob/main/Integration/images/14.05.2023-11.06.03.jpg" alt="caption" width="300"/>
</p>

Немаловажную роль в ускорении играют параметры __STACK_LIMIT__ и __TRANSMIT_SIZE__ (см исходник
_lib.hpp_). Данные параметры отвечают соответственно за максимальный размер локального стека
и за количество записей, переносимых из одного стека в другой за один раз. В ходе эксперементов
выяснилось, что оптимальные значения данных параметров примерно 4 и 3 соответственно. 
При больших значениях программа начинает давать худшие результаты при многопоточном исполнении.

###### Результаты работы программы при STACK_LIMIT = 10 TRANSMIT_SIZE = 8

<p align="center">
<img src="https://github.com/pavel-collab/ParProgMIPT/blob/main/Integration/images/14.05.2023-11.11.44.jpg" alt="caption" width="300"/>
</p>

###### Результаты работы программы при STACK_LIMIT = 32 TRANSMIT_SIZE = 20

<p align="center">
<img src="https://github.com/pavel-collab/ParProgMIPT/blob/main/Integration/images/14.05.2023-11.13.48.jpg" alt="caption" width="300"/>
</p>

Данный феномен легко объяснить, если посмотреть вывод отладочной информации программы (приводим
небольшой фрагмент из отладочного файла):
```
Thread [0] (accept range [0.112314, 0.115566]) Sac = -0.001353, Scb = -0.001231, Sab = -0.002575
thread [1] curent sem value is 1
Thread [0]. Local stack access
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [3] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [3] curent sem value is 1
thread [3] curent sem value is 1
thread [1] curent sem value is 1
thread [4] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [1] curent sem value is 1
thread [3] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [5] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [1] curent sem value is 1
thread [4] curent sem value is 1
thread [2] curent sem value is 1
thread [5] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [4] curent sem value is 1
thread [3] curent sem value is 1
```

Как видно, чем больше верхняя граница локального стека, тем реже потоки выгружают записи
из локального стека в глобальный. Это приводит к тому, что большая часть потоков простаивает без
работы, в то время как только нулевой поток занят полезной нагрузкой. Таким образом, мы не только
фактически подменяем многопоточную работу однопоточной, но и тратим время и ресурсы на создание
почти бесполезных потоков, которые большую часть времени простаивают без дела.