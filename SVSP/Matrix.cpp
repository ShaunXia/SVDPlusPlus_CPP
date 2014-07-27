#include "Matrix.h"

void Matrix::set(int r, int c, float n)
{
	matrix[r*col + c] = n;
}
float Matrix::get(int r, int c)
{
	return matrix[r*col + c];
}

void Matrix::print()
{
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			printf("%f ", matrix[i*col + j]);
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
			matrix[i*col + j]=0;
		}
	}
}


int Matrix::getColVector(int col, vector<float> *output)
{
	for (int i = 0; i <row; i++)
	{
		output->push_back(get(i, col));
	}
	return 1;
}

int Matrix::getRowVector(int row, vector<float> *output)
{
	for (int i = 0; i <col; i++)
	{
		output->push_back(get(row, i));
	}
	return 1;
}
