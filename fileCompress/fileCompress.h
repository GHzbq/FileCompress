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

	charInfo/* ���ﲻ�ܷ������� */ operator+(const charInfo& right) const
	{
		return (*this)._charCount + right._charCount; //����һ���������󣬷��� ���ܷ���ջ�ռ��϶��������
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

