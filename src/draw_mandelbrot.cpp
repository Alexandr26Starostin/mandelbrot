#include <stdio.h>
#include <assert.h>
#include <SFML/Graphics.hpp>
//#include <SFML/Audio.hpp>

#include "const_in_mandelbrot.h"
#include "draw_mandelbrot.h"

const float        BASE_SCALE                    = 1;
const float        D_SCALE                       = 0.5;
const uint         BASE_X_SIZE                   = 800;
const uint         BASE_Y_SIZE                   = 600;
const uint         HALF_OF_BASE_X_SIZE           = 400;
const uint         HALF_OF_BASE_Y_SIZE           = 300;
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

static sf::Color mandelbrot (sf::Vector2f coordinates_of_point);

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

//-----------------------------------------------------------------------------------

errors_in_mandelbrot draw_mandelbrot ()
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

        coordinates_of_point = BASE_COORDINATE_OF_POINT;

        x0 = min_x;
        y0 = max_y;

        for (uint y_pixel_index = 0; y_pixel_index < BASE_Y_SIZE; y_pixel_index++)
        {
            for (uint x_pixel_index = 0; x_pixel_index < BASE_X_SIZE; x_pixel_index++)
            {
                coordinates_of_point.x = x0;
                coordinates_of_point.y = y0;

                color_of_pixel = mandelbrot (coordinates_of_point);

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
    }

	return status;
}

static sf::Color mandelbrot (sf::Vector2f coordinates_of_point)
{
    float x_0 = coordinates_of_point.x,
          y_0 = coordinates_of_point.y;

    float x_i_1 = 0, 
          y_i_1 = 0;

    float p_i_2 = 0;

    sf::Color color_of_pixel = BASE_COLOR_OF_PIXELS;    // {0, 0, 0};   //sf::Color::Black

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