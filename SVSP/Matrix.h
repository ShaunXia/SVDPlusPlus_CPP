#ifndef MATRIX_H
#define MATRIX_H
#include <stdio.h>
#include <vector>
using namespace std;
class Matrix
{
public:
	Matrix(){};
	Matrix(int r, int c){
		row = r;
		col = c;
		matrix = new float[r*c];
	};
	void init(int, int);
	void set(int, int, float);
	float get(int, int);
	void print();
	int getColVector(int, vector<float>*);
	int getRowVector(int, vector<float>*);
private:
	float *matrix;
	int col;
	int row;
};
#endif