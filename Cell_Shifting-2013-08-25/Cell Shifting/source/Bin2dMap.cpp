#include <math.h>
#include "Bin2dMap.h"
#include "AnalyticForm.h"
#include "UmfpackHelper.h"
#include "NetList.h"
#include "Bin.h"
#include "graphics.h"

// Drawing functions.
static void button_press (float x, float y);
static void drawscreen (void);
static void new_button_func (void (*drawscreen_ptr) (void));

Bin2dMap *bin2dMapPtr = NULL;

void setBinMapPtr(Bin2dMap *bp)
{
 bin2dMapPtr = bp;
}


/////////////////////////////////////
// Class Bin2dMap Implementation.
/////////////////////////////////////

// Method: Bin2dMap()
//   Info: Constructor initialize values to zero. Number of row/column of bin are given.
Bin2dMap::Bin2dMap(NetList* netlist, AnalyticForm* aform, UmfpackHelper* umfpack,
                   const int binsRowCount, const int binsColCount, const int itrCount)
{
 this->binsRowCount = binsRowCount;
 this->binsColCount = binsColCount;
 netlistPtr = netlist;
 aformPtr = aform;
 umfpackPtr = umfpack;
 binsXdelta = (netlist->X) / binsColCount;
 binsYdelta = (netlist->Y) / binsRowCount;

 wireLengthTotal = 0.0;
 maxBinUtilization = 0.0;
 currentIteration = 0;
 iterationCount = itrCount;

 // Save original values of matrix, for next iterations.
 saveOriginalMatrixValues();

 // Copy initial solver output.
 x_lastSolverOutput = new double[aformPtr->m];
 y_lastSolverOutput = new double[aformPtr->m];

 saveLastSolverOutput(umfpackPtr);

}

// Method: saveOriginalMatrixValues()
//   Info: Save original values of matrix A, bx, by. 
void Bin2dMap::saveOriginalMatrixValues(void)
{
 // Create space for original matrix.
 A_orig = new double*[aformPtr->m];
 
 for(int i=0; i<aformPtr->m; i++)
    {
     A_orig[i] = new double[aformPtr->n];
    }

 bx_orig = new double[aformPtr->m];
 by_orig = new double[aformPtr->m];

 // Copy original matrix values.
 for(int i=0; i<aformPtr->m; i++)
    {
     for(int j=0; j<aformPtr->n; j++)
        {
         A_orig[i][j] = aformPtr->A[i][j];
        }
      bx_orig[i] = aformPtr->bx[i];
      by_orig[i] = aformPtr->by[i];
     }
}

// Method: savelastSoverOutput()
//   Info: save solver ouput for next iteration.
void Bin2dMap::saveLastSolverOutput(UmfpackHelper *umfLast)
{
 for (int i = 0 ; i < aformPtr->m ; i++)
     {
      x_lastSolverOutput[i] = umfLast->getCellLocationX(i);
      y_lastSolverOutput[i] = umfLast->getCellLocationY(i); 
     }
}

// Method: restoreOriginalMatrixValues()
//   Info: copy original values of matrix A, bx, by for next iteration.
void Bin2dMap::restoreOrigMatrixValues(void)
{
for(int i=0; i<aformPtr->m; i++)
    {
     for(int j=0; j<aformPtr->n; j++)
        {
         aformPtr->A[i][j] = A_orig[i][j];
        }
      aformPtr->bx[i] = bx_orig[i];
      aformPtr->by[i] = by_orig[i];
     }
}

// Method: doCellShifting()
//   Info: Perform cell shifting.
void Bin2dMap::doCellShifting(void)
{
 // Initialize graphics.
 initGraphics();

 // Create initial even bins.
 createRegularBins();

 for( currentIteration = 0; currentIteration < iterationCount; currentIteration++ )
    {
     // Using cell location, add cells to corresponding bin.
     addCellsToBins();
     
     // Find max bin utilization.
     findMaxUtilization();
     
     // Find total wire length.
     computeWireLengthHPWL();

     // Print info
     cout<<"\nIteration #" <<currentIteration <<endl;
     cout <<"Total WireLength: " <<wireLengthTotal
          << "  Max_Bin_tilization: " <<maxBinUtilization <<endl;

     // Disable movement control after few iterations.
     if( currentIteration == 3 )
       {
        Bin::movControlEnable = false; 
       }

     // Create unEqual bins rows.
     createUnEqualBinsRows();

     // Create unEqual bins columns.
     createUnEqualBinsColumns();

     // Add pseudo pins and spring constant in matrix Q. 
     addSpreadingForces();

     init_world (0,0,netlistPtr->X+4.5,netlistPtr->Y+4.5);
     drawscreen();
     event_loop(button_press, drawscreen);

     // Call the solver.
     UmfpackHelper umfpack(aformPtr->m, aformPtr->n, aformPtr->A, aformPtr->bx, aformPtr->by);
     umfpack.print();
 
     // Reset Bin info for next iteration. 
     resetBinsInfo();
     //restoreOrigMatrixValues();
     saveLastSolverOutput(&umfpack);
    }
}

// Method: createRegularBins()
//   Info: Create initial regular even bins.
void Bin2dMap::createRegularBins(void)
{
  // Holds the current x,y locations where new bin is added.
  int currentXoffset = 0, currentYoffset = 0;

  // Create initial empty rows of bins.
  binsRowColVec.resize(binsRowCount);

  // Bins iterator.
  vector< vector<Bin*> >::iterator itrRows;

  // Traverse each row, and add new bins upto the number of column.
  for( itrRows = binsRowColVec.begin(); itrRows != binsRowColVec.end(); itrRows++ )
     {
      for( int j=0; j < binsColCount; j++)
         {
          // Create a new bin.
          Bin *bp = new Bin();
          itrRows->push_back( bp );

          // Set initial equal bin x,y locations.
          bp->setEqualBinLocations(currentXoffset, currentYoffset, currentXoffset + binsXdelta, 
                                   currentYoffset + binsYdelta);

          // Update X offset for next bin.
          currentXoffset += binsXdelta;
         }

      // Finished with 1 row. Reset current X offset to 0, and update Y offset.
      currentXoffset = 0;
      currentYoffset += binsYdelta;
     }
}

// Method: addCellsToBins()
//   Info: Using cells location, add them to appropriate bin.
void Bin2dMap::addCellsToBins(void)
{
 // Get the count for movable cells.
 int cellCount = netlistPtr->blocksCount - netlistPtr->fixedBlocksCount;

 double locX, locY;
 int rowIndex, colIndex;

 // Loop through all the cells locations computed by the solver.
 for(int i=0; i<cellCount; i++)
    {
     // Find the index of the target bin where this cell belongs too.
     locX = x_lastSolverOutput[i];
     locY = y_lastSolverOutput[i]; 

     colIndex = ((int)locX) / binsXdelta;
     rowIndex = ((int)locY) / binsYdelta;
     
     // Insert cell to the bin.
     binsRowColVec[rowIndex][colIndex]->cellsInBin.push_back( new CellInfo(i,locX,locY) );

     // Update bin utilization
     binsRowColVec[rowIndex][colIndex]->findBinUtilization();
    }
}

// Method: createUnEqualBinsRows()
//   Info: Create unEqual bins structure for each Row.
void Bin2dMap::createUnEqualBinsRows()
{
 // Bins iterator.
 vector< vector<Bin*> >::iterator itrRows;

 // Traverse each row, and find unEqual bin boundries in x-direction.  
 for( itrRows = binsRowColVec.begin(); itrRows != binsRowColVec.end(); itrRows++ )
    {
     int columCount = (*itrRows).size();

     // Traverse each bin in this row.
     for(int m=0; m < columCount-1; m++)
        {
         // Get reference to current bin and the next bin.
         Bin *currentBinPtr = (*itrRows)[m];
         Bin    *nextBinPtr = (*itrRows)[m+1];

         // Calculate unequal bin boundry in X-direction for current bin.
         currentBinPtr->calculateUnequalBoundryX(nextBinPtr);
        }
    }

 // find movement control parameters ax and ay.
 double ax = (0.02 + (0.5 / (maxBinUtilization/2)));
  
 // For each row, linearly shift cells in x-direction.
 for( itrRows = binsRowColVec.begin(); itrRows != binsRowColVec.end(); itrRows++ )
    {
     // Column iterator.
     vector<Bin*>::iterator itrCols;

     // Traverse each bin in this row.
      for( itrCols = (*itrRows).begin(); itrCols != (*itrRows).end(); itrCols++) 
        {
         // shift cells for this bin.
         (*itrCols)->linearlyShiftCellX(ax); 
        }
    }
}

// Method: createUnEqualBinsColumns()
//   Info: Create unEqual bins structure for each Column.
void Bin2dMap::createUnEqualBinsColumns()
{
 // Traverse each column, and find unEqual bin boundries in y-direction.
 for(int m = 0; m < binsColCount; m++)
    {
     // Traverse each bin in this column.
     for(int n = 0; n < binsRowCount-1; n++)
        {
         // Get reference to current bin and the next bin.
         Bin *currentBinPtr = binsRowColVec[n][m];
         Bin    *nextBinPtr = binsRowColVec[n+1][m];

         // Calculate unequal bin boundry in Y-direction for current bin.
         currentBinPtr->calculateUnequalBoundryY(nextBinPtr);
        } 
    }

 // find movement control parameters ax and ay.
 double ay = (0.02 + (0.5 / (maxBinUtilization/2)));

 // For each column, linearly shift cells in y-direction.
 for(int m = 0; m < binsColCount; m++)
    {
     // Traverse each bin in this column.
     for(int n = 0; n < binsRowCount; n++)
        {
         binsRowColVec[n][m]->linearlyShiftCellY(ay);
        }
    }
}

// Method: addSpreadingForces()
//   Info: Add pseudo pins and spring constant in matrix Q.
void Bin2dMap::addSpreadingForces(void)
{
 // Bins iterator.
 vector< vector<Bin*> >::iterator itrRows;
 
 // Traverse each bin's cells to process original and target cell position.
 for( itrRows = binsRowColVec.begin(); itrRows != binsRowColVec.end(); itrRows++ )
    {
     // Column iterator.
     vector<Bin*>::iterator itrCols;

     // Traverse each bin in this row.
     for( itrCols = (*itrRows).begin(); itrCols != (*itrRows).end(); itrCols++)
        {
         // add pseudo pins for cells in this bin.
         (*itrCols)->processPseudoPins(netlistPtr,aformPtr, x_lastSolverOutput, y_lastSolverOutput);
        }
    }
}

// Method: resetBinsInfo()
//   Info: Reset movable cells vector in each bin.
void Bin2dMap::resetBinsInfo(void)
{
 // Bins iterator.
 vector< vector<Bin*> >::iterator itrRows;

 // Traverse each row. 
 for( itrRows = binsRowColVec.begin(); itrRows != binsRowColVec.end(); itrRows++ )
    {
     // Column iterator.
     vector<Bin*>::iterator itrCols;

     // Traverse each bin in this row.
     for( itrCols = (*itrRows).begin(); itrCols != (*itrRows).end(); itrCols++)
        {
         // Clear cells in this bin.
         (*itrCols)->clearCellsInBin();
        }
    } 
}

// Method: computeWireLengthHPWL()
//   Info: Find the total wire length for currenet placement
//         using half perimeter wire length model.
void Bin2dMap::computeWireLengthHPWL(void)
{
 double xLoc, yLoc;
 int xyIndex = 0; 
 double xMin = 10000, yMin = 10000; 
 double xMax = 0, yMax = 0;
 int fixCellOffset = netlistPtr->fixedBlocksCount + 1;

 vector <Net>::iterator itrNets;

 wireLengthTotal = 0;

 // Traverse all the nets. 
 for( itrNets = netlistPtr->nets.nets.begin(); 
      itrNets != netlistPtr->nets.nets.end(); 
      itrNets++ 
    ) 
    {
     if(itrNets->isValid() )
       {
        vector<Block>::iterator itrBlock;

        // find x,y loc for all blocks connected to this net.
        for( itrBlock =  (*itrNets).blocks.begin(); 
           itrBlock != (*itrNets).blocks.end(); 
           itrBlock++
           )
           {
            // fixed block info is in the netlist class. 
            if( itrBlock->bFixed )
              {
               xLoc = itrBlock->x;
               yLoc = itrBlock->y;
              }
            else
              {
               xyIndex = itrBlock->num - fixCellOffset;
               xLoc = x_lastSolverOutput[xyIndex];
               yLoc = y_lastSolverOutput[xyIndex];
              }

            // update the min and max location for this net. 
            if( xLoc < xMin )
              { 
               xMin = xLoc;
              }
            if( yLoc < yMin )
              {
               yMin = yLoc;
              }
            if( xLoc > xMax )
              {
               xMax = xLoc;
              }
            if( yLoc > yMax )
              {
               yMax = yLoc;
              }
            
             wireLengthTotal += ( (xMax - xMin) + (yMax - yMin) );
           } //inner for
       }
    }
}

// Method: findMaxUtilization()
//   Info: Find the total wire length for currenet placement
void Bin2dMap::findMaxUtilization(void)
{
 maxBinUtilization = 0.0;

 // Bins iterator.
 vector< vector<Bin*> >::iterator itrRows;

 // Traverse each bin to find max utilization.
 for( itrRows = binsRowColVec.begin(); itrRows != binsRowColVec.end(); itrRows++ )
    {
     // Column iterator.
     vector<Bin*>::iterator itrCols;

     // Traverse each bin in this row.
     for( itrCols = (*itrRows).begin(); itrCols != (*itrRows).end(); itrCols++)
        {
         if( (*itrCols)->binUtilization > maxBinUtilization )
           {
            maxBinUtilization = (*itrCols)->binUtilization;
           }
        }
    }
}

// Method: ~Bin2dMap()
//   Info: Destructor to delete all bins.
Bin2dMap::~Bin2dMap(void)
{
 // Bins iterator.
 vector< vector<Bin*> >::iterator itrRows;

 // Traverse each bin and delete it.
  for( itrRows = binsRowColVec.begin(); itrRows != binsRowColVec.end(); itrRows++ )
    {
     // Column iterator.
     vector<Bin*>::iterator itrCols;

     // Traverse each bin in this row.
     for( itrCols = (*itrRows).begin(); itrCols != (*itrRows).end(); itrCols++)
        {
         delete (*itrCols);
        }
    }

 // Delete original Matrix 
 for(int i=0; i<aformPtr->m; i++)
    {
     delete [] A_orig[i];
    }

 delete[] bx_orig;
 delete[] by_orig;
 delete[] A_orig;
 delete[] x_lastSolverOutput;
 delete[] y_lastSolverOutput;

 close_graphics();
}

void Bin2dMap::print(void)
{
// Bins iterator.
vector< vector<Bin*> >::iterator itrRows;

cout <<"ChipSize: " <<netlistPtr->X <<"," <<netlistPtr->Y <<" DeltaX,Y: "<< binsXdelta <<","
 <<binsYdelta <<endl;
cout <<"Row Size: " <<binsRowColVec.size() <<endl;
int ii=0;
  for( itrRows = binsRowColVec.begin(); itrRows != binsRowColVec.end(); itrRows++ )
     {
      int jj=0;
      vector<Bin*>::iterator itrCols;
      for( itrCols = (*itrRows).begin(); itrCols != (*itrRows).end(); itrCols++)
         {
          cout <<" bin[" <<ii <<"][" <<jj <<"]" <<endl;
          (*itrCols)->print();
          jj++;
         }
      ii++;
     }
}

void Bin2dMap::initGraphics(void)
{
 // Iinitialize display
 init_graphics("FastPlace Algorithm, Implementation by Mazhar Abidi");
 init_world (0,0,netlistPtr->X+4.5,netlistPtr->Y+4.5);
 create_button ("Proceed", "Show unEqual", new_button_func);
}

static void drawscreen(void)
{
/* redrawing routine for still pictures.  Graphics package calls  *
 * this routine to do redrawing after the user changes the window *
 * in any way.                                                    */

 if(bin2dMapPtr == NULL )
   {
    cout <<"drawscreen is not initialized properly. " <<endl;
    return;
   }

 clearscreen();  /* Should be first line of all drawscreens */

 setfontsize(10);
 setlinestyle(SOLID);

 char msgStr[100];
 sprintf(msgStr, "[ Iteration: %d ]  [ Wirelength: %f ]  [ MaxBinUtilization: %f ]",bin2dMapPtr->currentIteration,bin2dMapPtr->wireLengthTotal,bin2dMapPtr->maxBinUtilization);

 update_message(msgStr);

 // Draw Chip Boundry
 setcolor (BLACK);
 setlinewidth(1);
 drawrect (0.0,0.0,bin2dMapPtr->netlistPtr->X,bin2dMapPtr->netlistPtr->Y);

 // Draw equal/unEqual bin boundry.
 setlinewidth (1);
 setcolor(LIGHTGREY);
 vector< vector<Bin*> >::iterator itrRows;

 // Traverse each bin.
 for( itrRows = bin2dMapPtr->binsRowColVec.begin(); itrRows != bin2dMapPtr->binsRowColVec.end(); itrRows++ )
    {
     // Column iterator.
     vector<Bin*>::iterator itrCols;

     // Traverse each bin in this row.
     for( itrCols = (*itrRows).begin(); itrCols != (*itrRows).end(); itrCols++)
        {
         // Draw equal bins.
         setlinestyle(SOLID);
         setlinewidth (4);
         setcolor(LIGHTGREY);

         if( (*itrCols)->binEqualEndLocX !=  bin2dMapPtr->netlistPtr->X )
           {
            drawline( (*itrCols)->binEqualEndLocX, (*itrCols)->binEqualStartLocY, 
                      (*itrCols)->binEqualEndLocX, (*itrCols)->binEqualEndLocY );
           }
 
         if( (*itrCols)->binEqualEndLocY !=  bin2dMapPtr->netlistPtr->Y )
           {
            drawline( (*itrCols)->binEqualStartLocX, (*itrCols)->binEqualEndLocY,
                      (*itrCols)->binEqualEndLocX, (*itrCols)->binEqualEndLocY );
           }
        }
    }

 // Draw Fixed Cells in Red color.
 setcolor (RED);
 for(int i=0; i <= bin2dMapPtr->netlistPtr->fixedBlocksCount; i++)
    {
     int x,y;
     if( bin2dMapPtr->netlistPtr->blocks[i].isValid() )
       {
        if(bin2dMapPtr->netlistPtr->blocks[i].bFixed )
          {
           x = bin2dMapPtr->netlistPtr->blocks[i].x;
           y = bin2dMapPtr->netlistPtr->blocks[i].y;

           fillrect(x-0.08,y-0.08,x+0.08,y+0.08);
          }
       }

    }

 // Draw Moveable Cells in Green color.
 int count = bin2dMapPtr->netlistPtr->blocksCount - bin2dMapPtr->netlistPtr->fixedBlocksCount;

 setcolor (DARKGREEN);
 for(int i=0; i<count; i++)
    {
     fillrect(bin2dMapPtr->x_lastSolverOutput[i]-0.05,bin2dMapPtr->y_lastSolverOutput[i]-0.05,
              bin2dMapPtr->x_lastSolverOutput[i]+0.05,bin2dMapPtr->y_lastSolverOutput[i]+0.05);
    }

 // Draw label text
 setcolor (RED);
 drawtext (bin2dMapPtr->netlistPtr->X+2.0,2,"Fixed Cell RED",17.);
 setcolor (DARKGREEN);
 drawtext (bin2dMapPtr->netlistPtr->X+2.5,2.5,"Moveable Cell GREEN",22.);
 setcolor (LIGHTGREY);
 drawtext (bin2dMapPtr->netlistPtr->X+2.0,3,"Equal Bins GREY",20.);
 setcolor (MAGENTA);
 drawtext (bin2dMapPtr->netlistPtr->X+2.5,3.5,"unEqual Row MAGENTA",20.);
 setcolor (BLUE);
 drawtext (bin2dMapPtr->netlistPtr->X+2.3,4,"unEqual Col BLUE",20.);


}

static void button_press (float x, float y)
{
 /* Called whenever event_loop gets a button press in the graphics *
 *   * area.  Allows the user to do whatever he/she wants with button *
 *     * clicks.                                                        */

  printf("User clicked a button at coordinates (%f, %f)\n", x, y);
}

static void new_button_func (void (*drawscreen_ptr) (void)) 
{
 vector< vector<Bin*> >::iterator itrRows;

 // Traverse each bin.
 for( itrRows = bin2dMapPtr->binsRowColVec.begin(); itrRows != bin2dMapPtr->binsRowColVec.end(); itrRows++ )
    {
     // Column iterator.
     vector<Bin*>::iterator itrCols;

     // Traverse each bin in this row.
     for( itrCols = (*itrRows).begin(); itrCols != (*itrRows).end(); itrCols++)
        {
         // Draw unEqual bins.
         setlinestyle(DASHED);
         setlinewidth (1);
         setcolor(MAGENTA);

         if( (*itrCols)->binEqualEndLocX !=  bin2dMapPtr->netlistPtr->X )
           {
            drawline( (*itrCols)->binUnEqualEndLocX, (*itrCols)->binEqualStartLocY,
                      (*itrCols)->binUnEqualEndLocX, (*itrCols)->binEqualEndLocY );
           }

         if( (*itrCols)->binEqualEndLocY !=  bin2dMapPtr->netlistPtr->Y )
           {
            setcolor(BLUE);
            drawline( (*itrCols)->binEqualStartLocX, (*itrCols)->binUnEqualEndLocY,
                      (*itrCols)->binEqualEndLocX,   (*itrCols)->binUnEqualEndLocY );
           }
        }
    }
}

