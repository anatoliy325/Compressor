#pragma once

#include <sstream>
#include <string>
#include <algorithm>
#include <assert.h>

#include "linkedlist.h"
using namespace linkedlist;


typedef unsigned char byte;

typedef enum : unsigned { RLE = 0x000F, MtFT = 0x00F0, BWT = 0x0F00, Huffman = 0xF000 } _compression_alg;

class Compressor {
public:
	Compressor() {}
	virtual ~Compressor() {}

	int compress(byte *input, int inputSize, byte *compr, int *comprSize, unsigned compression_alg = 0xFFFF) {
		int res = 0;
		/*if ((compression_alg & RLE) == RLE) res = compressRLE(input, inputSize, );
		if ((compression_alg & BWT) == BWT) compressed = compressBWTransform(compressed);
		if ((compression_alg & MtFT) == MtFT) compressed = compressMtFTransform(compressed);
		if ((compression_alg & RLE) == RLE) compressed = compressRLE(compressed);
		if ((compression_alg & Huffman) == Huffman) compressed = compressHuffman(compressed);*/
		return res;
	}
	int decompress(byte *input, int inputSize, byte *compr, int *comprSize, unsigned compression_alg = Huffman) {
		int res = 0;
		/*if ((compression_alg & Huffman) == Huffman) decompressed = decompressHuffman(decompressed);
		if ((compression_alg & RLE) == RLE) decompressed = decompressRLE(decompressed);
		if ((compression_alg & MtFT) == MtFT) decompressed = decompressMtFTransform(decompressed);
		if ((compression_alg & BWT) == BWT) decompressed = decompressBWTransform(decompressed);
		if ((compression_alg & RLE) == RLE) decompressed = decompressRLE(decompressed);*/
		return res;
	}

private:
	void setBits(bool *bits, byte *output) {
		*output = 0;
		for (byte i = 0; i < 8; i++) if ( *(bits+i) ) *output |= byte(1 << i);
	}
	byte getNumSameBits(bool *bits, byte window) {
		byte i = 0;
		while (++i < window) if ( (*bits) ^ (*(bits+i)) ) break;
		return i;
	}

	byte* subarray(byte* inArray, int inSize, int offset = 0, int count = -1) {
		if (!inArray || inSize <= 0) return NULL;
		if (offset == 0 && count == -1) return inArray;
		offset = (offset < 0) ? (inSize+offset) : offset;
		count = (count < 0) ? (inSize-offset-1) : count;
		if ((offset+count) >= inSize) return NULL;
		byte* dest = new byte[count];
		memcpy(dest, (inArray+offset), count);
		return dest;
	}
	byte* subarrayinplace(byte** inArray, int *inSize, int offset = 0, int count = -1) {
		if (!inArray || !(*inArray)) return NULL;
		byte *buff = subarray(*inArray, *inSize, offset, count);
		if (!buff) return NULL;
		delete[] (*inArray);
		*inArray = buff;
		*inSize = (count < 0) ? ((*inSize)-offset-1) : count;
		return (*inArray);
	}
	int search(byte* inArray, int inSize, const byte& value, int offset = 0) {
		if (!inArray || inSize <= 0) return -1;
		offset = (offset < 0) ? (inSize+offset) : offset;
		for (int i = offset; i < inSize; i++)
			if (inArray[i] == value)
				return i;
		return -1;
	}
	bool issequence(byte* inArray, int inSize, const byte& value, int offset = 0) {
		if (!inArray || inSize <= 0) return false;
		offset = (offset < 0) ? (inSize+offset) : offset;
		for (int i = offset; i < inSize; i++) 
			if (inArray[i] != value) return false;
		return true;
	}

	static int paircompare(const void *a, const void *b) {
		//int ca = (*(std::pair<int,int>*)a).first, cb = (*(std::pair<int,int>*)b).first;
		int va = (*(std::pair<int,int>*)a).second, vb = (*(std::pair<int,int>*)b).second;
		return va - vb; //(va == vb) ? (ca - cb) : (va - vb);
	}
	int getIndex(int *arr, int size, int item) {
		for (int i = 0; i < size; i++)
			if (arr[i] == item) return i;
		return -1;
	}
	byte* toBitString(byte *input, int inSize, int *outSize) {
		if (!input || inSize <= 0 || !outSize) return NULL;
		int sz = inSize*8;
		byte *bits = new byte[sz];
		memset(bits, 0, sz);
		for (int i = 0; i < inSize; ++i) {
			int c = (int)input[i];
			int j = 0;
			while (j < 8) {
				bits[i+j] = byte('0'+char((c>>j) & 0x1));
				j++;
			}
		}
		*outSize = sz;
		return bits;
	}
	byte* encodeBitString(byte *bitstr, int inSize, int *outSize) {
		if (!bitstr || inSize <= 0 || !outSize) return NULL;
		// adding padding
		int padd = (inSize % 8) ? (8 - (inSize % 8)) : (0);
		int sz = inSize + padd;
		byte *paddedbits = new byte[sz];
		memcpy(paddedbits, bitstr, inSize);
		memset(paddedbits, byte('1'), padd);
		// convert bit string to byte-string
		int newsz = sz / 8;
		byte *encoded = new byte[newsz];
		memset(encoded, 0, newsz);
		for (int i = 0; i < sz; i += 8) {
			byte b = 0;
			for (int k = 0; k < 8; k++)
				if (paddedbits[i+k] == byte('1'))
					b |= byte(0x1 << k);
			encoded[i/8] = b;
		}
		// clean-up
		delete[] paddedbits;
		//
		*outSize = newsz;
		return encoded;
	}
	byte* decodeBitString(byte *bytestr, int inSize, int *outSize, byte *symbols, int symbSize) {
		int sz = 0;
		byte *bitstr = toBitString(bytestr, inSize, &sz);
		
		if (bitstr[sz-1] != byte('0')) {
			int k = 1;
			while (bitstr[sz-k] == byte('1')) k++;
			k--;

			if (k < 8) subarrayinplace(&bitstr, &sz, 0, sz-k);
			/*else {
				int delta = 0;
				if (k < ((int)symbols.size()-1)) delta = k;
				if (k > ((int)symbols.size()-1)) delta = k - ((int)symbols.size()-1);
				
				bitstr = bitstr.substr(0, sz-delta);
				std::cout << bitstr << "\t" << bitstr.size() << "\n";
			}*/
		}

		byte *decoded = new byte[sz];
		int s = symbSize-1;
		int k = 0;
		
		while (sz > 0) {
			if (bitstr[0] == byte('0')) {
				decoded[k++] = symbols[0];
				subarrayinplace(&bitstr, &sz, 1);
				continue;
			}
			int j = search(bitstr, sz, byte('0'));
			byte* sub = subarray(bitstr, sz, 0, j);
			if (!issequence(sub, j, byte('1'))) {
				delete[] sub;
				delete[] decoded;
				delete[] bitstr;
				return NULL;
			}
			if (j < s) {
				// not the last symbol code
				decoded[k++] = symbols[j];
				subarrayinplace(&bitstr, &sz, j+1);
			} else {
				// the last symbol code
				// sub.size() > s !!! (sub may also contains the next code!!!)
				decoded[k++] = symbols[s];
				subarrayinplace(&bitstr, &sz, s);
			}
			delete[] sub;
		}
		byte* buff = new byte[k];
		memcpy(buff, decoded, k);
		delete[] decoded;
		delete[] bitstr;
		return buff;
	}
	
	std::string ror(std::string s, size_t i) {
		if (i == 0) return s;
		std::rotate(s.rbegin(), s.rbegin() + 1, s.rend());
		return ror(s, i-1);
	}
	std::string rol(std::string s, size_t i) {
		if (i == 0) return s;
		std::rotate(s.begin(), s.begin() + 1, s.end());
		return rol(s, i-1);
	}

public:
	static const byte headerseparator = (byte)'|';
	static const byte bwtspecial = (byte)0;

	static byte* str2byte(std::string str) {
		if (str.size() == 0) return NULL;
		byte *arr = new byte[str.size()];
		//memcpy(arr, str.c_str(), str.size());
		for (size_t i = 0; i < str.size(); i++) arr[i] = byte(str[i]);
		return arr;
	}
	static std::string byte2str(const byte* arr, int inSize) {
		if (!arr || inSize <= 0) return "";
		//char *buff = new char[inSize];
		//memcpy(buff, arr, inSize);
		//std::string str(buff);
		//delete[] buff;
		std::string str = "";
		for (int i = 0; i < inSize; i++) str += arr[i];
		return str;
	}
	static void print(const byte* arr, int inSize, const char* separator = "\n") {
		if (!arr || inSize <= 0) return;
		for (int i = 0; i < inSize; i++) std::cout << arr[i] << separator;
	}

	std::string compressRLE(std::string input) {
		/*size_t sz = input.size();
		size_t i = 0, j = 0;
		std::string res = "";

		while((i+j) < sz) {
			char s = input[i];
			j = (i == sz-1) ? 0 : 1;
			while ((i+j) < sz && input[i+j] == s) j++;

			std::cout << i << "  " << j << "  " << (sz-(i+j)) << "\n";

			i += j;
		}

		return res;*/
		return input;
	}
	std::string decompressRLE(std::string input) {
		return input;
	}

	byte* compressMtFTransform(byte* input, int inSize) {
		if (!input || inSize <= 0) return NULL;
		
		byte *res = new byte[inSize];
		memset(res, 0, inSize);

		_node<byte> *alph = new _node<byte>();
		alph->val = 0;
		alph->next = NULL;
		for (int i = 1; i < 256; i++) pushback<byte>(alph, (byte)i);

		for (int i = 0; i < inSize; i++) {
			byte c = input[i];
			byte pos = (i == 0) ? c : find(alph, c);
			assert(pos >= 0);
			res[i] = pos;
			to_front<byte>(&alph, pos);
		}
		freemem<byte>(alph);

		return res;
	}
	byte* decompressMtFTransform(byte* input, int inSize) {
		if (!input || inSize <= 0) return NULL;
		
		byte *res = new byte[inSize];
		memset(res, 0, inSize);

		_node<byte> *alph = new _node<byte>();
		alph->val = 0;
		alph->next = NULL;
		for (int i = 1; i < 256; i++) pushback<byte>(alph, (byte)i);
		
		for (int i = 0; i < inSize; i++) {
			int pos = (int)input[i];
			byte c = index<byte>(alph, pos);
			res[i] = c;
			to_front<byte>(&alph, pos);
		}
		freemem<byte>(alph);

		return res;
	}
	/*
	std::string compressBWTransform(std::string input) {
		//return input;
		//
		// Burrows-Wheeler Transform
		// http://nuwen.net/bwtzip.html
		//
		size_t sz = input.size() + 1;
		std::string *arr = new std::string[sz];

		for(size_t i = 0; i < sz; i++) arr[i] = bwtspecial+input.substr(i);
		std::sort(arr, arr+sz);

		std::string res = "";
		for (size_t i = 0; i < sz; i++) res += (bwtspecial+input)[sz-arr[i].size()];

		delete[] arr;
		return (char)res.find_first_of(bwtspecial) + res;
	}
	std::string decompressBWTransform(std::string input) {
		//return input;
		//
		// Burrows-Wheeler Transform
		// http://nuwen.net/bwtzip.html
		//
		int n = (int)input[0];
		input = input.substr(1);
		size_t sz = input.size();

		std::string F = input;
		std::sort(F.begin(), F.end());
		
		int *T = new int[sz];
		memset(T, 0, sz);

		int letters[256] = {-1};
		bool beg = false;
		
		for (size_t i = 0; i < sz; i++) {
			byte l = (byte)F[i];
			int pos = input.find(l, letters[l]+1);
			if (!beg && input[0] == l) {
				pos = 0;
				beg = true;
			}
			letters[l] = pos;
			T[i] = pos;
			std::cout << l << "\t" << (int)l << "\t" << pos << "\t" << T[i] << "\n";
		}
		
		std::string res = "";
		
		for (size_t i = 0; i < input.size(); i++) {
			std::cout << i << "\t" << n << "\t" << T[n] << "\n";
			res += input[n];
			n = T[n];
		}
		
		delete[] T;
		return res;//.substr(1);
	}

	std::string compressHuffman(std::string input) {
		//
		// preform the compressing using Huffman algorithm:
		// http://www.cprogramming.com/tutorial/computersciencetheory/huffman.html
		//

		// 1. making the char. freq. index table
		std::pair<int,int> *table = new std::pair<int,int>[256];
		int nullcnt = 256;

		for (size_t i = 0; i < input.size(); i++) {
			if (table[(int)input[i]].first == 0) table[(int)input[i]].first = (int)input[i];
			if (table[(int)input[i]].second == 0) nullcnt--;
			table[(int)input[i]].second++;
		}

		qsort(table, 256, sizeof(std::pair<int,int>), paircompare);

		int sz = 256-nullcnt;
		int *symbols = new int[sz];

		// header - sorted symbols array for decompressing
		std::string header = "";

		for (int i = 0; i < sz; i++) {
			symbols[i] = table[256-i-1].first;
			header += std::string(1, (char)symbols[i]);
		}

		delete[] table;

		// 2. build the alphabet
		std::string *alphabet = new std::string[sz];
		int j = 1;

		alphabet[0] = "0";
		for (int i = 1; i < sz; i++) {
			if (i < sz-1) {
				alphabet[i] = std::string("1") * j;
				alphabet[i] += "0";
			} else {
				alphabet[i] = std::string("1") * (j-1);
				alphabet[i] += "1";
			}
			j++;
		}

		// 3. compress the input string using alphabet
		std::string compressed = "";

		for (size_t i = 0; i < input.size(); i++) {
			std::string encoded = alphabet[getIndex(symbols,sz,(int)input[i])];
			compressed += encoded;
		}

		delete[] symbols;
		delete[] alphabet;

		return std::string(1, headerseparator) + header + std::string(1, headerseparator) + encodeBitString(compressed);
	}
	std::string decompressHuffman(std::string input) {
		//
		// preform the compressing using Huffman algorithm:
		// http://www.cprogramming.com/tutorial/computersciencetheory/huffman.html
		//

		// 1. get the symbols table from the compressed string
		// header is separated with headerseparator character from the both sides
		std::string symbols = input.substr(1, input.find(headerseparator, 1)-1);
		// remove header
		input = input.substr(symbols.size()+2);
		// perform decompressing
		return decodeBitString(input, symbols);
	}
	*/
};