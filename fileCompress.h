#pragma once
#include <algorithm>
#include <string.h>
#include "huffmanTree.hpp"


struct charInfo
{
public:
	unsigned char _ch;
	size_t _charCount;
	std::string _strCode;

	charInfo(int count = 0)
		:_charCount(count)
		, _strCode("")
	{}

public:
	bool operator>(const charInfo& right) const
	{
		return (*this)._charCount > right._charCount;
	}

	charInfo/* 这里不能返回引用 */ operator+(const charInfo& right) const
	{
		return (*this)._charCount + right._charCount; //构建一个无名对象，返回 不能返回栈空间上对象的引用
	}

	charInfo& operator=(charInfo& right)
	{
		if (this != &right)
		{
			(*this)._ch = right._ch;
			(*this)._charCount = right._charCount;
			(*this)._strCode = right._strCode;
		}

		return *this;
	}

	bool operator==(const charInfo& right) const
	{
		return (*this)._charCount == right._charCount;
	}

	bool operator!=(const charInfo& right) const
	{
		return (*this)._charCount != right._charCount;
	}



};


class fileCompress
{
public:
	typedef huffmanTreeNode<charInfo>* PHTN;
	typedef huffmanTreeNode<charInfo>  HTN;
public:
	void FileCompress(const std::string fileName);
	void UnFileCompress(const std::string fileName);


private:
	void writeHuffmanCode(PHTN pRoot);
	void getLine(FILE* PF, std::string & str);
private:
	std::vector<charInfo> _v_charset;
};

