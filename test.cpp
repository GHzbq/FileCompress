#include "fileCompress.h"


void test()
{
	/*std::vector<charInfo> v;
	v.push_back(charInfo(1));
	v.push_back(charInfo(3));
	v.push_back(charInfo(5));
	v.push_back(charInfo(7));
	huffmanTree<charInfo> ht;
	charInfo* invaild = new charInfo(0);
	ht.createHuffmanTree(v, *invaild);*/

	fileCompress fc;
	fc.FileCompress("ceshi.txt");
	fc.UnFileCompress("ceshi.huff");
}

#include <string.h>
void test1()
{
	FILE* fIn = fopen("asdfghjkl.txt", "wb");
	if (nullptr == fIn)
	{
		std::cout << " 文件打开失败！！！！\n";
		return;
	}

	const char buf[] = "aaa\n\n\n\n\nbbbb\n\nccc";
	fwrite(buf, sizeof(char), strlen(buf), fIn);
	fclose(fIn);
}

void test2()
{
	std::string val_str = std::to_string(1234);
	std::cout << val_str << std::endl;

	int val = std::stoi(val_str);
	std::cout << val << std::endl;

	val_str = std::string("12.34");
	float va = std::stof(val_str);
	std::cout << va << std::endl;
}

int main()
{
	test();

	return 0;
}