#ifndef FIXEDBLOCK_H_
#define FIXEDBLOCK_H_

#include "Block.h"

class FixedBlock
{
public:
	std::string name;

	int num;

	int x, y;

	FixedBlock();

	virtual ~FixedBlock();

    void reset();

    friend ifstream& operator>> (ifstream& is, FixedBlock& fb);
};

#endif /*FIXEDBLOCK_H_*/
