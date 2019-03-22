#pragma once

#include <iostream>
#include <string>
#include <stdint.h>
#include <cassert>
#include <string.h>
#include "common.h"
#include "hashTable.h"

static size_t MIN_LOOKAHEAD = MAX_MATCH + 1;
static size_t MAX_DIST = WSIZE - MIN_LOOKAHEAD;

class LZ77
{
public:
	LZ77();
	~LZ77();
	void fileCompress(const std::string & filePath);
	void UnFileCompress(const std::string& filePath);
	void unFileCompress(const std::string& filePath);

private:
	UCH LongestMatch(USH hashHead, USH& matchStart); 
	void  writeMark(FILE*& fOut, UCH& mark, bool map, int& bitCount);
	void getLine(FILE* PF, std::string & str);
	void fillWindow(FILE* fIn);
	
private:
	USH _start;
	UCH* _pwin;
	HashTable _ht;
	size_t _lookAhead;
};