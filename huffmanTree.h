#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <queue>



template<class T>
struct huffmanTreeNode
{
	struct huffmanTreeNode<T>* _pLeft;
	struct huffmanTreeNode<T>* _pRight;
	struct huffmanTreeNode<T>* _pParent;
	T _weight;

	huffmanTreeNode(const T& weight)
		:_pLeft(nullptr)
		, _pRight(nullptr)
		, _pParent(nullptr)
		, _weight(weight)
	{}

public:
	bool operator>(const huffmanTreeNode<T>& right) const
	{
		return (*this)._weight > right._weight;
	}
};

template <class T>
class Compare
{
public:
	bool operator()(const huffmanTreeNode<T>* left, const huffmanTreeNode<T>* right) const
	{
		return left->_weight > right->_weight;
	}
};

template<class T>
class huffmanTree
{
public:
	typedef struct huffmanTreeNode<T>  HTN;
	typedef struct huffmanTreeNode<T>* PHTN;

public:
	huffmanTree()
		:_pRoot(nullptr)
	{}

	void createHuffmanTree(const std::vector<T>& v_charset, const T& invaild = T())
	{
		if (v_charset.size() == 0)
		{
			return;
		}
		// 建立小堆
		std::priority_queue<PHTN, std::vector<PHTN>, Compare<T>> q;

		for (size_t i = 0; i < v_charset.size(); ++i)
		{
			if (v_charset[i] != invaild)
				q.push(new HTN(v_charset[i]));
		}

		// 依次从小堆取两个元素，然后建立二叉树
		PHTN pleft = nullptr;
		PHTN pright = nullptr;
		PHTN pparent = nullptr;
		PHTN newNode = nullptr;
		while (q.size()  != 1)
		{
			pleft = q.top();
			q.pop();
			pright = q.top();
			q.pop();
			newNode = new HTN(pleft->_weight + pright->_weight);
			q.push(newNode);
			newNode->_pLeft = pleft;
			pleft->_pParent = newNode;
			newNode->_pRight = pright;
			pright->_pParent = newNode;
		}
		_pRoot = q.top();
		q.pop();
	}

	~huffmanTree()
	{
		if (_pRoot)
			_Destory(_pRoot);

		_pRoot = nullptr;
	}

	PHTN getPRoot()
	{
		return _pRoot;
	}
private:
	void _Destory(PHTN proot)
	{
		if (nullptr == proot)
			return;
		_Destory(proot->_pLeft);
		_Destory(proot->_pRight);

		delete proot;
	}

private:
	PHTN _pRoot;
};
