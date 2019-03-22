
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
		std::cout << "�ļ���ʧ��" << std::endl;
		return;
	}

	std::string compressFileName = filePath.substr(0, filePath.rfind('.'));;
	compressFileName += ".lz77";

	fseek(fIn, 0, SEEK_END);
	unsigned long int fileSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);

	FILE* fOut = fopen(compressFileName.c_str(), "wb");
	assert(fOut);

	// ����׺д���ļ���
	std::string sourceFileSuffix = filePath.substr(filePath.rfind('.'));
	sourceFileSuffix += '\n';
	fwrite(sourceFileSuffix.c_str(), sizeof(char), sourceFileSuffix.size(),fOut);

	unsigned long int markSize = 0;
	
	unsigned char ReadBuf[1024] = { 0 };
	size_t rdSize = 0;

	// ����ļ���СС��MIN_MATCH ���յ����ǲ���Ҫѹ���ģ����Ǽ�¼�ļ���׺ֱ�ӷ��ذ�
	if (fileSize <= MIN_MATCH)
	{
		rdSize = fread(ReadBuf, sizeof(unsigned char), sizeof(ReadBuf) / sizeof(ReadBuf[0]), fIn);

		// �ļ���СΪ0 ��Ǵ�СΪ0 ��¼һ�£������ѹ��
		fwrite(&fileSize, sizeof(unsigned long int), 1, fOut);
		fwrite(&markSize, sizeof(unsigned long int), 1, fOut);
		std::cout << "�ļ���СΪ0���������ļ�������" << std::endl;
		fclose(fOut);
		fclose(fIn);
		return;
	}

	// ����ļ����գ������ٴ�һ������ļ�
	std::string markFileName = filePath.substr(0, filePath.rfind('.'));
	markFileName += ".mark";
	FILE* fMark = fopen(markFileName.c_str(), "wb");
	assert(fMark);

	USH hashAddr = 0;
	_start = 0;

	// �ȶ�ȡһ��������������
	rdSize = fread(_pwin, sizeof(UCH), WSIZE * 2, fIn);
	_lookAhead += rdSize;

	// ׼���������ã����ǿ�ʼѹ��
	// ���������ֽڵ��ļ������Ѿ�����
	// ���������ǽ���hash�������ѹ��
	// ��ʹ��֮ǰ��������Ҫ�ֶ����㣬ǰ�����ֽڵĹ�ϣ��ַ
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
			// û��ƥ��
			// Դ�ַ�
			// д���
			fputc(_pwin[_start], fOut);
			writeMark(fMark, mark, false, bitCount);
			++_start;
			--_lookAhead;
		}
		else
		{
			// ���Ⱦ����

			writeMark(fMark, mark, true, bitCount);
			// �ļ�ָ��Ƚ϶࣬����һ��Ҫע��
			fwrite(&max_str_len, sizeof(UCH), 1, fOut);
			fwrite(&max_dist, sizeof(USH), 1, fOut);
			_lookAhead -= max_str_len;

			// ���¹�ϣ��
			while (max_str_len)
			{
				++_start;
				_ht.InsertString(hashAddr, _pwin[_start + 2], _start + 2, matchHead);
				--max_str_len;
			}
		}

		// �������ݲ������������
		if (_lookAhead <= MIN_LOOKAHEAD)
		{
			fillWindow(fIn);
		}
	}

	// ���ǶԱ��λ���һλ�Ĵ������û��8λ����Ҫ����һ��λ������д���ļ�
	if (bitCount > 0 && bitCount < 8)
	{
		mark <<= (8 - bitCount);
		fputc(mark, fMark);
	}
	// ���Ǹ��ӣ�������������"wb"��ʽ�򿪣����fread�Ļ�������0
	fclose(fMark);
	FILE* f = fopen(markFileName.c_str(), "rb");
	assert(f);
	// �õ�����ļ���С
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
		std::cout << "��ѹ���ļ���ʽ��֧��" << std::endl;
		return;
	}

	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (fIn == nullptr)
	{
		std::cout << "��ѹ���ļ���ʧ��" << std::endl;
		return;
	}

	std::string sourceFileName = filePath.substr(0, filePath.rfind('.'));
	sourceFileName += "_test";
	getLine(fIn, sourceFileName);

	FILE* fOut = fopen(sourceFileName.c_str(), "wb");
	assert(fOut);

	// �ļ���ʽ��Դ�ļ���׺��ѹ���ļ�������ļ���Դ�ļ���С������ļ���С
	long offset = 0 - sizeof(unsigned long);
	
	// ȡ������ļ���С
	fseek(fIn, offset, SEEK_END);
	unsigned long markSize = 0;
	fread(&markSize, sizeof(unsigned long), 1, fIn);

	// ȡ��ѹ���ļ���С
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
		std::cout << "����ļ���ȡʧ��" << std::endl;
		delete[] readBuf;
		fclose(fIn);
		fclose(fOut);
		fclose(fMark);
		return;
	}
	size_t wtSize = fwrite(readBuf, sizeof(unsigned long), rdSize, fMark);
	if (wtSize != markSize)
	{
		std::cout << "д����ļ�����" << std::endl;
		delete[] readBuf;
		fclose(fIn);
		fclose(fOut);
		fclose(fMark);
		return;
	}
	delete[] readBuf;
	// ����fMark����"wb"��ʽ�򿪣����ǽ���رգ�Ȼ������"rb"��ʽ��
	fclose(fMark);
	fMark = fopen(markFileName.c_str(), "rb");
	assert(fMark);
	// ������ѹ���ļ�ָ�뵽��ʼ�� // �е����⣬��Ϊ���б������Դ�ļ���׺
	fseek(fIn, suffix.size() , SEEK_SET);

	// ׼���������ã����ǿ�ʼ��ѹ��
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
			// ���Ⱦ����
			fread(&len, sizeof(UCH), 1, fIn);
			fread(&dist, sizeof(USH), 1, fIn);

			// �����ƶ����ļ�ָ��Ӧ����Դ�ļ��ģ�fOut��
			fflush(fOut);
			
			// ��ô��Ҳ��̫�ã���ΪfOut����"wb"��ʽ�򿪵�
			// �����ٴ�һ���ļ���
			// �ƶ��ļ�ָ��fOutW
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
			// ��̬������ڴ棬���ǽ����ͷŵ�
			delete[] buf;
			fileSize -= len;
		}
		else
		{
			// Դ�ַ�
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
		std::cout << "��ѹ���ļ���ʽ��֧��" << std::endl;
		return;
	}

	std::string sourceFileName = filePath.substr(0, filePath.rfind('.'));
	sourceFileName += "_test";
	// �򿪽�ѹ���ļ�
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		std::cout << "��ѹ���ļ���ʧ��" << std::endl;
		return;
	}

	getLine(fIn, sourceFileName);

	// ��Դ�ļ�
	FILE* fOut = fopen(sourceFileName.c_str(), "wb");
	assert(fOut);

	

	long int offset = 0;
	// ��ȡ����ļ���С
	offset = 0 - sizeof(unsigned long int);
	fseek(fIn, offset, SEEK_END);
	unsigned long int markSize = 0;
	fread(&markSize, sizeof(unsigned long int), 1, fIn);

	// ��ȡԴ�ļ���С
	offset = 0 - sizeof(unsigned long int) - sizeof(unsigned long int);
	fseek(fIn, offset, SEEK_END);
	unsigned long int fileSize = 0;
	fread(&fileSize, sizeof(unsigned long int), 1, fIn);

	// �ٴδ򿪽�ѹ���ļ���������ƫ����������ļ���ʼ��
	FILE* fMark = fopen(filePath.c_str(), "rb");
	assert(fMark);
	offset = 0 - sizeof(unsigned long int) - sizeof(unsigned long int) - markSize;
	fseek(fMark, offset, SEEK_END);

	// Դ�ļ�ָ�븴λ
	offset = 5;
	fseek(fIn, offset, SEEK_SET);

	// ������ҳ��Ⱦ���Եĸ����ļ�
	FILE* fWin = fopen(sourceFileName.c_str(), "rb");
	assert(fWin);

	UCH mark = 0;
	int bitCount = -1;

	while (fileSize)
	{
		// ��ȡ���
		if (bitCount < 0)
		{
			mark = fgetc(fMark);
			bitCount = 7;
		}

		if (mark & (1 << bitCount))
		{
			// 1 ���Ⱦ����
			// ��ȡ���Ⱦ����
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
			// 0 Դ�ַ�
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

//// ���ܣ��ڵ�ǰƥ���������ƥ��
//// ������
//// hashHead: ƥ��������ʼλ��
//// matchStart���ƥ�䴮�ڻ��������е���ʼλ��
//// ����ֵ���ƥ�䴮�ĳ���
//USH LongestMatch(USH hashHead, USH& matchStart)
//{
//	// ��ϣ�������������ȣ���ֹ�����ѭ��
//	int chain_length = 256;
//	// ʼ�ձ��ֻ�������ΪWSIZE����Ϊ��С�ĳ�ǰ�鿴��������MIN_LOOKAHEAD������
//	// ���ֻ����_start���MAX_DIST��Χ�ڵĴ�
//	USH limit = _start > MAX_DIST ? _start - MAX_DIST : 0;
//	// ��ƥ���ַ��������λ��
//	// [pScan, strend]
//	UCH* pScan = _pwin + _start;
//	UCH* strend = pScan + MAX_MATCH - 1;
//	// �������е����ƥ��
//	int bestLen = 0;
//	UCH* pCurMatchStart;
//	USH curMatchLen = 0;
//	// ��ʼƥ��
//	do
//	{
//		// ��������hashHead��λ�ÿ�ʼƥ��
//		pCurMatchStart = _pwin + hashHead;
//		while (pScan < strend && *pScan == *pCurMatchStart)
//		{
//			pScan++;
//			pCurMatchStart++;
//		}
//		// ����ƥ��ĳ��Ⱥ�ƥ�����ʼλ��
//		curMatchLen = (MAX_MATCH - 1) - (int)(strend - pScan);
//		pScan = strend - (MAX_MATCH - 1);
//		/*�������ƥ��ļ�¼*/
//		if (curMatchLen > bestLen)
//		{
//			matchStart = hashHead;
//			bestLen = curMatchLen;
//		}
//	} while ((hashHead = _ht._prev[hashHead & WMASK]) > limit
//		&& --chain_length != 0);
//	return curMatchLen;
//}

// matchHead: ƥ��������ʼλ��
// curMatchDist: ����Ͳ����������ƥ��ľ���
// ����ֵ���ƥ�䳤��
UCH  LZ77::LongestMatch(USH matchHead, USH& curMatchDist)
{	
	// ��ϣ������ܳ���256����ֹ��ѭ��
	int chain_length = 256;

	// 
	USH limit = _start > MAX_DIST ? _start - MAX_DIST : 0;
	
	UCH maxLen = 0;
	USH maxDist = 0;
	do
	{
		// �ڲ��һ������ҵ�ƥ�䴮����ʼλ��
		UCH curMatchLen = 0;
		UCH* pStart = _pwin + _start;
		UCH* pCurStart = _pwin + matchHead;
		UCH* pCurEnd = pCurStart + MAX_MATCH;
		// �ҵ�������ƥ�䳤��
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

	// ���Ҵ����������
	size_t readSize = 0;
	if (!feof(fIn))
	{
		readSize = fread(_pwin + _start + _lookAhead, 1, WSIZE, fIn);

		_lookAhead += readSize;
	}
}
