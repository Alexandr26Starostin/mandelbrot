#include <stdio.h>

#include "const_in_mandelbrot.h"
#include "draw_mandelbrot.h"

int main ()
{
	printf ("Hello!\nI am model of Mandelbrot\n");

	errors_in_mandelbrot status =  draw_mandelbrot ();
	
	return status;
}