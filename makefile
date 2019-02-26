main: fileCompress.cpp fileCompress.h huffmanTree.cpp huffmanTree.h test.cpp
	g++ $^ -o $@ -std=c++11

.PHONY: clean

clean: main ceshi.huff jieya.txt
	rm -f $^
