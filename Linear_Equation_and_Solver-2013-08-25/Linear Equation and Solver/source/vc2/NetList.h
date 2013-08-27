#ifndef NETLISTFILE_H_
#define NETLISTFILE_H_

#include "common.h"

#include "Net.h"

class Block;

class Net;

class NetList
{
private:
	NetList():blocksCount(0),fixedBlocksCount(0){}

public:

	float X, Y;

	vector<Block> blocks;

	int blocksCount;

	int fixedBlocksCount;

	Nets nets;

	NetList(const char* inputfile);
	
	virtual ~NetList();

	void print();
};

#endif /*NETLISTFILE_H_*/
