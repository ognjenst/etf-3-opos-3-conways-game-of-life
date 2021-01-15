#include "GameOfLifeLib.h"
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <CL/cl.h>
#include <string>
#include <thread>
#include <stdio.h>

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);


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


void LoadCustomMatrix(unsigned char*& buffer, int& width, int& height)
{
	width = 150;
	height = 150;
	unsigned char* image = new unsigned char[width * height];
	std::fill(image, image + width * height, 255);
	for (int i = 0; i < width * height; i++)
		image[i] = 255;


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

void RunTheGame(unsigned char*& buffer, const int width, const int height, int numberOfIterations, int startIteration = 0, int numberOfWorkItems = 8)
{
	int imageSize = width * height;
	unsigned char* buffer2, * buffer3, * buffer1;
	buffer1 = new unsigned char[(size_t)width * height];
	buffer2 = new unsigned char[(size_t)width * height];
	buffer3 = new unsigned char[(size_t)width * height];
	bool repetition = false;


	cl_mem d_a;
	cl_mem d_ca;					  // Buffer for the first iteration of image
	cl_mem d_cb;					  // Buffer for the second iteration of image
	cl_platform_id cpPlatform;        // OpenCL platform
	cl_device_id device_id;           // device ID
	cl_context context;               // context
	cl_command_queue queue;           // command queue
	cl_program program;               // program
	cl_program programCorrection;     // Correction values program
	cl_kernel kernel;                 // kernel
	cl_kernel kernelCorrection;		  // Correction values kernel

	cl_event event;


	size_t globalSize[2], localSize[2];
	cl_int err;

	// Number of work items in each local work group
	localSize[0] = localSize[1] = numberOfWorkItems;

	// Number of total work items - localSize must be devisor
	globalSize[0] = width;
	globalSize[1] = height;

	// Bind to platform
	err = clGetPlatformIDs(1, &cpPlatform, NULL);

	// Get ID for the device
	err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

	// Create a context  
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

	// Create a command queue 
	queue = clCreateCommandQueue(context, device_id, 0, &err);

	char* kernelSource = readKernelSource("GameOfLife.cl");
	char* kernelCorrectionSource = readKernelSource("kernelCorrection.cl");

	// Create the compute program from the source buffer
	program = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, NULL, &err);
	programCorrection = clCreateProgramWithSource(context, 1, (const char**)&kernelCorrectionSource, NULL, &err);

	// Build the program executable
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	err = clBuildProgram(programCorrection, 0, NULL, NULL, NULL, NULL);

	if (err)
	{
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		clGetProgramBuildInfo(programCorrection, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char* log = (char*)malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		clGetProgramBuildInfo(programCorrection, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		printf("%s\n", log);

		free(log);
	}

	// Create the compute kernel in the program we wish to run
	kernel = clCreateKernel(program, "gameOfLife", &err);
	kernelCorrection = clCreateKernel(programCorrection, "correctionGameOfLife", &err);

	size_t kernelWorkGroupSize = 0;
	clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &kernelWorkGroupSize, nullptr);
	clGetKernelWorkGroupInfo(kernelCorrection, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &kernelWorkGroupSize, nullptr);

	// Create the input and output arrays in device memory for our calculation
	d_a = clCreateBuffer(context, CL_MEM_READ_WRITE, imageSize, NULL, NULL);

	// Write our data set into the input array in device memory
	err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0, imageSize, buffer, 0, NULL, NULL);

	// Set the arguments to our compute kernel
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
	err |= clSetKernelArg(kernel, 1, sizeof(int), &width);

	err |= clSetKernelArg(kernelCorrection, 0, sizeof(cl_mem), &d_a);
	err |= clSetKernelArg(kernelCorrection, 1, sizeof(int), &width);


	for (int i = 0; i < numberOfIterations; i++)
	{

		// Execute the kernel over the entire range of the data set  
		err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, localSize, 0, NULL, &event);

		// Wait for the command queue to get serviced before reading back results
		clWaitForEvents(1, &event);

		// Coorect values for kernel data set
		clEnqueueNDRangeKernel(queue, kernelCorrection, 2, NULL, globalSize, localSize, 0, NULL, &event);
		clWaitForEvents(1, &event);

		// Read the results from the device
		clEnqueueReadBuffer(queue, d_a, CL_TRUE, 0, imageSize, buffer, 0, NULL, &event);
		clWaitForEvents(1, &event);

		const std::string outFile = std::string("image") + std::to_string(i + 1) + std::string(".pgm");
		writeImage(outFile.c_str(), buffer, width, height);
		// release OpenCL resources

		//// Detection
		//memcpy(buffer1, buffer, (size_t)(height * width));
		/*if (!memcmp(buffer3, buffer, (size_t)(height * width)))
		{
			unsigned char* image = new unsigned char[height * width * BYTES_PER_PIXEL];
			char* imageFileName = (char*)"bitmapImage.bmp";

			int a, b;
			for (a = 0; a < height; a++) {
				for (b = 0; b < width; b++) {
					if (buffer[a * width + b] == 0)
					{
						*(image + ((height - 1) - a) * width * BYTES_PER_PIXEL + b * BYTES_PER_PIXEL + 2) = (unsigned char)(255);             ///red
						*(image + ((height - 1) - a) * width * BYTES_PER_PIXEL + b * BYTES_PER_PIXEL + 1) = (unsigned char)(0);              ///green
						*(image + ((height - 1) - a) * width * BYTES_PER_PIXEL + b * BYTES_PER_PIXEL + 0) = (unsigned char)(0); ///blue
					}
					else
					{
						*(image + ((height - 1) - a) * width * BYTES_PER_PIXEL + b * BYTES_PER_PIXEL + 2) = (unsigned char)(255);             ///red
						*(image + ((height - 1) - a) * width * BYTES_PER_PIXEL + b * BYTES_PER_PIXEL + 1) = (unsigned char)(255);              ///green
						*(image + ((height - 1) - a) * width * BYTES_PER_PIXEL + b * BYTES_PER_PIXEL + 0) = (unsigned char)(255); ///blue
					}
				}
			}*/

			/*generateBitmapImage((unsigned char*)image, height, width, imageFileName);
			repetition = true;*/

		//}

		/*if (i >= startIteration)
		{
			const std::string outFile = std::string("image") + std::to_string(i + 1) + std::string(".pgm");
			writeImage(outFile.c_str(), buffer, width, height);
		}*/
	}
	// END OF FOR


	clReleaseMemObject(d_a);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	//release host memory
	free(kernelSource);
}


// Main program functions

void RunGameFromImage(int numberOfIterations, int startIteration)
{
	int width = -1;
	int height = -1;
	unsigned char* buffer = nullptr;
	readImage("image0.pgm", buffer, width, height);
	//readImage("simage0.pgm", buffer, width, height);
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

	RunTheGame(buffer, width, height, numberOfIterations, startIteration, 5);
}


// BMP Generation
void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName)
{
	int widthInBytes = width * BYTES_PER_PIXEL;

	unsigned char padding[3] = { 0, 0, 0 };
	int paddingSize = (4 - (widthInBytes) % 4) % 4;

	int stride = (widthInBytes)+paddingSize;

	FILE* imageFile = fopen(imageFileName, "wb");

	unsigned char* fileHeader = createBitmapFileHeader(height, stride);
	fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

	unsigned char* infoHeader = createBitmapInfoHeader(height, width);
	fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

	int i;
	for (i = 0; i < height; i++) {
		fwrite(image + (i * widthInBytes), BYTES_PER_PIXEL, width, imageFile);
		fwrite(padding, 1, paddingSize, imageFile);
	}

	fclose(imageFile);
}

unsigned char* createBitmapFileHeader(int height, int stride)
{
	int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

	static unsigned char fileHeader[] = {
		0,0,     /// signature
		0,0,0,0, /// image file size in bytes
		0,0,0,0, /// reserved
		0,0,0,0, /// start of pixel array
	};

	fileHeader[0] = (unsigned char)('B');
	fileHeader[1] = (unsigned char)('M');
	fileHeader[2] = (unsigned char)(fileSize);
	fileHeader[3] = (unsigned char)(fileSize >> 8);
	fileHeader[4] = (unsigned char)(fileSize >> 16);
	fileHeader[5] = (unsigned char)(fileSize >> 24);
	fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

	return fileHeader;
}

unsigned char* createBitmapInfoHeader(int height, int width)
{
	static unsigned char infoHeader[] = {
		0,0,0,0, /// header size
		0,0,0,0, /// image width
		0,0,0,0, /// image height
		0,0,     /// number of color planes
		0,0,     /// bits per pixel
		0,0,0,0, /// compression
		0,0,0,0, /// image size
		0,0,0,0, /// horizontal resolution
		0,0,0,0, /// vertical resolution
		0,0,0,0, /// colors in color table
		0,0,0,0, /// important color count
	};

	infoHeader[0] = (unsigned char)(INFO_HEADER_SIZE);
	infoHeader[4] = (unsigned char)(width);
	infoHeader[5] = (unsigned char)(width >> 8);
	infoHeader[6] = (unsigned char)(width >> 16);
	infoHeader[7] = (unsigned char)(width >> 24);
	infoHeader[8] = (unsigned char)(height);
	infoHeader[9] = (unsigned char)(height >> 8);
	infoHeader[10] = (unsigned char)(height >> 16);
	infoHeader[11] = (unsigned char)(height >> 24);
	infoHeader[12] = (unsigned char)(1);
	infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL * 8);

	return infoHeader;
}