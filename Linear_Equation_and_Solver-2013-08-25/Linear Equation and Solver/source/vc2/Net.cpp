#include "Net.h"

#include "Block.h"

Net::Net():num(-2)
{
}

void Net::print()
{
	if(!isValid())
		return;

	output << "Net Number: " << num << 
		" Blocks (" << blocks.size() << "):"<<endl;
	{
		vector<Block>::iterator it;
		for(
			it = blocks.begin();
			it != blocks.end();
		it++)
			it->print(false);
	}
}

Net::~Net()
{
}

bool Net::isValid()
{
    return num != -2;
}

void Nets::Add(Block& b)
{
	count++;
	vector<int>::iterator itBlkNets;
	for(
		itBlkNets = b.nets.begin();
		itBlkNets != b.nets.end();
		itBlkNets++)
		{
			if(nets.size() <= *itBlkNets)
				nets.resize((*itBlkNets)*2);

			nets[*itBlkNets].blocks.push_back(b);
			nets[*itBlkNets].num = *itBlkNets;
		}
}

void Nets::print()
{
	vector <Net>::iterator it;
	for(
		it = nets.begin();
		it != nets.end();
		it++)
			it->print();
}
