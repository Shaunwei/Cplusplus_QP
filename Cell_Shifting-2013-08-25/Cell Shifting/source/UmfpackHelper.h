#pragma once

class UmfpackHelper
{

	int *Ap;

	int *Ai;

	double *Ax;

	double* bx;

	double* by;

	double* x;

	double* y;

	int m,n;

	void Solve();

public:

	UmfpackHelper(int m, int n, double**A, double*bx, double*by);
	void print();
       
        double getCellLocationX(const int cellIndex)
	{
	 return x[cellIndex];
	}

	double getCellLocationY(const int cellIndex)
	{
	 return y[cellIndex];
	}

        

public:
	~UmfpackHelper(void);
};
