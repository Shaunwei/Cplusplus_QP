#include "FixedBlock.h"

FixedBlock::FixedBlock()
{
}

FixedBlock::~FixedBlock()
{
}

void FixedBlock::reset()
{
    name = "";
    num = -2;
	x = y = 0;
}

ifstream& operator>> (ifstream& is, FixedBlock& b)
{
	is >> b.name;
    if(b.name == "-1")
        throw end_of_section();
    is >> b.num;

	is >> b.x >> b.y;

    return is;
}
