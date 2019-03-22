
#include "LZ77.h"

LZ77::LZ77()
	:_pwin(new UCH[WSIZE * 2])
	, _ht(WSIZE)
	, _start(0)
	, _lookAhead(0)
{}

LZ77::~LZ77()
{
	if (_pwin)
	{
		delete[] _pwin;
		_pwin = nullptr;
	}
}

void LZ77::fileCompress(const std::string & filePath)
{
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (fIn == nullptr)
	{
		std::cout << "文件打开失败" << std::endl;
		return;
	}

	std::string compressFileName = filePath.substr(0, filePath.rfind('.'));;
	compressFileName += ".lz77";

	fseek(fIn, 0, SEEK_END);
	unsigned long int fileSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);

	FILE* fOut = fopen(compressFileName.c_str(), "wb");
	assert(fOut);

	// 将后缀写到文件中
	std::string sourceFileSuffix = filePath.substr(filePath.rfind('.'));
	sourceFileSuffix += '\n';
	fwrite(sourceFileSuffix.c_str(), sizeof(char), sourceFileSuffix.size(),fOut);

	unsigned long int markSize = 0;
	
	unsigned char ReadBuf[1024] = { 0 };
	size_t rdSize = 0;

	// 如果文件大小小于MIN_MATCH 按照道理是不需要压缩的，我们记录文件后缀直接返回吧
	if (fileSize <= MIN_MATCH)
	{
		rdSize = fread(ReadBuf, sizeof(unsigned char), sizeof(ReadBuf) / sizeof(ReadBuf[0]), fIn);

		// 文件大小为0 标记大小为0 记录一下，方便解压缩
		fwrite(&fileSize, sizeof(unsigned long int), 1, fOut);
		fwrite(&markSize, sizeof(unsigned long int), 1, fOut);
		std::cout << "文件大小为0，创建空文件，返回" << std::endl;
		fclose(fOut);
		fclose(fIn);
		return;
	}

	// 如果文件不空，我们再打开一个标记文件
	std::string markFileName = filePath.substr(0, filePath.rfind('.'));
	markFileName += ".mark";
	FILE* fMark = fopen(markFileName.c_str(), "wb");
	assert(fMark);

	USH hashAddr = 0;
	_start = 0;

	// 先读取一个缓冲区的数据
	rdSize = fread(_pwin, sizeof(UCH), WSIZE * 2, fIn);
	_lookAhead += rdSize;

	// 准备工作做好，我们开始压缩
	// 不超过三字节的文件我们已经处理
	// 接下来我们借助hash表来完成压缩
	// 在使用之前，我们需要手动计算，前两个字节的哈希地址
	for (size_t i = 0; i < MIN_MATCH ; ++i)
	{
		_ht.HashFunc(hashAddr, _pwin[i]);
	}

	USH matchHead = 0;
	int bitCount = 0;
	UCH mark = 0;

	while (_lookAhead)
	{
		
		_ht.InsertString(hashAddr, _pwin[_start+2], _start, matchHead);

		USH max_dist = 0;
		UCH max_str_len = 0;
		if (matchHead && _lookAhead > MIN_LOOKAHEAD)
		{
			max_str_len = LongestMatch(matchHead, max_dist);
		}
		if (max_str_len < MIN_MATCH)
		{
			// 没有匹配
			// 源字符
			// 写标记
			fputc(_pwin[_start], fOut);
			writeMark(fMark, mark, false, bitCount);
			++_start;
			--_lookAhead;
		}
		else
		{
			// 长度距离对

			writeMark(fMark, mark, true, bitCount);
			// 文件指针比较多，操作一定要注意
			fwrite(&max_str_len, sizeof(UCH), 1, fOut);
			fwrite(&max_dist, sizeof(USH), 1, fOut);
			_lookAhead -= max_str_len;

			// 更新哈希表
			while (max_str_len)
			{
				++_start;
				_ht.InsertString(hashAddr, _pwin[_start + 2], _start + 2, matchHead);
				--max_str_len;
			}
		}

		// 窗口数据不够就填充数据
		if (_lookAhead <= MIN_LOOKAHEAD)
		{
			fillWindow(fIn);
		}
	}

	// 这是对标记位最后一位的处理，如果没满8位，需要左移一定位数，并写入文件
	if (bitCount > 0 && bitCount < 8)
	{
		mark <<= (8 - bitCount);
		fputc(mark, fMark);
	}
	// 这是个坑！！！！！！以"wb"方式打开，如果fread的话，返回0
	fclose(fMark);
	FILE* f = fopen(markFileName.c_str(), "rb");
	assert(f);
	// 得到标记文件大小
	fseek(f, 0, SEEK_END);
	markSize = ftell(fMark);
	UCH* markBuf = new UCH[markSize];
	memset(markBuf, 0x00, sizeof(UCH) * markSize);
	fseek(f, 0, SEEK_SET);
	rdSize = fread(markBuf, sizeof(UCH), markSize, f);
	if (rdSize != markSize)
	{
		assert(0);
	}

	size_t wtSize = fwrite(markBuf, sizeof(UCH), markSize, fOut);
	if (wtSize != markSize)
	{
		assert(0);
	}
	delete[] markBuf;
	fwrite(&fileSize, sizeof(unsigned long int), 1, fOut);
	fwrite(&markSize, sizeof(unsigned long int), 1, fOut);

	fclose(fIn);
	fclose(fOut);
	fclose(f);
	remove(markFileName.c_str());
}
void LZ77::UnFileCompress(const std::string& filePath)
{
	std::string suffix = filePath.substr(filePath.rfind('.'));
	if (suffix != ".lz77")
	{
		std::cout << "解压缩文件格式不支持" << std::endl;
		return;
	}

	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (fIn == nullptr)
	{
		std::cout << "解压缩文件打开失败" << std::endl;
		return;
	}

	std::string sourceFileName = filePath.substr(0, filePath.rfind('.'));
	sourceFileName += "_test";
	getLine(fIn, sourceFileName);

	FILE* fOut = fopen(sourceFileName.c_str(), "wb");
	assert(fOut);

	// 文件格式：源文件后缀，压缩文件，标记文件，源文件大小，标记文件大小
	long offset = 0 - sizeof(unsigned long);
	
	// 取出标记文件大小
	fseek(fIn, offset, SEEK_END);
	unsigned long markSize = 0;
	fread(&markSize, sizeof(unsigned long), 1, fIn);

	// 取出压缩文件大小
	offset = 0 - sizeof(unsigned long)-sizeof(unsigned long);
	fseek(fIn, offset, SEEK_END);
	unsigned long fileSize = 0;
	fread(&fileSize, sizeof(unsigned long), 1, fIn);

	std::string markFileName = filePath.substr(0, filePath.rfind('.'));
	markFileName += ".mark";
	FILE* fMark = fopen(markFileName.c_str(), "wb");
	assert(fMark);

	offset = 0 - sizeof(unsigned long)-sizeof(unsigned long)-markSize;
	fseek(fIn, offset, SEEK_END);
	unsigned char * readBuf = new unsigned char[markSize];
	memset(readBuf, 0x00, sizeof(unsigned char)* markSize);
	size_t rdSize = fread(readBuf, sizeof(unsigned char), markSize, fIn);
	if (rdSize != markSize)
	{
		std::cout << "标记文件获取失败" << std::endl;
		delete[] readBuf;
		fclose(fIn);
		fclose(fOut);
		fclose(fMark);
		return;
	}
	size_t wtSize = fwrite(readBuf, sizeof(unsigned long), rdSize, fMark);
	if (wtSize != markSize)
	{
		std::cout << "写标记文件出错" << std::endl;
		delete[] readBuf;
		fclose(fIn);
		fclose(fOut);
		fclose(fMark);
		return;
	}
	delete[] readBuf;
	// 由于fMark是以"wb"方式打开，我们将其关闭，然后再以"rb"方式打开
	fclose(fMark);
	fMark = fopen(markFileName.c_str(), "rb");
	assert(fMark);
	// 调整解压缩文件指针到开始处 // 有点问题，因为首行保存的是源文件后缀
	fseek(fIn, suffix.size() , SEEK_SET);

	// 准备工作做好，我们开始解压缩
	UCH mark = 0;
	char bitCount = -1;
	USH len = 0;
	USH dist = 0;
	UCH ch = 0;

	FILE * fOutW = fopen(sourceFileName.c_str(), "rb");
	assert(fOutW);
	while (fileSize)
	{
		if (bitCount < 0)
		{
			mark = fgetc(fMark);
			bitCount = 7;
		}

		if (mark & (1 << bitCount))
		{
			// 长度距离对
			fread(&len, sizeof(UCH), 1, fIn);
			fread(&dist, sizeof(USH), 1, fIn);

			// 我们移动的文件指针应该是源文件的（fOut）
			fflush(fOut);
			
			// 这么搞也不太好，因为fOut是以"wb"方式打开的
			// 我们再打开一个文件吧
			// 移动文件指针fOutW
			offset = 0 - dist;
			fseek(fOutW, offset, SEEK_END);
			unsigned char* buf = new unsigned char[len];
			memset(buf, 0x00, sizeof(unsigned char)* len);

			rdSize = fread(buf, sizeof(unsigned char), len, fOutW);
			if (rdSize != len)
			{
				assert(0);
				delete[] buf;
				fclose(fIn);
				fclose(fOut);
				fclose(fMark);
				fclose(fOutW);
				return;
			}
			wtSize = fwrite(buf, sizeof(unsigned char), len, fOut);
			if (wtSize != len)
			{
				assert(0);
				delete[] buf;
				fclose(fIn);
				fclose(fOut);
				fclose(fMark);
				fclose(fOutW);
				return;
			}
			// 动态申请的内存，我们将其释放掉
			delete[] buf;
			fileSize -= len;
		}
		else
		{
			// 源字符
			UCH ch = fgetc(fIn);
			fputc(ch, fOut);
			fileSize -= 1;
		}
		--bitCount;
	}
}

void LZ77::unFileCompress(const std::string& filePath)
{
	std::string suffix = filePath.substr(filePath.rfind('.'));
	if (suffix != ".lz77")
	{
		std::cout << "解压缩文件格式不支持" << std::endl;
		return;
	}

	std::string sourceFileName = filePath.substr(0, filePath.rfind('.'));
	sourceFileName += "_test";
	// 打开解压缩文件
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		std::cout << "解压缩文件打开失败" << std::endl;
		return;
	}

	getLine(fIn, sourceFileName);

	// 打开源文件
	FILE* fOut = fopen(sourceFileName.c_str(), "wb");
	assert(fOut);

	

	long int offset = 0;
	// 获取标记文件大小
	offset = 0 - sizeof(unsigned long int);
	fseek(fIn, offset, SEEK_END);
	unsigned long int markSize = 0;
	fread(&markSize, sizeof(unsigned long int), 1, fIn);

	// 获取源文件大小
	offset = 0 - sizeof(unsigned long int) - sizeof(unsigned long int);
	fseek(fIn, offset, SEEK_END);
	unsigned long int fileSize = 0;
	fread(&fileSize, sizeof(unsigned long int), 1, fIn);

	// 再次打开解压缩文件，并设置偏移量到标记文件开始处
	FILE* fMark = fopen(filePath.c_str(), "rb");
	assert(fMark);
	offset = 0 - sizeof(unsigned long int) - sizeof(unsigned long int) - markSize;
	fseek(fMark, offset, SEEK_END);

	// 源文件指针复位
	offset = 5;
	fseek(fIn, offset, SEEK_SET);

	// 方便查找长度距离对的辅助文件
	FILE* fWin = fopen(sourceFileName.c_str(), "rb");
	assert(fWin);

	UCH mark = 0;
	int bitCount = -1;

	while (fileSize)
	{
		// 读取标记
		if (bitCount < 0)
		{
			mark = fgetc(fMark);
			bitCount = 7;
		}

		if (mark & (1 << bitCount))
		{
			// 1 长度距离对
			// 读取长度距离对
			UCH len = fgetc(fIn);
			USH dist = 0;
			fread(&dist, sizeof(USH), 1, fIn);

			fflush(fOut);

			offset = 0 - dist;
			fseek(fWin, offset, SEEK_END);
			fileSize -= len;

			while (len)
			{
				UCH ch = fgetc(fWin);
				fputc(ch, fOut);
				--len;
				fflush(fOut);
			}
		}
		else
		{
			// 0 源字符
			UCH ch = fgetc(fIn);
			fputc(ch, fOut);
			--fileSize;
		}
		--bitCount;
	}

	fclose(fIn);
	fclose(fMark);
	fclose(fOut);
	fclose(fWin);
}

void  LZ77::writeMark(FILE*& fOut, UCH& mark, bool map, int& bitCount)
{
	mark <<= 1;
	++bitCount;
	if (map)
		mark |= 1;

	if (bitCount == 8)
	{
		fputc(mark, fOut);
		mark = 0;
		bitCount = 0;
	}
}

//// 功能：在当前匹配链中找最长匹配
//// 参数：
//// hashHead: 匹配链的起始位置
//// matchStart：最长匹配串在滑动窗口中的起始位置
//// 返回值：最长匹配串的长度
//USH LongestMatch(USH hashHead, USH& matchStart)
//{
//	// 哈希链的最大遍历长度，防止造成死循环
//	int chain_length = 256;
//	// 始终保持滑动窗口为WSIZE，因为最小的超前查看窗口中有MIN_LOOKAHEAD的数据
//	// 因此只搜索_start左边MAX_DIST范围内的串
//	USH limit = _start > MAX_DIST ? _start - MAX_DIST : 0;
//	// 待匹配字符串的最大位置
//	// [pScan, strend]
//	UCH* pScan = _pwin + _start;
//	UCH* strend = pScan + MAX_MATCH - 1;
//	// 本次链中的最佳匹配
//	int bestLen = 0;
//	UCH* pCurMatchStart;
//	USH curMatchLen = 0;
//	// 开始匹配
//	do
//	{
//		// 从搜索区hashHead的位置开始匹配
//		pCurMatchStart = _pwin + hashHead;
//		while (pScan < strend && *pScan == *pCurMatchStart)
//		{
//			pScan++;
//			pCurMatchStart++;
//		}
//		// 本次匹配的长度和匹配的起始位置
//		curMatchLen = (MAX_MATCH - 1) - (int)(strend - pScan);
//		pScan = strend - (MAX_MATCH - 1);
//		/*更新最佳匹配的记录*/
//		if (curMatchLen > bestLen)
//		{
//			matchStart = hashHead;
//			bestLen = curMatchLen;
//		}
//	} while ((hashHead = _ht._prev[hashHead & WMASK]) > limit
//		&& --chain_length != 0);
//	return curMatchLen;
//}

// matchHead: 匹配链的起始位置
// curMatchDist: 输出型参数，带出最长匹配的距离
// 返回值：最长匹配长度
UCH  LZ77::LongestMatch(USH matchHead, USH& curMatchDist)
{	
	// 哈希链最长不能超过256，防止死循环
	int chain_length = 256;

	// 
	USH limit = _start > MAX_DIST ? _start - MAX_DIST : 0;
	
	UCH maxLen = 0;
	USH maxDist = 0;
	do
	{
		// 在查找缓冲区找到匹配串的起始位置
		UCH curMatchLen = 0;
		UCH* pStart = _pwin + _start;
		UCH* pCurStart = _pwin + matchHead;
		UCH* pCurEnd = pCurStart + MAX_MATCH;
		// 找单条链的匹配长度
		while ( pCurStart < pCurEnd &&  *pStart == *pCurStart)
		{
			++pStart;
			++pCurStart;
			++curMatchLen;
		}

		if (maxLen < curMatchLen)
		{
			maxLen = curMatchLen;
			maxDist = matchHead;
		}

	} while ((matchHead = _ht.GetNext(matchHead)) > limit && --chain_length != 0);

	curMatchDist = _start - maxDist;
	return maxLen;
}

void LZ77::getLine(FILE* PF, std::string & str)
{
	unsigned char ch = 0;
	while (!feof(PF))
	{
		ch = fgetc(PF);
		if (ch == '\n')
			return;
		else
			str += ch;
	}
}

void LZ77::fillWindow(FILE* fIn)
{
	if (_start >= WSIZE + MAX_DIST)
	{
		memcpy(_pwin, _pwin + WSIZE, WSIZE);
		memset(_pwin + WSIZE, 0x00, WSIZE);
		_start -= WSIZE;

		_ht.update();
	}

	// 向右窗口填充数据
	size_t readSize = 0;
	if (!feof(fIn))
	{
		readSize = fread(_pwin + _start + _lookAhead, 1, WSIZE, fIn);

		_lookAhead += readSize;
	}
}
