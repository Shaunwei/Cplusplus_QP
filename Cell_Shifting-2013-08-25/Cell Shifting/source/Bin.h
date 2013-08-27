#ifndef BIN_H_
#define BIN_H_

#include "common.h"

class NetList;
class Block;
class AnalyticForm;
class UmfpackHelper;

// Class: CellInfo 
//  Info: Holds placement information about a single cell.
class CellInfo 
{
 public:
        CellInfo(int cellIndex, double locationX, double locationY);
       ~CellInfo(void);
        
        int cellId;
        double originalLocX, originalLocY;
        double targetLocX, targetLocY; 
        //
        double pX, pY;
};

// Class: Bin
//  Info: Represents a single bin.
class Bin 
{
public:
        Bin(void);
        void setEqualBinLocations(const int startX, const int startY,
                                  const int endX, const int endY);
        void findBinUtilization(void);
        void calculateUnequalBoundryX(Bin *nextBinPtr);
        void calculateUnequalBoundryY(Bin *nextBinPtr);
        void linearlyShiftCellX(const double ax);
        void linearlyShiftCellY(const double ay);
        void processPseudoPins(NetList* netlistPtr, AnalyticForm* aformPtr, 
                     double* x_lastSolverOutput,double* y_lastSolverOutput);
        void findResultantForce(NetList* netlistPtr, CellInfo* cellInfoPtr, AnalyticForm* aformPtr, 
                 double* pFx, double* pFy, double* x_lastSolverOutput,double* y_lastSolverOutput);
        void spCase1(NetList* netlistPtr, CellInfo* cellPtr, 
                     double *pX, double *pY, double *pDx, double *pDy);
        void spCase2(NetList* netlistPtr, CellInfo* cellPtr, 
                     double *pX, double *pY, double *pDx, double *pDy);
        void spCase3(NetList* netlistPtr, CellInfo* cellPtr, 
                     double *pX, double *pY, double *pDx, double *pDy);
        void spCase4(NetList* netlistPtr, CellInfo* cellPtr, 
                     double *pX, double *pY, double *pDx, double *pDy);

        void clearCellsInBin(void);
        void print();
        ~Bin(void);

//private:
	// Initial equal bin start/end locations.
        int binEqualStartLocX, binEqualStartLocY;
        int binEqualEndLocX, binEqualEndLocY;
        // Unequal bin start/end locations. 
        float binUnEqualStartLocX, binUnEqualStartLocY;
        float binUnEqualEndLocX, binUnEqualEndLocY;

        double binUtilization;
        double binArea;
        vector<CellInfo*> cellsInBin;
        static bool movControlEnable;
};


#endif /* BIN_H_ */
