#include <stdio.h>
#include <assert.h>
#include <SFML/Graphics.hpp>
//#include <SFML/Audio.hpp>

#include "const_in_mandelbrod.h"
#include "draw_mandelbrod.h"

errors_in_mandelbrod draw_mandelbrod ()
{
    errors_in_mandelbrod status = NOT_ERROR;

    uint x_size_window = 800;
    uint y_size_window = 600; 

	sf::RenderWindow window(sf::VideoMode({x_size_window, y_size_window}), "model_of_mandekbrod");

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

        window.clear();
        window.display();
    }

	return status;
}