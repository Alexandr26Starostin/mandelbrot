#include <stdio.h>
#include <assert.h>
#include <SFML/Graphics.hpp>
//#include <SFML/Audio.hpp>

#include "const_in_mandelbrot.h"
#include "draw_mandelbrot.h"

errors_in_mandelbrot draw_mandelbrot ()
{
    errors_in_mandelbrot status = NOT_ERROR;

    uint x_size_window = 800;
    uint y_size_window = 600;

    sf::Color my_color {255, 255, 255};
    sf::Vector2u size_of_image = {x_size_window,     y_size_window};
    sf::Vector2u coordinates   = {x_size_window / 2, y_size_window / 2}; 

    // Create image - array of pixels
    sf::Image image (size_of_image, sf::Color::White);
  
    //---------------------------------------------------------------------------------------
    //draw some pixels
    uint x_save = coordinates.x;

    for (uint y = 0; y < 10; y++)
    {
        coordinates.x = x_save;

        for (uint x = 0; x < 10; x++)
        {
            coordinates.x += 1;
            image.setPixel(coordinates, sf::Color::Black);
        }

        coordinates.y += 1;
    }
    //---------------------------------------------------------------------------------------
    
    // Create a texture from the image
    sf::Texture texture;
    if (!texture.loadFromImage(image))
    {
        printf ("\n\nError in %s:%d\nCannot load texture from image", __FILE__, __LINE__);
        return CAN_NOT_LOAD_FROM_IMAGE;
    }
    
    // Create a sprite to display from texture
    sf::Sprite sprite(texture);

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
            }
        }

        window.clear(sf::Color::White);
        window.draw(sprite);
        window.display();
    }

	return status;
}
