#ifndef BLOCK_H_
#define BLOCK_H_

#include "common.h"

class end_of_section:public std::exception{};

class Block
{
public:

	std::string name;

	int num;

	vector<int> nets;

	bool bFixed;

	int x, y;

	Block();

	virtual ~Block();

	void ReadBlock(ifstream &is);

	void SetFixed(int x, int y);

	void print(bool bPrintNets);

    bool isValid();

    void reset();

    friend ifstream& operator>> (ifstream& is, Block& b);
};

#endif /*BLOCK_H_*/
