#include "fileCompress.h"
#include "LZ77.h"

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

	/*fileCompress fc;
	fc.FileCompress("croppedImage_cropped.jpg");
	fc.UnFileCompress("croppedImage_cropped.huff");*/

	std::string str = "mnoabczxyuvwabc123456abczxydefgh";
	size_t size = str.size();

	//std::string file = "test.txt\r";
	//FILE* fp = fopen(file.c_str(), "wb");
	//assert(fp);

	LZ77 lz;
	lz.fileCompress("ceshi1.txt");
	
	fileCompress fc;
	fc.FileCompress("ceshi1.lz77");
	fc.UnFileCompress("ceshi1.huff");
	lz.unFileCompress("ceshi1_testUnFC.lz77");
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