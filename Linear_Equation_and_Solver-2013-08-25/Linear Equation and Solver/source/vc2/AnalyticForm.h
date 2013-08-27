#pragma once

#include "common.h"

class NetList;
class Block;

class AnalyticForm
{
public:
	void Prepare(int m, int n);

	void ZeroMatrices();

	AnalyticForm(NetList& netlist);

	double** A;

	double* bx;

	double* by;

	int m,n;

	int m_offset, n_offset;	// skip fixed blocks

	void ComputeBlockPair(
		vector <Block>::iterator it1,
		vector <Block>::iterator it2);

	void print();

public:
	~AnalyticForm(void);
};
