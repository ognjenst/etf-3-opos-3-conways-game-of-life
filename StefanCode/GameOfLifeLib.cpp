#include "GameOfLifeLib.h"
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <CL/cl.h>
#include <string>
#include <thread>


bool isPrime(int num) {
	bool flag = true;
	for (int i = 2; i <= num / 2; i++) {
		if (num % i == 0) {
			flag = false;
			break;
		}
	}
	return flag;
}

char* readKernelSource(const char* filename)
{
	char* kernelSource = nullptr;
	long length;
	FILE* f = fopen(filename, "r");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		kernelSource = (char*)calloc(length, sizeof(char));
		if (kernelSource)
			fread(kernelSource, 1, length, f);
		fclose(f);
	}
	return kernelSource;
}

void readImage(const char* filename, unsigned char*& array, int& width, int& height)
{
	FILE* fp = fopen(filename, "rb"); /* b - binary mode */
	if (!fscanf(fp, "P5\n%d %d\n255\n", &width, &height)) {
		throw "error";
	}
	unsigned char* image = new unsigned char[(size_t)width * height];
	fread(image, sizeof(unsigned char), (size_t)width * (size_t)height, fp);
	fclose(fp);
	array = image;
}

void writeImage(const char* filename, unsigned char* array, const int width, const int height)
{
	FILE* fp = fopen(filename, "wb"); /* b - binary mode */
	fprintf(fp, "P5\n%d %d\n255\n", width, height);
	fwrite(array, sizeof(unsigned char), (size_t)width * (size_t)height, fp);
	fclose(fp);
}

void LoadCustomMatrix(unsigned char*& buffer, int &width, int &height)
{
	width = 50;
	height = 50;
	unsigned char* image = new unsigned char[width * height];
	std::fill(image, image + 100, 255);
	image[3 * width + 3] = 0;
	image[3 * width + 7] = 0;
	image[4 * width + 5] = 0;
	image[5 * width + 4] = 0;
	image[5 * width + 6] = 0;
	image[6 * width + 5] = 0;
	image[7 * width + 3] = 0;
	image[7 * width + 7] = 0;

	image[3 * width + 13] = 0;
	image[3 * width + 17] = 0;
	image[4 * width + 15] = 0;
	image[5 * width + 14] = 0;
	image[5 * width + 16] = 0;
	image[6 * width + 15] = 0;
	image[7 * width + 13] = 0;
	image[7 * width + 17] = 0;

	image[3 * width + 23] = 0;
	image[3 * width + 27] = 0;
	image[4 * width + 25] = 0;
	image[5 * width + 24] = 0;
	image[5 * width + 26] = 0;
	image[6 * width + 25] = 0;
	image[7 * width + 23] = 0;
	image[7 * width + 27] = 0;
	buffer = image;
}

void RunTheGame(unsigned char*& buffer, int width, int height, int numberOfIterations, int startIteration = 0, int numberOfWorkItems = 8)
{
	int imageSize = width * height;
	unsigned char *buffer2, *buffer3, *buffer1;
	buffer1 = new unsigned char[(size_t)width * height];
	buffer2 = new unsigned char[(size_t)width * height];
	buffer3 = new unsigned char[(size_t)width * height];
	for (int i = 0; i < numberOfIterations; i++)
	{
		cl_mem d_a;
		cl_platform_id cpPlatform;        // OpenCL platform
		cl_device_id device_id;           // device ID
		cl_context context;               // context
		cl_command_queue queue;           // command queue
		cl_program program;               // program
		cl_kernel kernel;                 // kernel

		size_t globalSize[2], localSize[2];
		cl_int err;

		// Number of work items in each local work group
		localSize[0] = localSize[1] = numberOfWorkItems;

		// Number of total work items - localSize must be devisor
		globalSize[0] = width;
		globalSize[1] = width;

		// Bind to platform
		err = clGetPlatformIDs(1, &cpPlatform, NULL);

		// Get ID for the device
		err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

		// Create a context  
		context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

		// Create a command queue 
		queue = clCreateCommandQueue(context, device_id, 0, &err);

		char* kernelSource = readKernelSource("GameOfLife.cl");

		// Create the compute program from the source buffer
		program = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, NULL, &err);

		// Build the program executable 
		err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

		if (err)
		{
			// Determine the size of the log
			size_t log_size;
			clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

			// Allocate memory for the log
			char* log = (char*)malloc(log_size);

			// Get the log
			clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

			// Print the log
			printf("%s\n", log);

			free(log);
		}

		// Create the compute kernel in the program we wish to run
		kernel = clCreateKernel(program, "gameOfLife", &err);

		size_t kernelWorkGroupSize = 0;
		clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &kernelWorkGroupSize, nullptr);

		// Create the input and output arrays in device memory for our calculation
		d_a = clCreateBuffer(context, CL_MEM_READ_WRITE, imageSize, NULL, NULL);

		// Write our data set into the input array in device memory
		err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0, imageSize, buffer, 0, NULL, NULL);

		// Set the arguments to our compute kernel
		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
		err |= clSetKernelArg(kernel, 1, sizeof(int), &width);

		// Execute the kernel over the entire range of the data set  
		err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, localSize, 0, NULL, NULL);

		// Wait for the command queue to get serviced before reading back results
		clFinish(queue);

		memcpy(buffer3, buffer2, (size_t)(height * width));
		memcpy(buffer2, buffer1, (size_t)(height * width));

		// Read the results from the device
		clEnqueueReadBuffer(queue, d_a, CL_TRUE, 0, imageSize, buffer, 0, NULL, NULL);

		clFinish(queue);

		// release OpenCL resources
		clReleaseMemObject(d_a);
		clReleaseProgram(program);
		clReleaseKernel(kernel);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);

		//release host memory
		free(kernelSource);

		// Fix quantum values
		for (int i = 0; i < width * height; i++)
		{
			if (buffer[i] == 100) buffer[i] = 0;
			if (buffer[i] == 200) buffer[i] = 255;
		}

		//// Detection
		memcpy(buffer1, buffer, (size_t)(height * width));
		if (!memcmp(buffer3, buffer, (size_t)(height * width)))
		{

			std::cout << "OP";
			//unsigned char bmpfile_header[54];
			//FILE* fp = fopen("white.bmp", "wb");
			//fread(bmpfile_header, sizeof(unsigned char), 54, fp); // read the 54-byte
			//unsigned char data[] = {'0'};
			//fseek(fp, 54, SEEK_SET);
			//for (int i = 0; i < 256; i++) {
			//	for (int j = 0; j < 256; j++) {
			//		/*buffer[i * 256 + j];*/
			//		fwrite(data, 1, 1, fp);
			//		fwrite(data, 1, 1, fp);
			//		fwrite(data, 1, 1, fp);
			//	}
			//}

			//fclose(fp);
		}

		if (i >= startIteration)
		{
			const std::string outFile = std::string("image") + std::to_string(i + 1) + std::string(".pgm");
			writeImage(outFile.c_str(), buffer, width, height);
		}
	}
}

void RunGameFromImage(int numberOfIterations, int startIteration)
{
	int width = -1;
	int height = -1;
	unsigned char* buffer = nullptr;
	readImage("image0.pgm", buffer, width, height);
	RunTheGame(buffer, width, height, numberOfIterations, startIteration);
}

void RunGameFromCustomMatrix(int numberOfIterations, int startIteration)
{
	int width = -1;
	int height = -1;
	int devisor = 1;
	unsigned char* buffer = nullptr;
	LoadCustomMatrix(buffer, width, height);

	if (width * height % 8 == 0) devisor = 8;
	else if (width * height % 7 == 0) devisor = 7;
	else if (width * height % 6 == 0) devisor = 6;
	else if (width * height % 5 == 0) devisor = 5;
	else if (width * height % 4 == 0) devisor = 4;
	else if (width * height % 3 == 0) devisor = 3;
	else if (width * height % 2 == 0) devisor = 2;

	RunTheGame(buffer, width, height, numberOfIterations, startIteration,  5);
}