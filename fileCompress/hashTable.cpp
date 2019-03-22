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

// hashAddr����һ�ι�ϣ��ַ ch�����л�������һ���ַ� 
// pos��ch�ڻ��������е�λ�� matchHead�����ƥ�䣬����ƥ�䴮����ʼλ��
void HashTable::InsertString(USH& hashAddr, UCH ch, USH pos, USH& macthHead)
{
	// �����ϣ��ַ
	HashFunc(hashAddr, ch);
	// ����ѹ���Ĳ��Ͻ��У�pos�϶������WSIZE������WMASK��֤��Խ��
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
	// ����head����
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

	// ����prev����
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


