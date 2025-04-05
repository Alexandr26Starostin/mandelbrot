# Модель Мандельброта
## О чём этот проект
Множество Мандельброта - множество точек на комплексной плоскости, которые после любого количества итераций остаётся внутри границы этого множества. Остальные точки уходят на бесконечность относительно начала координат.

Формулы для расчёта множества:

```
z[i+1] = z[i] * z[i] + z[0]
```

Или в координатах комплексной плоскости:

```
x[i+1] = x[i] * x[i] - y[i] * y[i] + x[0]

y[i+1] = 2 * x[i] *y[i] + y[0]
```

Причём заранее задать множество Мандельброта нельзя. Те каждую точку комплексной плоскости нужно просчитать в отдельности.

По этой причине расчёты выполняются долго. Цель данного проекта -  изучить особенности архитектуры x86, использовать XMM и YMM регистры для обработки нескольких точек плоскости за одну инструкцию процессора и использовать для этого Intrinsics func. В конце мы оценим улучшение производительности.

Для оценки работы будем использовать графическое окно, на котором будет нарисовано множество Мандельброта, и fps при его рисовании.

## Ключевые моменты в коде

Ключевыми являются функции расчёта точек множества Мандельброта.

Первоначальная функция расчёта:

```C
static sf::Color old_mandelbrot (sf::Vector2f coordinates_of_point)
{
    float x_0 = coordinates_of_point.x,
          y_0 = coordinates_of_point.y;

    float x_i_1 = 0, 
          y_i_1 = 0;

    float p_i_2 = 0;

    sf::Color color_of_pixel = BASE_COLOR_OF_PIXELS; 

    for (size_t iteration = 1; iteration <= MAX_NUMBER_OF_ITERATION_POINT; iteration++)
    {
        x_i_1 = coordinates_of_point.x;
        y_i_1 = coordinates_of_point.y;

        coordinates_of_point.x = deg_2_(x_i_1) - deg_2_(y_i_1) + x_0;   
        coordinates_of_point.y =  2 * x_i_1 * y_i_1 + y_0;

        p_i_2 = deg_2_(coordinates_of_point.x) + deg_2_(coordinates_of_point.y);

        if (p_i_2 >= MAX_DISTANCE_2)
        {
            color_of_pixel.r = (uint8_t) iteration * 50 % 256;
            color_of_pixel.b = (uint8_t) iteration * 20 % 256;
            return color_of_pixel;
        }
    }

    return sf::Color::Black; 
}
```

Функция расчёта с Intrinsics func:

```C
static __m256 mandelbrot (__m256 x_coordinates, __m256 y_coordinates)
{
    __m256 max_distances       = _mm256_set_ps (MAX_DISTANCE_2, MAX_DISTANCE_2, MAX_DISTANCE_2, MAX_DISTANCE_2, 
                                                MAX_DISTANCE_2, MAX_DISTANCE_2, MAX_DISTANCE_2, MAX_DISTANCE_2);

    __m256 results_of_cmp      = _mm256_set_ps (0, 0, 0, 0, 0, 0, 0, 0);
    __m256 count_of_iterations = _mm256_set_ps (0, 0, 0, 0, 0, 0, 0, 0);

    __m256 begin_x_coordinates = _mm256_set_ps (0, 0, 0, 0, 0, 0, 0, 0);

    __m256 x0_coordinates = x_coordinates;
    __m256 y0_coordinates = y_coordinates;

    __m256 distance_of_points = _mm256_set_ps (0, 0, 0, 0, 0, 0, 0, 0);

    __m256 mask_for_result = _mm256_set_ps (MASK_FOR_RESULT, MASK_FOR_RESULT, MASK_FOR_RESULT, MASK_FOR_RESULT, 
                                            MASK_FOR_RESULT, MASK_FOR_RESULT, MASK_FOR_RESULT, MASK_FOR_RESULT); 

    for (size_t iteration = 0; iteration < MAX_NUMBER_OF_ITERATION_POINT; iteration++)
    {
        begin_x_coordinates = x_coordinates;

        x_coordinates = deg_2_(x_coordinates) - deg_2_(y_coordinates) + x0_coordinates;
        y_coordinates = 2 * begin_x_coordinates * y_coordinates + y0_coordinates;

        distance_of_points = deg_2_(x_coordinates) + deg_2_(y_coordinates);

        results_of_cmp = _mm256_cmp_ps (distance_of_points, max_distances, _CMP_LT_OS);    //_CMP_LT_OS <--> <

        results_of_cmp = _mm256_and_ps (results_of_cmp, mask_for_result);

        count_of_iterations = _mm256_add_ps (count_of_iterations, results_of_cmp);

        if (cmp_mmx_and_null (results_of_cmp)) {break;}
    }

    return count_of_iterations; 
}
```

Первая функция по циклу считает по одной точке плоскости. Вторая функция, используя YMM-регистры, кладёт в одну переменную 8 точек и с помощью Intrinsics func обрабатывает сразу эти 8 точек. Это позволяет улучшить производительность программы. Мы кладём в данные регистры 8 значений типа *float* координат восьми точек.

## Информация о Intrinsics func

Intrinsics func используют XMM и YMM регистры и выполняют операции с ними. Для каждого процессора (архитектура x86) существуют свой набор таких функций. С полным набором функций можно ознакомиться [здесь](https://www.laruence.com/sse/#expand=4929,4932,4906,4929,4906,1012,3869,1012,291,291,282,300,1012,133,744).

Чтобы определить набор технологий и список Intrinsics func, которые вы можете использовать, нужно смотреть на ваш процессор. В нашем случае используется процессор [Intel N95](https://www.chaynikam.info/N95.html). Этот процессор поддерживает следующие технологии:
- MMX
- SSE
- SSE2
- SSE3
- SSE4.1
- SSE4.2
- AVX
- AVX2

Соответственно в нашем проекте используются функции из этих технологий.

Чтобы подключить весь список функций нужно подключить библиотеку:
```C
#include <immintrin.h>
```
и при компиляции с помощью g++ указать флаг компиляции:
```
-mavx2
``` 

## Графика

Для того чтобы нарисовать множество Мандельброта, мы используем графическую библиотеку [SFML](https://www.sfml-dev.org/documentation/3.0.0/) - 3.0.0.  
Имеется небольшой интерфейс для управления окном:
- <kbd>Esc</kbd> - закрыть окно
- <kbd>1</kbd> - увеличить картинку
- <kbd>2</kbd> - уменьшить картинку
- <kbd>&uarr;</kbd> - сдвинуть картинку вверх
- <kbd>&darr;</kbd> - сдвинуть картинку вниз
- <kbd>&larr;</kbd> - сдвинуть картинку влево
- <kbd>&rarr;</kbd> - сдвинуть картинку вправо

При запуске программы сначала открывается первая версия программы. После её закрытия через <kbd>Esc</kbd> появится вторая версия. Уже после её закрытия программа завершает работу.

Во время отрисовки окон в консоль постоянно выводится значение fps, вычисленное с помощью библиотеки:
```C
#include <time.h>
```

Точки на картинке имеют разные цвета. Если точка имеет чёрный цвет, то она принадлежит множеству Мандельброта. Остальные точки - не принадлежат ему. Мы будет считать, что точка принадлежит множеству Мандельброта, если спустя 256 итераций расстояние между ней и началом координат < 10. Для других точек мы может установить итерацию *iteration*, на которой она перестала удовлетворять нашему условию (расстояние < 10) и придать ей с помощью этой информации свой цвет:
```C
color_of_pixel.r = (uint8_t) iteration * 50 % 256;
color_of_pixel.g = 255;
color_of_pixel.b = (uint8_t) iteration * 20 % 256;
```

## Использование данной программы

Для использования данной программы необходимо:
1. Установить g++, SFML.
2. Скопировать себе ветку *master* нашего репозитория. 
3. В консоль ввести *make*
4. В консоль ввести *./mandelbrot* (название исходника можно поменять в *Makefile*)

Возможные коды ошибок и сообщений о них (в сообщениях об ошибках выходит информация о том, в каком месте кода произошла ошибка):
- 0 - нет ошибок.
- 1 - не может превратить текстуру с данными обо всех просчитанных пикселях в массив для изображения.
- 2 - не может корректно засечь время в тактах процессора.

## Оценка улучшения производительности программы

Оценим результат нашего улучшения производительности.

Во время измерений будем следить за тем, чтобы измерения происходили в одинаковых условиях (температура, наличии зарядки, положение ноутбука, неизменность самого устройства, отключение всех посторонних приложений). Дополнительно в настройках компьютера поставим на выполнение программы только одно конкретное ядро. Также во время измерения не будем изменять картинку, чтобы не менять список точек, которые программа будет вычислять и рисовать.

Возьмём 10 значений fps для каждой версии. Приборную погрешность измерения будем считать *= 0.003 1/с*. 

Таблица с результатами измерений fps:

| Номер измерения | Первая версия (обычная), 1/c | Вторая версия (оптимизированная), 1/c |
| --------------- | ---------------------------- | ------------------------------------- |
|       1         |         3.111                |        11.621                         |
|       2         |         3.290                |        13.053                         |
|       3         |         3.315                |        12.919                         |
|       4         |         3.170                |        14.045                         |
|       5         |         2.979                |        14.422                         |
|       6         |         3.324                |        13.877                         |
|       7         |         3.336                |        13.969                         |
|       8         |         2.888                |        14.304                         |
|       9         |         3.279                |        14.094                         |
|       10        |         2.959                |        13.845                         |

Тк значения находятся приблизительно в одном диапазоне значений, то для каждого из измерений мы можем посчитать среднее значений fps и найти среднюю погрешность отклонения. 

Среднее значение fps в  первой версии: (3.1651  +- 0.0009) 1/c.
Среднее значение fps во второй версии: (13.6149 +- 0.0009) 1/c.

Тогда производительность программы увеличилась в (4.301 +- 0.001) раз.

Получился неожиданный результат, тк ожидалось увеличение производительности приблизительно в 8 раз. Возможно, что какая-то часть кода работает не эффективно, что не позволяет осуществиться такому увеличению производительности программы. Тем не менее производительность выросла в 4.3 раза, что является положительным результатом нашей работы.

## Вывод

На данном проекте мы научились использовать Intrinsics func и с помощью них мы смогли улучшить производительность программы в 4.3 раза.

## Доработки

Список того, что нужно доработать в данном проекте:

- Улучшить качество картинки во второй версии программы (по неизвестной причине она ухудшилась).
- Добавить параметры командной строки для возможности отключения графики (это может быть полезно для проведения более точного измерения)
- Заменить измерение времени с помощью библиотеки *time.h* на функцию *rdtsc*, которая позволяет измерить время выполнения программы прямо машинных тактах (также для улучшения качества проведения измерения)