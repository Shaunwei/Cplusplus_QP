#include "common.h"

#include "NetList.h"
#include "AnalyticForm.h"
#include "UmfpackHelper.h"

fstream output;

void mymain(int argc, _TCHAR *argv[])
{
    string infile;

    if(argc != 2)
    {
        cout << "Usage: placement input_file" << endl;
        cout << "Using default input file: input.ap" << endl;
        infile = "input.ap";
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
}

int _tmain(int argc, _TCHAR* argv[])
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

