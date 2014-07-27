#include "Matrix.h"

void Matrix::set(int r, int c, float n)
{
	matrix[r*row + c] = n;
}
float Matrix::get(int r, int c)
{
	return matrix[r*row + c];
}

void Matrix::print()
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			printf("%f ", matrix[i*row + j]);
		}
		printf("\n");
	}
}
void Matrix::init(int r,int c)
{
	row = r;
	col = c;
	matrix = new float[r*c];
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			matrix[i*row + j]=0;
		}
	}
}