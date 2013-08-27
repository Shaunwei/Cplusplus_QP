#include "Block.h"

Block::Block()
{
}

Block::~Block()
{
}

void Block::print(bool bPrintNets)
{
	if(!isValid())
		return;

	output << "  Name: " << setw(20) << name << "\tNum: " << setw(4) << num <<
		"\tFixed: " << bFixed;
	if(bFixed)
		output << "\tx: " << x << "\ty: "<< y;

	if(!bPrintNets){
	    output << endl;
		return;
	}

	output << "\tNets (" << nets.size() << "):\t";
    vector<int>::iterator it;
    for(it = nets.begin(); it != nets.end(); it++)
    {
        output << *it << " ";
    }
    output << endl;
}

bool Block::isValid()
{
    return name.size() != 0;
}

void Block::reset()
{
	bFixed = false;
    name = "";
    num = -2;
	nets.clear();
}

void Block::SetFixed(int x, int y)
{
	bFixed = true;
	this->x = x;
	this->y = y;
}

ifstream& operator>> (ifstream& is, Block& b)
{
	b.bFixed = false;
	is >> b.name;
    if(b.name == "-1")
        throw end_of_section();
    is >> b.num;

	int net;
    while(true)
    {
        is >> net;
        if (net != -1)
            b.nets.push_back(net);
        else
            break;
    }

	return is;
}
