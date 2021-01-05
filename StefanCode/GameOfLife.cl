__kernel void gameOfLife(__global unsigned char* mat, const int n)
{
	int row = get_global_id(0);
	int col = get_global_id(1);

	if (row == 0 || row == n - 1 || col == 0 || col == n - 1)
	{
		return;
	}

	int liveNeighbours = 0;
	int isAlive = !(mat[(row)*n + (col)]);

	//mat[(row)*n + (col)] = 128 * isAlive;

	liveNeighbours += ((mat[(row - 1) * n + (col - 1)] == 0) || (mat[(row - 1) * n + (col - 1)] == 100)); //1
	liveNeighbours += ((mat[(row - 1) * n + (col)] == 0) || (mat[(row - 1) * n + (col)] == 100)); //2
	liveNeighbours += ((mat[(row - 1) * n + (col + 1)] == 0) || (mat[(row - 1) * n + (col + 1)] == 100)); //3
	liveNeighbours += ((mat[(row) * n + (col - 1)] == 0) || (mat[(row) * n + (col - 1)] == 100)); //4
	liveNeighbours += ((mat[(row) * n + (col + 1)] == 0) || (mat[(row) * n + (col + 1)] == 100)); //6
	liveNeighbours += ((mat[(row + 1) * n + (col - 1)] == 0) || (mat[(row + 1) * n + (col - 1)] == 100)); //7
	liveNeighbours += ((mat[(row + 1) * n + (col)] == 0) || (mat[(row + 1) * n + (col)] == 100)); //8
	liveNeighbours += ((mat[(row + 1) * n + (col + 1)] == 0) || (mat[(row + 1) * n + (col + 1)] == 100)); //9
	

	//liveNeighbours += !(mat[(row - 1) * n + (col - 1)]); //1
	//liveNeighbours += !(mat[(row - 1) * n + (col)]); //2
	//liveNeighbours += !(mat[(row - 1)*n + (col + 1)]); //3
	//liveNeighbours += !(mat[(row)*n + (col - 1)]); //4
	//liveNeighbours += !(mat[(row)*n + (col + 1)]); //6
	//liveNeighbours += !(mat[(row + 1)*n + (col - 1)]); //7
	//liveNeighbours += !(mat[(row + 1) * n + (col)]); //8
	//liveNeighbours += !(mat[(row + 1) * n + (col + 1)]); //9

	if ((isAlive && liveNeighbours == 2) || (liveNeighbours == 3))
	{
		if(isAlive)
		{
			mat[(row)*n + (col)] = 0;
		}
		else
		{
		mat[(row)*n + (col)] = 100;
		}

	}
	else
	{
		if(!isAlive)
		{
			mat[(row)*n + (col)] = 255;
		}
		else
		{
			mat[(row)*n + (col)] = 200;
		}
	}
}