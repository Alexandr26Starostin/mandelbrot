#include <stdio.h>
#include <assert.h>
#include <SFML/Graphics.hpp>
#include <immintrin.h>
#include <time.h>

#include "const_in_mandelbrot.h"
#include "draw_mandelbrot.h"

const float        BASE_SCALE                    = 1;
const float        D_SCALE                       = 0.5;
const uint         BASE_X_SIZE                   = 800;
const uint         BASE_Y_SIZE                   = 600;
const uint         HALF_OF_BASE_X_SIZE           = BASE_X_SIZE / 2;
const uint         HALF_OF_BASE_Y_SIZE           = BASE_Y_SIZE / 2;
const sf::Color    BASE_COLOR_OF_PIXELS          = {100, 255, 100};
const float        PHYSICAL_X_SIZE               = 3;
const float        PHYSICAL_Y_SIZE               = 2;
const float        BASE_X_MIN                    = -2;
const float        BASE_Y_MAX                    = 1;
const float        BASE_SHIFTING_ON_HORIZONTAL   = 0.25;
const float        BASE_SHIFTING_ON_VERTICAL     = 0.25;
const sf::Vector2f BASE_COORDINATE_OF_POINT      = {0, 0};
const float        MAX_DISTANCE_2                = 100;
const size_t       MAX_NUMBER_OF_ITERATION_POINT = 256;
const size_t       LEN_POINTS_IN_MMX             = 8;   
const size_t       MASK_FOR_CMP                  = 1;       
const float        E_LOCALITY                    = (float) 0.0001;
const float        MASK_FOR_RESULT               = (float) 0x00000001;

enum count_of_vector_t
{
    ONE_VECTOR  = 1,
    TWO_VECTORS = 2
};

enum mode_of_working_t
{
	NOT_MODE         = 0,
	PRINT_FAST       = 1,
	PRINT_WITH_PAUSE = 2
};

struct list_arg_for_print_vector_t
{
    void*             x_coordinates;
    void*             y_coordinates;
    count_of_vector_t count_of_vectors;  
    size_t            count_of_elements;
    char              type; 
    mode_of_working_t mode;
};

static __m256    mandelbrot       (__m256 x_coordinates, __m256 y_coordinates);
static sf::Color old_mandelbrot   (sf::Vector2f coordinates_of_point);
static bool      cmp_mmx_and_null (__m256 vector);

errors_in_mandelbrot print_list_of_points (list_arg_for_print_vector_t list_arg);

#define deg_2_(value) value * value 

#define do_zoom_(operation)                                         \
    x_center = min_x + dx * HALF_OF_BASE_X_SIZE;                    \
    y_center = max_y - dy * HALF_OF_BASE_Y_SIZE;                    \
                                                                    \
    scale =  scale operation D_SCALE;                               \
                                                                    \
    dx = PHYSICAL_X_SIZE / ((float) BASE_X_SIZE * scale);           \
    dy = PHYSICAL_Y_SIZE / ((float) BASE_Y_SIZE * scale);           \
                                                                    \
    min_x = x_center - HALF_OF_BASE_X_SIZE * dx;                    \
    max_y = y_center + HALF_OF_BASE_Y_SIZE * dy;                    \
                                                                    \
    shifting_on_horizontal = BASE_SHIFTING_ON_HORIZONTAL / scale;   \
    shifting_on_vertical   = BASE_SHIFTING_ON_VERTICAL   / scale;   

#define print_type_of_vector_(type_of_vector, str_for_arg)                              \
    type_of_vector* list_of_x = (type_of_vector*) list_arg.x_coordinates;   			\
    type_of_vector* list_of_y = (type_of_vector*) list_arg.y_coordinates;   			\
                                                                                		\
    for (size_t index = 0; index < list_arg.count_of_elements; index++)      			\
    {                                                                       			\
        printf (str_for_arg, *(list_of_x));                                    			\
        list_of_x++;                                                            		\
    }                                                                           		\
                                                                                		\
    if (list_arg.count_of_vectors == TWO_VECTORS)                               		\
    {                                                                           		\
        printf ("\n");                                                          		\
                                                                                		\
        for (size_t index = 0; index < list_arg.count_of_elements; index++)     		\
        {                                                                       		\
            printf (str_for_arg, *(list_of_y));                                         \
            list_of_y++;                                                                \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    printf ("\n------------------------------------------------------------\n\n");

//-----------------------------------------------------------------------------------

errors_in_mandelbrot draw_mandelbrot ()
{
    errors_in_mandelbrot status = NOT_ERROR;

    sf::Color color_of_pixel = BASE_COLOR_OF_PIXELS;

    // Create image - array of pixels
    sf::Vector2u size_of_image = {BASE_X_SIZE, BASE_Y_SIZE};
    sf::Image image (size_of_image, color_of_pixel);

    float scale = BASE_SCALE;
    
    float dx = PHYSICAL_X_SIZE / ((float) BASE_X_SIZE),
          dy = PHYSICAL_Y_SIZE / ((float) BASE_Y_SIZE);

    float min_x = BASE_X_MIN,
          max_y = BASE_Y_MAX;

    float x0 = min_x, 
          y0 = max_y; 

    float shifting_on_horizontal = BASE_SHIFTING_ON_HORIZONTAL,
          shifting_on_vertical   = BASE_SHIFTING_ON_VERTICAL;

    float x_center = 0, 
          y_center = 0;

    __m256 x_coordinates       = _mm256_set_ps (0, 0, 0, 0, 0, 0, 0, 0);
    __m256 y_coordinates       = _mm256_set_ps (0, 0, 0, 0, 0, 0, 0, 0);
    __m256 count_of_iterations = _mm256_set_ps (0, 0, 0, 0, 0, 0, 0, 0);

    sf::Texture texture;

	clock_t time1 = 0;
	clock_t time2 = 0;

    //---------------------------------------------------------------------------------------

	sf::RenderWindow window(sf::VideoMode({BASE_X_SIZE, BASE_Y_SIZE}), "model_of_mandelbrot");

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num1)
                    {do_zoom_(+)}
                    
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num2)
                {
                    if (scale > D_SCALE) {do_zoom_(-)}
                }

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Up)
                    max_y += shifting_on_vertical;
                
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Down)
                    max_y -= shifting_on_vertical;

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Left)
                    min_x -= shifting_on_horizontal;

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Right)
                    min_x += shifting_on_horizontal;       
            }
        }

        //----------------------------------------------------------------------------------------
        //draw model_of_mandelbrot

		time1 = clock ();
		if (time1 == -1)
		{
			printf ("\n\nError in %s:%d\nCannot do clock();", __FILE__, __LINE__);
			return ERROR_IN_CHECK_TIME;
		}

        x0 = min_x;
        y0 = max_y;

        for (uint y_pixel_index = 0; y_pixel_index < BASE_Y_SIZE; y_pixel_index++)
        {
            for (uint x_pixel_index = 0; x_pixel_index < BASE_X_SIZE; x_pixel_index += LEN_POINTS_IN_MMX)
            {
                y_coordinates = _mm256_set_ps (y0,        y0,        y0,        y0,        
                                               y0,        y0,        y0,        y0);
                x_coordinates = _mm256_set_ps (x0,        x0 + dx,   x0 + 2*dx, x0 + 3*dx, 
                                               x0 + 4*dx, x0 + 5*dx, x0 + 6*dx, x0 + 7*dx);

                count_of_iterations = mandelbrot (x_coordinates, y_coordinates);

                float* iteration_for_point = (float*) &count_of_iterations;

                for (size_t index_of_point = 0; index_of_point < LEN_POINTS_IN_MMX; index_of_point++)
                {
                    int iteration = (int) *(iteration_for_point + index_of_point);

                    if (iteration == MAX_NUMBER_OF_ITERATION_POINT)
                    {
                        image.setPixel({x_pixel_index + (int) index_of_point, y_pixel_index}, sf::Color::Black);
                    }

                    else
                    {
                        color_of_pixel.r = (uint8_t) iteration * 50 % 256;
                        color_of_pixel.b = (uint8_t) iteration * 20 % 256;

                        image.setPixel({x_pixel_index + (int) index_of_point, y_pixel_index}, color_of_pixel);
                    }
                }

                x0 += dx * LEN_POINTS_IN_MMX;      //x0 += dx * 8
            }

            x0  = min_x;
            y0 -= dy;
        }

        // Create a texture from the image
        if (!texture.loadFromImage(image))
        {
            printf ("\n\nError in %s:%d\nCannot load texture from image", __FILE__, __LINE__);
            return CAN_NOT_LOAD_FROM_IMAGE;
        }

        // Create a sprite to display from texture
        sf::Sprite sprite(texture);

        //----------------------------------------------------------------------------------------

        window.clear(BASE_COLOR_OF_PIXELS);
        window.draw(sprite);
        window.display();

		time2 = clock ();
		if (time2 == -1)
		{
			printf ("\n\nError in %s:%d\nCannot do clock();", __FILE__, __LINE__);
			return ERROR_IN_CHECK_TIME;
		}

		printf("fps == %f\n", CLOCKS_PER_SEC / ((double) (time2 - time1)));
    }

	return status;
}

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

static bool cmp_mmx_and_null (__m256 vector)
{
    float* ptr_on_vector = (float*) &vector;

    for (size_t index = 0; index < LEN_POINTS_IN_MMX; index++)
    {
        if (*(ptr_on_vector + index) >= E_LOCALITY)
        {
            return false;
        }
    }
    return true;
}

errors_in_mandelbrot print_list_of_points (list_arg_for_print_vector_t list_arg)
{
    assert (list_arg.x_coordinates);
    assert (list_arg.y_coordinates);

    if (list_arg.mode == NOT_MODE) {return NOT_ERROR;}

	switch (list_arg.type)
	{
		case 'f': {print_type_of_vector_(float, "%6f | ")  break;}
		case 'd': {print_type_of_vector_(int,   "%6d | ")  break;}

		default: return NOT_ERROR;
	}
    

    if (list_arg.mode == PRINT_WITH_PAUSE) {getchar ();}

    return NOT_ERROR;
}

//----------------------------------------------------------------------------------------------------------------------------

errors_in_mandelbrot old_draw_mandelbrot ()
{
    errors_in_mandelbrot status = NOT_ERROR;

    sf::Color color_of_pixel = BASE_COLOR_OF_PIXELS;

    // Create image - array of pixels
    sf::Vector2u size_of_image = {BASE_X_SIZE, BASE_Y_SIZE};
    sf::Image image (size_of_image, sf::Color::Black);

    float scale = BASE_SCALE;
    
    float dx = PHYSICAL_X_SIZE / ((float) BASE_X_SIZE * scale),
          dy = PHYSICAL_Y_SIZE / ((float) BASE_Y_SIZE * scale);

    float min_x = BASE_X_MIN,
          max_y = BASE_Y_MAX;

    float x0 = min_x, 
          y0 = max_y; 

    float shifting_on_horizontal = BASE_SHIFTING_ON_HORIZONTAL,
          shifting_on_vertical   = BASE_SHIFTING_ON_VERTICAL;

    float x_center = 0, 
          y_center = 0;

    sf::Vector2f coordinates_of_point = BASE_COORDINATE_OF_POINT;

    sf::Texture texture;

	clock_t time1 = 0;
	clock_t time2 = 0;

    //---------------------------------------------------------------------------------------

	sf::RenderWindow window(sf::VideoMode({BASE_X_SIZE, BASE_Y_SIZE}), "model_of_mandelbrot");

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num1)
                    {do_zoom_(+)}
                    
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num2)
                {
                    if (scale > D_SCALE) {do_zoom_(-)}
                }

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Up)
                    max_y += shifting_on_vertical;
                
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Down)
                    max_y -= shifting_on_vertical;

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Left)
                    min_x -= shifting_on_horizontal;

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Right)
                    min_x += shifting_on_horizontal;       
            }
        }

        //----------------------------------------------------------------------------------------
        //draw model_of_mandelbrot

		time1 = clock ();
		if (time1 == -1)
		{
			printf ("\n\nError in %s:%d\nCannot do clock();", __FILE__, __LINE__);
			return ERROR_IN_CHECK_TIME;
		}

        coordinates_of_point = BASE_COORDINATE_OF_POINT;

        x0 = min_x;
        y0 = max_y;

        for (uint y_pixel_index = 0; y_pixel_index < BASE_Y_SIZE; y_pixel_index++)
        {
            for (uint x_pixel_index = 0; x_pixel_index < BASE_X_SIZE; x_pixel_index++)
            {
                coordinates_of_point.x = x0;
                coordinates_of_point.y = y0;

                color_of_pixel = old_mandelbrot (coordinates_of_point);

                image.setPixel({x_pixel_index, y_pixel_index}, color_of_pixel);

                x0 += dx;
            }

            x0  = min_x;
            y0 -= dy;
        }

        // Create a texture from the image
        if (!texture.loadFromImage(image))
        {
            printf ("\n\nError in %s:%d\nCannot load texture from image", __FILE__, __LINE__);
            return CAN_NOT_LOAD_FROM_IMAGE;
        }

        // Create a sprite to display from texture
        sf::Sprite sprite(texture);

        //----------------------------------------------------------------------------------------

        window.clear(BASE_COLOR_OF_PIXELS);
        window.draw(sprite);
        window.display();

		time2 = clock ();
		if (time2 == -1)
		{
			printf ("\n\nError in %s:%d\nCannot do clock();", __FILE__, __LINE__);
			return ERROR_IN_CHECK_TIME;
		}

		printf("fps == %f\n", CLOCKS_PER_SEC / ((double) (time2 - time1)));
    }

	return status;
}

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

//rdtsc - время в машинных тактах