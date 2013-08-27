#include "common.h"

#include "Umfpack.h"

#include "UmfpackHelper.h"

UmfpackHelper::UmfpackHelper(int m, int n, double**A, double*bx, double*by)
{
	this->m = m;
	this->n = n;

	this->bx = new double[m];
	this->by = new double[m];
	this->x = new double[m];
	this->y = new double[m];
	
	int *Ti = new int[m*n];
	int *Tj = new int[m*n];
	double *Tx = new double[m*n];

	Ap = new int[n+1];
	Ai = new int[m*n];
	Ax = new double[m*n];

	for(int i=0; i<m; i++)
	{
		for(int j=0; j<n; j++)
		{
			Ti[i*m+j] = i;
			Tj[i*m+j] = j;
			Tx[i*m+j] = A[i][j];
		}
		this->bx[i] = bx[i];
		this->by[i] = by[i];
	}

	umfpack_di_triplet_to_col(m, n, m*n, Ti, Tj, Tx, Ap, Ai, Ax, (int *) NULL);

	delete[] Ti;
	delete[] Tj;
	delete[] Tx;

	Solve();
}


UmfpackHelper::~UmfpackHelper(void)
{
	delete[] Ap;
	delete[] Ai;
	delete[] Ax;
	delete[] bx;
	delete[] by;
	delete[] x;
	delete[] y;
}

void UmfpackHelper::Solve()
{
	double *null = (double *) NULL ;
	void *Symbolic, *Numeric ;
	(void) umfpack_di_symbolic (m, n, Ap, Ai, Ax, &Symbolic, null, null) ;
	(void) umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, null, null) ;
	umfpack_di_free_symbolic (&Symbolic) ;

	(void) umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, x, bx, Numeric, null, null) ;
	(void) umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, y, by, Numeric, null, null) ;

	umfpack_di_free_numeric (&Numeric) ;
}


void UmfpackHelper::print()
{
	output << endl;
	output << "========================================== " << endl;
	output << "================ SOLUTION ================ " << endl;
	output << "========================================== " << endl;
	for (int i = 0 ; i < n ; i++) 
		//printf ("(x,y) [%d] = %3g,%3g\n", i, x [i], y [i]) ;
		output << "x,y [" << setw(3) << i << "]: " << setw(6) << x [i] << " , "<< setw(6) << y [i] << endl;
}
