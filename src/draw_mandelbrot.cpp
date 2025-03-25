#include <stdio.h>
#include <assert.h>
#include <SFML/Graphics.hpp>
//#include <SFML/Audio.hpp>

#include "const_in_mandelbrot.h"
#include "draw_mandelbrot.h"

static sf::Color mandelbrot (sf::Vector2f coordinates_of_point);

errors_in_mandelbrot draw_mandelbrot ()
{
    errors_in_mandelbrot status = NOT_ERROR;

    uint x_size_window = 800;
    uint y_size_window = 600;

    sf::Color color_of_pixel {0, 0, 0};

    // Create image - array of pixels
    sf::Vector2u size_of_image = {x_size_window, y_size_window};
    sf::Image image (size_of_image, sf::Color::Black);

    float scale   = 1,
          d_scale = 2;
    
    float dx = 3 / ((float) x_size_window * scale),
          dy = 2 / ((float) y_size_window * scale);

    float min_x = -2 / scale,
          max_y = 1  / scale;
        //min_x = -2 / scale, 
        //min_y = -1 / scale;

    float shifting_on_horizontal = 0.25,
          shifting_on_vertical   = 0.25;


    float x0 = min_x, 
          y0 = max_y; 

    float x_center = 0,
          y_center = 0;

    sf::Vector2f coordinates_of_point = {0, 0};

    sf::Texture texture;

    //---------------------------------------------------------------------------------------

	sf::RenderWindow window(sf::VideoMode({x_size_window, y_size_window}), "model_of_mandelbrot");

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
                {
                    dx *= scale;
                    dy *= scale;

                    min_x *= scale;
                    max_y *= scale;

                    scale += d_scale;

                    dx = 3 / ((float) x_size_window * scale);
                    dy = 2 / ((float) y_size_window * scale);

                    min_x /= scale;
                    max_y /= scale;
                }

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num2)
                {
                    dx *= scale;
                    dy *= scale;

                    min_x *= scale;
                    max_y *= scale;

                    scale -= d_scale;

                    dx = 3 / ((float) x_size_window * scale);
                    dy = 2 / ((float) y_size_window * scale);

                    min_x /= scale;
                    max_y /= scale;
                }

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Up)
                    max_y += shifting_on_vertical / scale;
                
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Down)
                    max_y -= shifting_on_vertical / scale;

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Left)
                    min_x -= shifting_on_horizontal / scale;

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Right)
                    min_x += shifting_on_horizontal / scale;
                
            }
        }

        //----------------------------------------------------------------------------------------
        //draw model_of_mandelbrot

        coordinates_of_point = {0, 0};

        x0 = min_x;
        y0 = max_y;

        for (uint y_index = 0; y_index < y_size_window; y_index++)
        {
            for (uint x_index = 0; x_index < x_size_window; x_index++)
            {
                coordinates_of_point.x = x0;
                coordinates_of_point.y = y0;

                color_of_pixel = mandelbrot (coordinates_of_point);

                image.setPixel({x_index, y_index}, color_of_pixel);

                x0 += dx;
            }

            x0 = min_x;

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

        window.clear(sf::Color::Black);
        window.draw(sprite);
        window.display();
    }

	return status;
}

static sf::Color mandelbrot (sf::Vector2f coordinates_of_point)
{
    float x_0 = coordinates_of_point.x,
          y_0 = coordinates_of_point.y;

    float p_max_2 = 100;    //p_max^2
 
    size_t max_number_of_iteration_point = 256; 

    float x_i_1 = 0, 
          y_i_1 = 0;

    float p_i_2 = 0;

    for (size_t iteration = 1; iteration <= max_number_of_iteration_point; iteration++)
    {
        x_i_1 = coordinates_of_point.x;
        y_i_1 = coordinates_of_point.y;

        coordinates_of_point.x = x_i_1 * x_i_1 - y_i_1 * y_i_1 + x_0;
        coordinates_of_point.y =  2 * x_i_1 * y_i_1 + y_0;

        p_i_2 = coordinates_of_point.x * coordinates_of_point.x + coordinates_of_point.y * coordinates_of_point.y;

        if (p_i_2 >= p_max_2)
        {
            return sf::Color::Black;
        }
    }

    return sf::Color::White;
}
