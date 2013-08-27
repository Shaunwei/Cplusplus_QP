#include "common.h"

#include "NetList.h"
#include "AnalyticForm.h"
#include "UmfpackHelper.h"

#include "Bin2dMap.h"

fstream output;

void mymain(int argc, char* argv[])
{
    string infile;

    if(argc != 5)
    {
        cout << "\n Usage: fastPlaceRun input_file iterationCount binRowCount binColCount \n" << endl;
    }
    else
    {
	infile = argv[1];
    }

    output.open((infile+".out").c_str(), ios_base::out);

    NetList netlist(infile.c_str());
    netlist.print();

    AnalyticForm aform(netlist);
    aform.print();

    UmfpackHelper umfpack(aform.m, aform.n, aform.A, aform.bx, aform.by);
    umfpack.print();

    // Create the bin2dMap object
    Bin2dMap bins2dMap(&netlist, &aform, &umfpack, atoi(argv[3]), atoi(argv[4]), atoi(argv[2]));

    // Init Graphic pointers.
    setBinMapPtr(&bins2dMap);

    // Perform cell shifting.
    bins2dMap.doCellShifting();
}

int main(int argc, char* argv[])
{
    string infile;
    try
    {
        mymain(argc, argv);
    }
    catch (std::runtime_error e)
    {
        output << "Error: " << e.what();
    }

    output << endl << "End." ;

    return 0;
}
