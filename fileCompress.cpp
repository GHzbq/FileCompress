#include "fileCompress.h"

void fileCompress::writeHuffmanCode(PHTN pRoot)
{
	if (nullptr == pRoot)
		return;
	
	if ((pRoot->_pLeft == nullptr) && (pRoot->_pRight == nullptr))
	{
		std::string & strCode = _v_charset[/* unsigned char */(pRoot->_weight)._ch]._strCode;
		PHTN pcur = pRoot;
		PHTN pParent = pcur->_pParent;
		while (pParent)
		{
			if (pParent->_pLeft == pcur)
			{
				strCode += '0';
			}
			else if(pParent->_pRight == pcur)
			{
				strCode += '1';
			}
			pcur = pParent;
			pParent = pParent->_pParent;
		}

		// ����ͳ�Ƶı����Ǵ�Ҷ�ӽڵ㵽���ڵ�ģ���Ҫ��ת
		reverse(strCode.begin(), strCode.end());

		// Ҷ�ӽڵ㴦����ϣ����ش�������Ҷ�ӽڵ�
		return;
	}

	writeHuffmanCode(pRoot->_pLeft);
	writeHuffmanCode(pRoot->_pRight);
}


void  fileCompress::FileCompress(const std::string fileName)
{
	// 1. ��ͳ��ѹ���ļ��ַ����ִ���
	FILE* fIn = fopen(fileName.c_str(), "rb");

	if (nullptr == fIn)
	{
		std::cout << "ѹ���ļ���ʧ��" << std::endl;
		return;
	}

	// �����Ȳ���һ���ļ���С������ļ���СΪ0�����ǾͲ�ѹ����
	fseek(fIn, 0, SEEK_END);
	long int fSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);
	if (fSize == 0)
	{
		std::cout << "ѹ���ļ���СΪ0" << std::endl;
		fclose(fIn);
		return;
	}
	
	unsigned char Readbuf[1024] = { 0 };
	
	_v_charset.resize(256);
	for (size_t i = 0; i < _v_charset.size(); ++i)
	{
		_v_charset[i]._ch = i;
	}

	while (!feof(fIn))
	{
		memset(Readbuf, 0x00, sizeof(Readbuf) / sizeof(Readbuf[0]));
		size_t rdSize = fread(Readbuf, sizeof(unsigned char), sizeof(Readbuf) / sizeof(Readbuf[0]), fIn);
		if (rdSize == 0)
		{
			// �����ļ���
			break;
		}
		
		for (size_t i = 0; i < rdSize; ++i)
		{
			// ͳ�ƴ���
			++_v_charset[/* (unsigned char)*/Readbuf[i]]._charCount;
		}
	}

	
	// �����ļ�ָ�뵽�ļ���ʼ��
	fseek(fIn, 0, SEEK_SET);

	// 2. ����ͳ�Ƴ����ַ���������huffman��
	huffmanTree<charInfo> ht;
	ht.createHuffmanTree(_v_charset);

	// 3. ����huffman����ͳ��huffman����
	writeHuffmanCode(ht.getPRoot());

	// 4. Ϊ�˷����ѹ�������ǰ�huffmanҶ�ڵ���Ϣд��ѹ���ļ��Ŀ�ʼ����
	std::string FC_filename;
	FC_filename += fileName.substr(/*fileName.rfind('\\')*/0, fileName.rfind('.'));
	FC_filename += ".huff";

	FILE* fOut = fopen(FC_filename.c_str(), "wb");
	if (fOut == nullptr)
	{
		std::cout << "ѹ���ļ���ʧ��" << std::endl;
		return;
	}
	std::string HeadInfo;
	HeadInfo += fileName.substr(fileName.rfind('.'));
	HeadInfo += '\n';
	std::string Head;
	charInfo invaild;
	size_t legalNodeCount = 0;
	for (size_t i = 0; i < _v_charset.size(); ++i)
	{
		if (_v_charset[i]._charCount)
		{
			++legalNodeCount;
			Head += _v_charset[i]._ch;
			Head += ','; 
			// memset(Readbuf, 0x00, 1024);
			// _itoa( _v_charset[i]._charCount, Readbuf, 10);
			std::string str_uint = std::to_string(_v_charset[i]._charCount);
			// Head += Readbuf;
			Head += str_uint;
			Head += '\n';
		}
	}
	/*memset(Readbuf, 0x00, sizeof(Readbuf) / sizeof(Readbuf[0]));
	_itoa(legalNodeCount, Readbuf, 10);
	HeadInfo += Readbuf;*/
	std::string str_uint = std::to_string(legalNodeCount);
	HeadInfo += str_uint;
	HeadInfo += '\n';
	HeadInfo += Head;
	fwrite(HeadInfo.c_str(), sizeof(unsigned char), HeadInfo.size(), fOut);


	// 5. ����huffman���룬ѹ���ļ�
	memset(Readbuf, 0x00, sizeof(Readbuf) / sizeof(Readbuf[0]));
	//unsigned char WriteBuf[1024];
	size_t writeSize = 0;
	int pos = 0;
	unsigned char temp = 0;

	while (true)
	{
		size_t rdSize = fread(Readbuf, sizeof(unsigned char), sizeof(Readbuf) / sizeof(Readbuf[0]), fIn);
		if (rdSize == 0)
		{
			// �ļ�������
			break;
		}

		for (size_t i = 0; i < rdSize; ++i)
		{
			std::string& strCode = _v_charset[Readbuf[i]]._strCode;

			for (size_t j = 0; j < strCode.size(); ++j)
			{
				temp <<= 1;
				if ('1' == strCode[j])
				{
					temp |= 1;
				}

				++pos;
				if (8 == pos)
				{
					fputc(temp, fOut);
					pos = 0;
					temp = 0;
				}

			}
		}
	} // end of while(true)

	if (pos > 0 && pos < 8)
	{
		temp <<= (8 - pos);
		fputc(temp, fOut);
	}

	//while (!feof(fIn))
	//{
	//	memset(Readbuf, 0x00, sizeof(Readbuf) / sizeof(Readbuf[0]));

	//	size_t rdSize = fread(Readbuf, sizeof(unsigned char), sizeof(Readbuf) / sizeof(Readbuf[0]), fIn);
	//	if (rdSize == 0)
	//	{
	//		// �ļ�������
	//		break;
	//	}

	//	// 
	//	pos = 7;
	//	temp = 0;
	//	for (size_t i = 0; i < rdSize; ++i)
	//	{
	//		std::string &strCode = _v_charset[/*(unsigned char)*/Readbuf[i]]._strCode;
	//		for (size_t j = 0; j < strCode.size(); ++j)
	//		{
	//			if (strCode[j] == '1')
	//			{
	//				temp |= (1 << pos);
	//			}
	//			else
	//			{
	//				temp &= ~(1 << pos);
	//			}

	//			--pos;
	//			if (pos == -1)
	//			{
	//				pos = 7;
	//				WriteBuf[writeSize++] = temp;
	//				temp = 0;
	//			}

	//			if (writeSize == 1024)
	//			{
	//				fwrite(WriteBuf, sizeof(unsigned char), 1024, fOut);
	//				writeSize = 0;
	//			}
	//		}
	//	}// end of for
	//}// end of while
	//if (pos != -1 && writeSize != 0)
	//{
	//	// temp <<= pos; // �о������������
	//	WriteBuf[writeSize++] = temp;
	//	fwrite(WriteBuf, sizeof(unsigned char), writeSize, fOut);// 0x80 1000 0000
	//	writeSize = 0;
	//}

	fclose(fIn);
	fclose(fOut);
}

void fileCompress::UnFileCompress(const std::string fileName)
{
	// �ȼ���ѹ�ļ���ʽ�Ϸ���
	// ��׺�Ƿ��� .huff
	std::string suffix = fileName.substr(fileName.rfind('.'));
	if (suffix != ".huff")
	{
		std::cout << "��ѹ���ļ���ʽ���Ϸ�" << std::endl;
		return;
	}

	std::string sourceFilename = fileName.substr(0, fileName.rfind('.'));

	FILE* fIn = fopen(fileName.c_str(), "rb");
	if (nullptr == fIn)
	{
		std::cout << "��ѹ���ļ���ʧ��" << std::endl;
		return;
	}

	suffix.clear();
	getLine(fIn, suffix);
	sourceFilename += suffix;

	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	FILE* fOut = fopen("jieya.txt"/*sourceFilename.c_str()*/, "wb");
	if (fOut == nullptr)
	{
		std::cout << "�ļ���ʧ��" << std::endl;
		return;
	}

	std::string str_lineCount;
	getLine(fIn, str_lineCount);
	size_t lineCount = /*std::stoi(str_lineCount);*/atoi(str_lineCount.c_str());
	_v_charset.clear();
	_v_charset.resize(256);

	for (size_t i = 0; i < _v_charset.size(); ++i)
	{
		_v_charset[i]._ch = i;
		_v_charset[i]._charCount = 0;
		_v_charset[i]._strCode = "";
	}

	std::string NodeInfo;
	for (size_t i = 0; i < lineCount; ++i)
	{
		NodeInfo.clear();
		getLine(fIn, NodeInfo); // �˴��������� // \n,1
		if (NodeInfo.empty())
		{
			NodeInfo += '\n';
			getLine(fIn, NodeInfo);
		}
		_v_charset[(unsigned char)NodeInfo[0]]._charCount = atoi(NodeInfo.c_str() + 2);
	}

	// �ؽ�huffman��
	huffmanTree<charInfo> ht;
	ht.createHuffmanTree(_v_charset);

	PHTN pcur = ht.getPRoot();
	if (nullptr == pcur)
	{
		std::cout << "huffman������ʧ��" << std::endl;
		fclose(fIn);
		fclose(fOut);
		return;
	}

	// ��ȡhuffman����
	writeHuffmanCode(ht.getPRoot());
	
	
	//size_t totalSize = (ht.getPRoot())->_weight._charCount;
	//PHTN pcur = ht.getPRoot();
	//unsigned char ch = fgetc(fIn);
	//int pos = 8;
	//while (1)
	//{
	//	--pos;
	//	if ((ch >> pos) & 1)
	//	{
	//		pcur = pcur->_pRight;
	//	}
	//	else
	//	{
	//		pcur = pcur->_pLeft;
	//	}

	//	if (pcur->_pLeft == nullptr && pcur->_pRight == nullptr)
	//	{
	//		fputc(pcur->_weight._ch, fOut);
	//		pcur = ht.getPRoot();   //�ٴδӸ��ڵ����
	//		--totalSize;
	//	}
	//	
	//	if (pos == 0)
	//	{
	//		ch = fgetc(fIn);
	//		pos = 8;
	//	}
	//	if (totalSize == 0)  //����ȡѹ��ʱΪ�˴չ�һ���ֽڶ��ӽ�ȥ�ı���λ
	//		break;
	//}

	unsigned char ReadBuf[1024] = { 0 };
	unsigned char WriteBuf[1024] = { 0 };
	size_t wtSize = 0;

	pcur = ht.getPRoot();
	size_t totalSize = pcur->_weight._charCount;
	
	
	// long int curfileSize = ftell(fIn);

	// int count = 0;

	int pos = 7;
	while (true)
	{
		size_t rdSize = fread(ReadBuf, sizeof(unsigned char), sizeof(ReadBuf) / sizeof(ReadBuf[0]), fIn);
		if (rdSize == 0)
		{
			break;
		}

		for (size_t i = 0; i < rdSize; ++i)
		{
			pos = 7;
			unsigned char uch = ReadBuf[i];
			for (size_t j = 0; j < 8; ++j)
			{
				if (uch & (1 << pos))
				{
					// 1
					pcur = pcur->_pRight;
				}
				else
				{
					// 0
					pcur = pcur->_pLeft;
				}
				--pos;
				
				// ���pcur�Ƿ��ߵ�Ҷ�ӽڵ�λ��
				if (pcur->_pLeft == nullptr && pcur->_pRight == nullptr)
				{
					// ��Ҷ�ӽڵ�
					fputc((pcur->_weight)._ch, fOut);
					pcur = ht.getPRoot();
					--totalSize;
					if (0 == totalSize)
					{
						break;
					}
				}
			} // end of for(j)
		}// end of for(i)
	}// end of while(true)

	//while (!feof(fIn))
	//{
	//	// ����ַ���
	//	memset(ReadBuf, 0x00, sizeof(ReadBuf)/sizeof(ReadBuf[0]));
	//	size_t rdSize = fread(ReadBuf, sizeof(unsigned char), sizeof(ReadBuf) / sizeof(ReadBuf[0]), fIn);
	//	// ++count;
	//	//int ret = feof(fIn);
	//	//if (ret == 0)
	//	//{
	//	//	std::cout << "fghjkll\n";// 
	//	//}
	//	if (rdSize == 0)
	//	{
	//		// �ļ��Ѷ���
	//		return;
	//	}
	//	unsigned char ch = 0;
	//	int pos = 7;
	//	for (size_t i = 0; i < rdSize ; ++i)
	//	{
	//		ch = ReadBuf[i];
	//		while (pos > -1)
	//		{
	//			if (ch & (1 << pos))
	//			{
	//				--pos;
	//				pcur = pcur->_pRight;
	//			}
	//			else
	//			{
	//				--pos;
	//				pcur = pcur->_pLeft;
	//			}

	//			if (pcur->_pLeft == nullptr && pcur->_pRight == nullptr)
	//			{
	//				// Ҷ�ӽڵ���
	//				WriteBuf[wtSize++] = (pcur->_weight)._ch;
	//				--totalSize;
	//				if (totalSize == 0)
	//				{
	//					if (wtSize != 0)
	//					{
	//						fwrite(WriteBuf, sizeof(unsigned char), sizeof(WriteBuf) / sizeof(WriteBuf[0]), fOut);
	//						memset(WriteBuf, 0x00, sizeof(WriteBuf) / sizeof(WriteBuf[0]));
	//						wtSize = 0;
	//					}
	//					break;
	//				}
	//				if (wtSize == 1024)
	//				{
	//					fwrite(WriteBuf, sizeof(unsigned char), sizeof(WriteBuf) / sizeof(WriteBuf[0]), fOut);
	//					memset(WriteBuf, 0x00, sizeof(WriteBuf)/sizeof(WriteBuf[0]));
	//					wtSize = 0;
	//				}
	//				pcur = ht.getPRoot();
	//			}
	//		} // end of while
	//		pos = 7;
	//		if (totalSize == 0)
	//		{
	//			if (wtSize != 0)
	//			{
	//				fwrite(WriteBuf, sizeof(unsigned char), sizeof(WriteBuf) / sizeof(WriteBuf[0]), fOut);
	//				memset(WriteBuf, 0x00, sizeof(WriteBuf) / sizeof(WriteBuf[0]));
	//				wtSize = 0;
	//			}
	//			break;
	//		}
	//	}

	//	if (totalSize == 0)
	//	{
	//		break;
	//	}

	//}// end of while

	

	if (wtSize > 0)
	{
		fwrite(WriteBuf, sizeof(unsigned char), wtSize, fOut);
	}
	fclose(fIn);
	fclose(fOut);
}

void fileCompress::getLine(FILE* PF, std::string & str)
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
