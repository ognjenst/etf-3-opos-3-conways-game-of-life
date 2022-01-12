__kernel void detectionKernel(__global unsigned char* matA, __global unsigned char* matB, const int n, __global int* flag)
{
	int row = get_global_id(0);
	int col = get_global_id(1);

	if (row == 0 || row == n - 1 || col == 0 || col == n - 1)
	{
		return;
	}


	if(matA[(row)*n + (col)] != matB[(row)*n + (col)])
	{
		flag[0] += 1;
	}
}