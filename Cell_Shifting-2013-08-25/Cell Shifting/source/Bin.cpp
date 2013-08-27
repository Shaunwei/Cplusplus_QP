#include <math.h>
#include "Bin2dMap.h"
#include "AnalyticForm.h"
#include "UmfpackHelper.h"
#include "NetList.h"
#include "Bin.h"


///////////////////////////////////
//// Class CellInfo Implementation.
///////////////////////////////////

// Method: CellInfo()
//   Info: Constructor initializes cell id and placement location.
CellInfo::CellInfo(int cellIndex, double locationX, double locationY)
{
 cellId = cellIndex;
 originalLocX = locationX;  
 originalLocY = locationY;

 targetLocX = 0.0; 
 targetLocY = 0.0;
}

CellInfo::~CellInfo(void)
{

}

/////////////////////////////
// Class Bin Implementation.
/////////////////////////////
bool Bin::movControlEnable = true;

// Method: Bin()
//   Info: Constructor initializes values to zero. 
Bin::Bin()
{
 binEqualStartLocX = 0; 
 binEqualStartLocY = 0;
 binEqualEndLocX = 0; 
 binEqualEndLocY = 0;
        
 binUnEqualStartLocX = 0; 
 binUnEqualStartLocY = 0;
 binUnEqualEndLocX = 0; 
 binUnEqualEndLocY = 0;
 
 binUtilization = 0.0;
 binArea = 0.0;
}

// Method: setEqualBinLocations()
//   Info: Sets the x,y locations of equal size bin.
void Bin::setEqualBinLocations(const int startX, const int startY, 
                               const int endX, const int endY)
{
 binEqualStartLocX = startX;
 binEqualStartLocY = startY;
 binEqualEndLocX = endX;
 binEqualEndLocY = endY; 
 binArea = (endX-startX) * (endY-startY);

 binUnEqualStartLocX = startX;
 binUnEqualStartLocY = startY;
 binUnEqualEndLocX = endX;
 binUnEqualEndLocY = endY;
}

// Method: Bin::findBinUtilization()
//   Info: Calculate the bin utilization from cells inside the bin. 
void Bin::findBinUtilization(void)
{
 // Each cell is unit size. Utilization is cell count divided by bin area.
 binUtilization = (cellsInBin.size()) / binArea ;
}

// Method: calculateUnequalBoundryX()
//   Info: Calculate unequal bin boundry in X direction. 
void Bin::calculateUnequalBoundryX(Bin *nextBinPtr)
{
 // Delta value provided in the algorithm.
 const double deltaValue = 1.5;

 binUnEqualEndLocX = ( (binEqualStartLocX * (nextBinPtr->binUtilization + deltaValue)) +
                     (nextBinPtr->binEqualEndLocX * (binUtilization + deltaValue)) ) / 
                     (binUtilization + nextBinPtr->binUtilization + (2 * deltaValue));

 // Also update the start bin boundry of the next bin. 
 nextBinPtr->binUnEqualStartLocX = binUnEqualEndLocX;
}

// Method: calculateUnequalBoundryY()
//   Info: Calculate unequal bin boundry in Y direction. 
void Bin::calculateUnequalBoundryY(Bin *nextBinPtr)
{
 // Delta value provided in the algorithm.
 const double deltaValue = 1.5;

 binUnEqualEndLocY = ( (binEqualStartLocY * (nextBinPtr->binUtilization + deltaValue)) +
                     (nextBinPtr->binEqualEndLocY * (binUtilization + deltaValue)) ) /
                     (binUtilization + nextBinPtr->binUtilization + (2 * deltaValue));

 // Also update the start bin boundry of the next bin.
 nextBinPtr->binUnEqualStartLocY = binUnEqualEndLocY;
}

// Method: linearlyShiftCellX()
//   Info: Linearly shift cells to unequal bin structure in x-direction. 
void Bin::linearlyShiftCellX(const double ax)
{
 vector<CellInfo*>::iterator itrCell;
 double targetLocXTemp;

 for(itrCell= cellsInBin.begin(); itrCell != cellsInBin.end(); itrCell++)
    {
     if( ! Bin::movControlEnable )
       {
        (*itrCell)->targetLocX = ((binUnEqualEndLocX * ((*itrCell)->originalLocX - binEqualStartLocX)) +
                               (binUnEqualStartLocX * (binEqualEndLocX - (*itrCell)->originalLocX))) /
                              (binEqualEndLocX - binEqualStartLocX);
       }
     else
       {
      
        // Movement control (uncomment to use this option)
       targetLocXTemp = ((binUnEqualEndLocX * ((*itrCell)->originalLocX - binEqualStartLocX)) +
                               (binUnEqualStartLocX * (binEqualEndLocX - (*itrCell)->originalLocX))) /
                              (binEqualEndLocX - binEqualStartLocX);

       if( targetLocXTemp < (*itrCell)->originalLocX ) 
         {
          (*itrCell)->targetLocX = (*itrCell)->originalLocX - (ax * fabs( (targetLocXTemp - (*itrCell)->originalLocX)));
         }
       else if( targetLocXTemp > (*itrCell)->originalLocX )
         {
          (*itrCell)->targetLocX = (*itrCell)->originalLocX + (ax * fabs( (targetLocXTemp - (*itrCell)->originalLocX)));
         }
       else
        {
         (*itrCell)->targetLocX = (*itrCell)->originalLocX;
        }
       }
     
    }
}

// Method: linearlyShiftCellY()
//   Info: Linearly shift cells to unequal bin structure in y-direction.
void Bin::linearlyShiftCellY(const double ay)
{
 vector<CellInfo*>::iterator itrCell;
 double targetLocYTemp;

 for(itrCell= cellsInBin.begin(); itrCell != cellsInBin.end(); itrCell++)
    {
     if( ! Bin::movControlEnable )
       {
        (*itrCell)->targetLocY = ((binUnEqualEndLocY * ((*itrCell)->originalLocY - binEqualStartLocY)) +
                               (binUnEqualStartLocY * (binEqualEndLocY - (*itrCell)->originalLocY))) /
                              (binEqualEndLocY - binEqualStartLocY);
       }
     else
       {
        // Movement control (uncomment to use this option)
        targetLocYTemp = ((binUnEqualEndLocY * ((*itrCell)->originalLocY - binEqualStartLocY)) +
                               (binUnEqualStartLocY * (binEqualEndLocY - (*itrCell)->originalLocY))) /
                              (binEqualEndLocY - binEqualStartLocY);

        if( targetLocYTemp < (*itrCell)->originalLocY )
          {
           (*itrCell)->targetLocY = (*itrCell)->originalLocY - (ay * fabs( (targetLocYTemp - (*itrCell)->originalLocY)));
          }
        else if( targetLocYTemp > (*itrCell)->originalLocY )
          {
           (*itrCell)->targetLocY = (*itrCell)->originalLocY + (ay * fabs( (targetLocYTemp - (*itrCell)->originalLocY)));
          }
        else
          {
           (*itrCell)->targetLocY = (*itrCell)->originalLocY;
          }
       }
      
    }
}

// Pseudo pin is on top chip boundry
void Bin::spCase1(NetList* netlistPtr, CellInfo* cellPtr, double *pX, double *pY, double *pDx, double *pDy)
{
        *pY = netlistPtr->Y;
        *pX = (( (cellPtr->targetLocX - cellPtr->originalLocX) /
                ( cellPtr->targetLocY - cellPtr->originalLocY)) *
              (netlistPtr->Y - cellPtr->targetLocY)) + cellPtr->targetLocX;

        *pDx = ( (cellPtr->targetLocX - cellPtr->originalLocX) /
                (cellPtr->targetLocY - cellPtr->originalLocY)) *
              (netlistPtr->Y - cellPtr->targetLocY);

        *pDy = netlistPtr->Y - cellPtr->targetLocY;

}

// Pseudo pin is on right chip boundry
void Bin::spCase2(NetList* netlistPtr, CellInfo* cellPtr, double *pX, double *pY, double *pDx, double *pDy)
{
        *pX = netlistPtr->X;
        *pY = (( ( cellPtr->targetLocY - cellPtr->originalLocY) /
                ( cellPtr->targetLocX - cellPtr->originalLocX)) *
              (netlistPtr->X - cellPtr->targetLocX)) + cellPtr->targetLocY;

        *pDy = ( (cellPtr->targetLocY - cellPtr->originalLocY) /
                (cellPtr->targetLocX - cellPtr->originalLocX)) *
              (netlistPtr->X - cellPtr->targetLocX);

        *pDx = netlistPtr->X - cellPtr->targetLocX;

}

// Pseudo pin is on bottom chip boundry
void Bin::spCase3(NetList* netlistPtr, CellInfo* cellPtr, double *pX, double *pY, double *pDx, double *pDy)
{
       *pY = 0;
       *pX = (( ( cellPtr->targetLocX - cellPtr->originalLocX) /
                ( cellPtr->targetLocY - cellPtr->originalLocY)) *
              (*pY - cellPtr->targetLocY)) + cellPtr->targetLocX;

        *pDx = ( (cellPtr->targetLocX - cellPtr->originalLocX) /
                (cellPtr->targetLocY - cellPtr->originalLocY)) *
              (*pY - cellPtr->targetLocY);

        *pDy = *pY - cellPtr->targetLocY;
}

// Pseudo pin is on left chip boundry
void Bin::spCase4(NetList* netlistPtr, CellInfo* cellPtr, double *pX, double *pY, double *pDx, double *pDy)
{
        *pX = 0;
        *pY = (( ( cellPtr->targetLocY - cellPtr->originalLocY) /
                ( cellPtr->targetLocX - cellPtr->originalLocX)) *
              (*pX - cellPtr->targetLocX)) + cellPtr->targetLocY;

        *pDy = ( (cellPtr->targetLocY - cellPtr->originalLocY) /
                (cellPtr->targetLocX - cellPtr->originalLocX)) *
              (*pX - cellPtr->targetLocX);

        *pDx = *pX - cellPtr->targetLocX;
}

void Bin::findResultantForce(NetList* netlistPtr, CellInfo* cellInfoPtr, AnalyticForm* aformPtr,
                            double* pFx, double* pFy, double* x_lastSolverOutput,double* y_lastSolverOutput)
{
 for(int n=0; n < aformPtr->n; n++)
    {
     if( (aformPtr->A[cellInfoPtr->cellId][n] != 0) && (cellInfoPtr->cellId != n)) 
       {
         *pFx = (*pFx) + (x_lastSolverOutput[n] - cellInfoPtr->targetLocX);
         *pFy = (*pFy) + (y_lastSolverOutput[n] - cellInfoPtr->targetLocY);
       }

    }

}

// Method: processPseudoPins()
//   Info: Process each cell in this bin to add spreading forces.
void Bin::processPseudoPins(NetList* netlistPtr, AnalyticForm* aformPtr, double* x_lastSolverOutput,double* y_lastSolverOutput)
{
 vector<CellInfo*>::iterator itrCell;

 // For each cell calculate pseudo nets and spring constant.
 for(itrCell= cellsInBin.begin(); itrCell != cellsInBin.end(); itrCell++)
    {
     double pFx = 0.0;
     double pFy = 0.0;
     double pX = 0.0;
     double pY = 0.0;
     double pDx = 0.0;
     double pDy = 0.0;
     double springConstant = 0;

     // Find resultant and spreading force.
     findResultantForce(netlistPtr,(*itrCell), aformPtr, &pFx, &pFy, x_lastSolverOutput, y_lastSolverOutput);

     // skip if cell is not shifted.
     if( (((*itrCell)->targetLocX - (*itrCell)->originalLocX) == 0)  && 
         (((*itrCell)->targetLocY - (*itrCell)->originalLocY) == 0) 
       )
       {
        continue;
       }
 
     if( ((*itrCell)->targetLocY > (*itrCell)->originalLocY) 
         &&
         (
          ((*itrCell)->targetLocX > (*itrCell)->originalLocX) || 
          (((*itrCell)->targetLocX - (*itrCell)->originalLocX) == 0)
         )
       )
       {
        // Case 1 Pseudo pin is on top chip boundry
        pY = netlistPtr->Y;
        pX = (( ( (*itrCell)->targetLocX - (*itrCell)->originalLocX) /
                ( (*itrCell)->targetLocY - (*itrCell)->originalLocY)) *
              (netlistPtr->Y - (*itrCell)->targetLocY)) + (*itrCell)->targetLocX;
       
        if( pX >  netlistPtr->X )
          {
           spCase2(netlistPtr, (*itrCell), &pX, &pY, &pDx, &pDy);
          }
        else
          {

           pDx = ( ((*itrCell)->targetLocX - (*itrCell)->originalLocX) /
                ((*itrCell)->targetLocY - (*itrCell)->originalLocY)) *
              (netlistPtr->Y - (*itrCell)->targetLocY);

           pDy = netlistPtr->Y - (*itrCell)->targetLocY;
          }
       }
      else if( ((*itrCell)->targetLocY < (*itrCell)->originalLocY) 
               &&
               (
               ((*itrCell)->targetLocX > (*itrCell)->originalLocX) || 
                (((*itrCell)->targetLocX - (*itrCell)->originalLocX) == 0)
               )
             )
       {
        // Case 2 Pseudo pin is on right chip boundry
        pX = netlistPtr->X;

        if( ((*itrCell)->targetLocX - (*itrCell)->originalLocX) != 0)
          {
           pY = (( ( (*itrCell)->targetLocY - (*itrCell)->originalLocY) /
                  ( (*itrCell)->targetLocX - (*itrCell)->originalLocX)) *
                (netlistPtr->X - (*itrCell)->targetLocX)) + (*itrCell)->targetLocY;
          }
        else
          {
           pY = -1;
          }

        if( pY < 0 )
          {           
          spCase3(netlistPtr, (*itrCell), &pX, &pY, &pDx, &pDy);
          }
        else
          {

        pDy = ( ((*itrCell)->targetLocY - (*itrCell)->originalLocY) /
                ((*itrCell)->targetLocX - (*itrCell)->originalLocX)) *
              (netlistPtr->X - (*itrCell)->targetLocX);

        pDx = netlistPtr->X - (*itrCell)->targetLocX; 
        }
       }
      else if( ((*itrCell)->targetLocX < (*itrCell)->originalLocX) 
               &&
               (
                ((*itrCell)->targetLocY < (*itrCell)->originalLocY) || 
                (((*itrCell)->targetLocY - (*itrCell)->originalLocY) == 0)
               )
             )
       {
        // Case 3 Pseudo pin is on bottom chip boundry
        pY = 0;
        
        if( ((*itrCell)->targetLocY - (*itrCell)->originalLocY)  != 0)
          {
           pX = (( ( (*itrCell)->targetLocX - (*itrCell)->originalLocX) /
                 ( (*itrCell)->targetLocY - (*itrCell)->originalLocY)) *
                 (pY - (*itrCell)->targetLocY)) + (*itrCell)->targetLocX;
          }
        else
          {
           pX = -1;
          }

        if( pX < 0 )
          { 
           spCase4(netlistPtr, (*itrCell), &pX, &pY, &pDx, &pDy);
          }
        else
          {
        pDx = ( ((*itrCell)->targetLocX - (*itrCell)->originalLocX) /
                ((*itrCell)->targetLocY - (*itrCell)->originalLocY)) *
              (pY - (*itrCell)->targetLocY);

        pDy = pY - (*itrCell)->targetLocY;
        }
       }
      else if( ((*itrCell)->targetLocX < (*itrCell)->originalLocX) 
               &&
               (
                ((*itrCell)->targetLocY > (*itrCell)->originalLocY) ||
                (((*itrCell)->targetLocY - (*itrCell)->originalLocY) == 0)
               )
             )
       {
        // case 4 Pseudo pin is on left chip boundry
        pX = 0;
        pY = (( ( (*itrCell)->targetLocY - (*itrCell)->originalLocY) / ( (*itrCell)->targetLocX - (*itrCell)->originalLocX)) * (pX - (*itrCell)->targetLocX)) + (*itrCell)->targetLocY;

        if( pY > netlistPtr->Y )
          {
           spCase1(netlistPtr, (*itrCell), &pX, &pY, &pDx, &pDy);
          }
        else
          {
        pDy = ( ((*itrCell)->targetLocY - (*itrCell)->originalLocY) /
                ((*itrCell)->targetLocX - (*itrCell)->originalLocX)) *
              (pX - (*itrCell)->targetLocX);

        pDx = pX - (*itrCell)->targetLocX;
        }
       }
     else if ( ((*itrCell)->targetLocX > (*itrCell)->originalLocX) && 
               (((*itrCell)->targetLocY - (*itrCell)->originalLocY) == 0) 
             )
       {
        spCase2(netlistPtr, (*itrCell), &pX, &pY, &pDx, &pDy);
       }
     else
      {
       cout <<" Error: pseudo pin locations not found.\n" <<endl;
       exit(0);
      }

      
     // Compute spring constant value.
      springConstant = (sqrt( ((pFx*pFx)+(pFy*pFy))) / 
                             sqrt( ((pDx*pDx)+(pDy*pDy))));

     // Update matrix Q diagonal, and dX dY vectors.
     int n = (*itrCell)->cellId;
     aformPtr->A[n][n] += ( 2 * springConstant );
     aformPtr->bx[n]   += ( (2 * springConstant) * pX);
     aformPtr->by[n]   += ( (2 * springConstant) * pY);
     //
     (*itrCell)->pX = pX;
     (*itrCell)->pY = pY;
    } //end for
}

// Method: clearCellsInBin()
//   Info: Remove all cells from the bin, and clear unEqual boundry.
void Bin::clearCellsInBin(void)
{
 // Clear variables for next iteration.
 binUtilization = 0;
 binUnEqualStartLocX = binEqualStartLocX;
 binUnEqualStartLocY = binEqualStartLocY;
 binUnEqualEndLocX = binEqualEndLocX;
 binUnEqualEndLocY = binEqualEndLocY;

 // Iterator for cells in this bin.
 vector<CellInfo*>::iterator itrCell;

 for(itrCell= cellsInBin.begin(); itrCell != cellsInBin.end(); itrCell++)
    {
     delete (*itrCell);
    }

 cellsInBin.resize(0);
}

// Method: print()
//   Info: prints bins size and coordinates.
void Bin::print()
{
 //cout <<"Bin Info:" <<endl;
 cout <<"   equalStartX,Y:" <<binEqualStartLocX <<"," <<binEqualStartLocY 
      <<"   equalEndX,Y:" <<binEqualEndLocX <<"," <<binEqualEndLocY <<endl;
 cout <<" unEqualStartX,Y:" <<binUnEqualStartLocX <<"," <<binUnEqualStartLocY
      <<" unEqualEndX,Y:"   <<binUnEqualEndLocX <<"," <<binUnEqualEndLocY;
 cout <<" binArea:" <<binArea <<" binUtilization:" <<binUtilization <<endl;

 vector<CellInfo*>::iterator it;
 cout <<"  CellsInBin: ";
 for(it = cellsInBin.begin(); it != cellsInBin.end(); it++)
    {
     cout << (*it)->cellId << " ";
    }
 cout <<endl;
}

Bin::~Bin(void)
{

}

