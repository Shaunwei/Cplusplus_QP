#include "NetList.h"

#include "Block.h"
#include "FixedBlock.h"

using namespace std;
/*
Example input file:
       50 50
       -1
       blk1 1 2 3 4 -1
       blk2 2 5 4 -1
       blk3 3 5 6 2 -1
       blk4 4 6 3 -1
        -1
       blk1 1 50 0
       blk4 4 0 50
       -1
*/
NetList::NetList(const char* inputfilename):blocksCount(0),fixedBlocksCount(0)
{
	ifstream input_file(inputfilename);
	if(!input_file.is_open())
        throw std::runtime_error(std::string("input file can not be opened: ") + inputfilename);

    int minusOne;

// X Y size ===========================
//  50 50
    input_file >> X >> Y;

//  -1
    input_file >> minusOne;
    if(minusOne != -1)
        throw std::runtime_error(std::string("input file not valid: ") + inputfilename);

//  Movable Blocks =====================
//  blk1 1 2 3 4 -1
	{
		Block b;
		while(true)
		{
			b.reset();
			try{
				input_file >> b;
			}catch(end_of_section){
				break;
			}
			if(blocks.size() <= b.num)
				blocks.resize(b.num*2);
			blocks[b.num] = b;
			blocksCount++;
		}
	}
//  Fixed Blocks =====================
//  blk4 4 0 50
	{
		FixedBlock fb;
		while(true)
		{
			fb.reset();
			try{
				input_file >> fb;
			}catch(end_of_section){
				break;
			}
			blocks[fb.num].SetFixed(fb.x, fb.y);
			fixedBlocksCount++;
		}
	}

	{
		vector<Block>::iterator it;
		for(it = blocks.begin(); it != blocks.end(); it++)
		{
			nets.Add(*it);
		}
	}

}

NetList::~NetList()
{
}

void NetList::print()
{
    output << "Size: " << X << "," << Y << endl;

	output << endl;
	output << "========================================== " << endl;
	output << "================= BLOCKS =================" << endl;
	output << "========================================== " << endl;

    vector<Block>::iterator itb;
    for(itb = blocks.begin(); itb != blocks.end(); itb++)
    {
        itb->print(true) ;
    }

	output << endl;
	output << "========================================== " << endl;
	output << "================== NETS ==================" << endl;
	output << "========================================== " << endl;

	nets.print();
}
