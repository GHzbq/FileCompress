#include "hashTable.h"

HashTable::HashTable(size_t size)
	:_prev(new USH[size * 2])
	, _head(_prev + size)
	, _size(size * 2)
{
	memset(_head, 0x00, size * 2);
}

HashTable::~HashTable()
{
	if (_prev)
	{
		delete[] _prev;
		_prev = nullptr;
		_head = nullptr;
	}
}

void HashTable::HashFunc(USH& hashAddr, UCH ch)
{
	hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch)) & HASH_MASK;
}



USH HashTable::H_SHIFT()
{
	return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;
}

// hashAddr：上一次哈希地址 ch：先行缓冲区第一个字符 
// pos：ch在滑动窗口中的位置 matchHead：如果匹配，保存匹配串的起始位置
void HashTable::InsertString(USH& hashAddr, UCH ch, USH pos, USH& macthHead)
{
	// 计算哈希地址
	HashFunc(hashAddr, ch);
	// 随着压缩的不断进行，pos肯定会大于WSIZE，与上WMASK保证不越界
	_prev[pos & WMASK] = _head[hashAddr];
	macthHead = _head[hashAddr];
	_head[hashAddr] = pos;
}

USH HashTable::GetNext(USH matchPos)
{
	return _prev[matchPos];
}

void HashTable::update()
{
	// 更新head数组
	for (size_t i = 0; i < HASH_SIZE; ++i)
	{
		if (_head[i] >= WSIZE)
		{
			_head[i] -= WSIZE;
		}
		else
		{
			_head[i] = 0;
		}
	}

	// 更新prev数组
	for (size_t i = 0; i < WSIZE; ++i)
	{
		if (_prev[i] >= WSIZE)
		{
			_prev[i] -= WSIZE;
		}
		else
		{
			_prev[i] = 0;
		}
	}
}


