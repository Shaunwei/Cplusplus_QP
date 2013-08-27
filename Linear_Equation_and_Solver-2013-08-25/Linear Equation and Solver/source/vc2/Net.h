#pragma once

#include "common.h"

#include "Block.h"

class Net
{
public:

	int num;

	vector<Block> blocks;

	Net(void);

	bool isValid();

	void print();

public:

	~Net(void);
};

class Nets
{
public:

	Nets():count(0){}

	int count;

	vector <Net> nets;

	void Add(Block& bl);

	void print();

};
