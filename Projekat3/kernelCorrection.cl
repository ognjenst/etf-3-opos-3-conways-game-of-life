// Kernel that converts values back to binary

__kernel void correctionGameOfLife(__global unsigned char* mat, const int n)
{
	int row = get_global_id(0);
	int col = get_global_id(1);

	if (mat[(row)*n + (col)] == 100)
	{
		 mat[(row)*n + (col)] = 0;
	}
	if (mat[(row)*n + (col)] == 200)
	{
		mat[(row)*n + (col)] = 255;
	}
}
