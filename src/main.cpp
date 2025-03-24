#include <stdio.h>

#include "const_in_mandelbrod.h"
#include "draw_mandelbrod.h"

int main ()
{
	printf ("Hello!\nI am model of Mandelbrod\n");

	errors_in_mandelbrod status =  draw_mandelbrod ();
	
	return status;
}