#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include <string>
#include <iostream>
#include "GameOfLifeLib.h"


static void setBuffer(unsigned char*& arr) {
	unsigned char* image = new unsigned char[100];
	std::fill(image, image + 100, 255);
	image[3 * 10 + 3] = 0;
	image[3 * 10 + 7] = 0;
	image[4 * 10 + 5] = 0;
	image[5 * 10 + 4] = 0;
	image[5 * 10 + 6] = 0;
	image[6 * 10 + 5] = 0;
	image[7 * 10 + 3] = 0;
	image[7 * 10 + 7] = 0;
	arr = image;
}

int main()
{
	int selectedOption, numberOfIterations, saveOnIteration;

	std::cout << "-------------------" << std::endl << "Conway's Game of Life - OpenCL" << std::endl << "-------------------" << std::endl << std::endl;
	std::cout << "** Please select one of the options? **" << std::endl << std::endl;
	std::cout << "How do you want to load the matrix?" << std::endl;
	std::cout << "-- 1. Run the custom matrix" << std::endl;
	std::cout << "-- 2. Run the game from the image matrix" << std::endl << "->";
	std::cin >> selectedOption;
	std::cout << std::endl << "How many iterations do you want?" << std::endl;
	std::cin >> numberOfIterations;

	switch (selectedOption)
	{
	case 1:
		RunGameFromCustomMatrix(numberOfIterations, 0);
		break;
	case 2:
		RunGameFromImage(numberOfIterations, 0);
	default:
		break;
	}
	return 0;
}