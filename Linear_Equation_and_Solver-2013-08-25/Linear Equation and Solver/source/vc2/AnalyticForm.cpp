#include "AnalyticForm.h"
#include "NetList.h"

void AnalyticForm::Prepare(int m, int n)
{
	this->m = m;
	this->n = n;

	A = new double*[m];
	for(int i=0; i<m; i++)
	{
		A[i] = new double[n];
	}
	bx = new double[m];
	by = new double[m];

	ZeroMatrices();
}

void AnalyticForm::ZeroMatrices()
{
	for(int i=0; i<m; i++)
	{
		for(int j=0; j<n; j++)
		{
			A[i][j] = 0;
		}
		bx[i]=0;
		by[i]=0;
	}
}

void AnalyticForm::ComputeBlockPair(
	vector <Block>::iterator it1,
	vector <Block>::iterator it2
)
{
	if(it1->bFixed && it2->bFixed)
		throw(std::runtime_error("Fixed blocks connected on same net."));
	
	if(it2->bFixed)
	{
		// fixed x movable block
		A[it1->num-m_offset][it1->num-n_offset] += 2;
		bx[it1->num-m_offset] += 2*it2->x;
		by[it1->num-m_offset] += 2*it2->y;
	}
	else if( it1->bFixed )
	{
		// fixed x movable block
		A[it2->num-m_offset][it2->num-n_offset] += 2;
		bx[it2->num-m_offset] += 2*it1->x;
		by[it2->num-m_offset] += 2*it1->y;
	}
	else
	{
		// moveable x movable block	
		A[it1->num-m_offset][it1->num-n_offset] += 2;
		A[it2->num-m_offset][it2->num-n_offset] += 2;
		A[it1->num-m_offset][it2->num-n_offset] -= 2;
		A[it2->num-m_offset][it1->num-n_offset] -= 2;
	}
}

AnalyticForm::AnalyticForm(NetList& netlist)
{
	int size = netlist.blocksCount-netlist.fixedBlocksCount;
	m_offset = n_offset = netlist.fixedBlocksCount+1;

	Prepare( size, size);

	vector <Net>::iterator itNet;
	vector <Block>::iterator itBlk;
	vector <Block>::iterator itBlk2;
	
	// Looping for all nets
	for(
		itNet = netlist.nets.nets.begin();
		itNet != netlist.nets.nets.end();
		itNet++)
	{
		// Take all block pairs in this net
		for(
			itBlk = itNet->blocks.begin();
			itBlk != itNet->blocks.end();
			itBlk++)
		{
			for(
				itBlk2 = itBlk+1;
				itBlk2 != itNet->blocks.end();
				itBlk2++)
			{
				// Compute matrix elements for 
				// block pair itBlk and itBlk2
				ComputeBlockPair(itBlk, itBlk2);
			}
		}
	}

}

AnalyticForm::~AnalyticForm(void)
{
	for(int i=0; i<m; i++)
	{
		delete [] A[i];
	}

	delete[] bx;
	delete[] by;
	delete[] A;

}

void AnalyticForm::print()
{
	output << endl;
	output << "========================================== " << endl;
	output << "============= ANALYTIC FORM ============== " << endl;
	output << "========================================== " << endl;

	int i;
	output << "A=" << endl;
	for(i=0; i<m; i++)
	{
		for(int j=0; j<n; j++)
		{
			output << setw(4) << A[i][j] ;
		}
		output << endl;
	}

	output << "========================================== " << endl;
	output << "bx=" << endl;
	for(i=0; i<m; i++)
	{
		output << setw(5) << bx[i] << endl;
	}

	output << "========================================== " << endl;
	output << "by=" << endl;
	for(i=0; i<m; i++)
	{
		output << setw(5) << by[i] << endl;
	}
}
