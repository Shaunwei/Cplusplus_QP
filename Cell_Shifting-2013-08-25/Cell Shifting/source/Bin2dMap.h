#ifndef BIN2dMAP_H_
#define BIN2dMAP_H_

#include "common.h"

class NetList;
class Block;
class AnalyticForm;
class UmfpackHelper;
class CellInfo;
class Bin;

class Bin2dMap
{
public:
	Bin2dMap(NetList* netlist, AnalyticForm* aform, UmfpackHelper* umfpack, 
                 const int binsRowCount, const int binsColCount, const int itrCount);
        void doCellShifting(void);
         void createRegularBins(void);
         void addCellsToBins(void);
         void createUnEqualBinsRows(void);
         void createUnEqualBinsColumns(void);
         void addSpreadingForces(void);
         void resetBinsInfo(void);
         void saveOriginalMatrixValues(void);
         void saveLastSolverOutput(UmfpackHelper *umfLast);
         void restoreOrigMatrixValues(void);
        void computeWireLengthHPWL(void);
        void findMaxUtilization(void);
        void initGraphics(void);

	void print();
	~Bin2dMap(void);

//private: 
 	vector< vector<Bin*> > binsRowColVec;
	int binsRowCount, binsColCount;
	int binsXdelta, binsYdelta;
	NetList* netlistPtr;
 	AnalyticForm* aformPtr;
        UmfpackHelper* umfpackPtr;
        double** A_orig;
        double* bx_orig; 
        double* by_orig;
        double* x_lastSolverOutput;
        double* y_lastSolverOutput;
        double  maxBinUtilization;
        double  wireLengthTotal;
           int  currentIteration, iterationCount;

};

void setBinMapPtr(Bin2dMap *bp);

#endif /* BIN2dMAP_H_ */
