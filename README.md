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

