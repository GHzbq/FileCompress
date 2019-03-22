#pragma once
#include <string.h>

#include "common.h"


// ��ϣͰ�ĸ���Ϊ2^15
const USH HASH_BITS = 15;
// ��ϣ��Ĵ�С
const USH HASH_SIZE = (1 << HASH_BITS);
// ��ϣ����
const USH HASH_MASK = HASH_SIZE - 1;


class HashTable
{
public:
	HashTable(size_t size);
	~HashTable();
	void InsertString(USH& hashAddr, UCH ch, USH pos, USH& macthHead);
	void HashFunc(USH& HashAddr, UCH ch);

	USH GetNext(USH matchPos);

	void update();

private:
	
	USH H_SHIFT();

public:
	USH*   _prev;
	USH*   _head;
	size_t _size;
};