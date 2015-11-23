#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <assert.h>

typedef unsigned char byte;

std::string operator*(const std::string& s, unsigned int n) {
	std::stringstream out;
	while (n--) out << s;
	return out.str();
}

typedef enum : unsigned { RLE = 0x000F, MtFT = 0x00F0, BWT = 0x0F00, Huffman = 0xF000 } _compression_alg;

class Compressor {
public:
	Compressor() {}
	virtual ~Compressor() {}

	std::string compress(std::string &input, unsigned compression_alg = 0xFFFF) {
		std::string compressed = input;
		if ((compression_alg & RLE) == RLE) compressed = compressRLE(compressed);
		if ((compression_alg & BWT) == BWT) compressed = compressBWTransform(compressed);
		if ((compression_alg & MtFT) == MtFT) compressed = compressMtFTransform(compressed);
		if ((compression_alg & RLE) == RLE) compressed = compressRLE(compressed);
		if ((compression_alg & Huffman) == Huffman) compressed = compressHuffman(compressed);
		return compressed;
	}
	std::string decompress(std::string &input, unsigned compression_alg = Huffman) {
		std::string decompressed = input;
		if ((compression_alg & Huffman) == Huffman) decompressed = decompressHuffman(decompressed);
		if ((compression_alg & RLE) == RLE) decompressed = decompressRLE(decompressed);
		if ((compression_alg & MtFT) == MtFT) decompressed = decompressMtFTransform(decompressed);
		if ((compression_alg & BWT) == BWT) decompressed = decompressBWTransform(decompressed);
		if ((compression_alg & RLE) == RLE) decompressed = decompressRLE(decompressed);
		return decompressed;
	}

	static std::string fromBytes(const byte* arr, unsigned size) {
		if (!arr || size == 0) return "";
		std::string str(size, 0);
		for (unsigned i = 0; i < size; i++) str[i] = byte2char(arr[i]);
		return str;
	}
	static byte* toBytes(std::string &str) {
		if (str.empty()) return NULL;
		byte *arr = new byte[str.size()];
		for (int i = 0; i < (int)str.size(); i++) arr[i] = char2byte(str[i]);
		return arr;
	}

protected:

	static inline byte char2byte(const char& c) {
		return (byte(c & 0x80) + byte(c & 0x7F));
	}
	static inline char byte2char(const byte& b) {
		return char(char(b & 0x7F) | char(b & 0x80));
	}

	void setBits(bool *bits, byte *output) {
		*output = 0;
		for (byte i = 0; i < 8; i++) if ( *(bits+i) ) *output |= byte(1 << i);
	}
	byte getNumSameBits(bool *bits, byte window) {
		byte i = 0;
		while (++i < window) if ( (*bits) ^ (*(bits+i)) ) break;
		return i;
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
	std::string toBitString(std::string &input) {
		std::string bits = "";
		for (size_t i = 0; i < input.size(); ++i) {
			int c = int(char2byte(input[i]));
			int j = 0;
			while (j < 8) {
				bits += std::string(1, ('0'+char((c>>j) & 0x1)));
				j++;
			}
		}
		return bits;
	}
	std::string encodeBitString(std::string &bitstr) {
		size_t sz = bitstr.size();
		// adding padding
		size_t padd = (sz % 8) ? (8 - (sz % 8)) : (0);
		bitstr += std::string("1") * padd;
		// convert bit string to byte-string
		std::string res = "";
		for (size_t i = 0; i < bitstr.size(); i += 8) {
			byte b = 0;
			for (int k = 0; k < 8; k++)
				if (bitstr[i+k] == '1')
					b |= byte(0x1 << k);
			res += std::string(1, byte2char(b));
		}
		return res;
	}
	std::string decodeBitString(std::string &bytestr, std::string &symbols) {
		std::string bitstr = toBitString(bytestr);
		size_t sz = bitstr.size();
		if (bitstr[sz-1] != '0') {
			int k = 1;
			while (bitstr[sz-k] == '1') k++;
			k--;
			if (k < 8) bitstr = bitstr.substr(0, sz-k);
		}
		std::string decoded = "";
		size_t s = symbols.size()-1;
		while (bitstr.size() > 0) {
			if (bitstr[0] == '0') {
				decoded += symbols[0];
				bitstr = bitstr.substr(1);
				continue;
			}
			size_t j = bitstr.find_first_of("0");
			std::string sub = bitstr.substr(0, j);
			if (sub != std::string("1")*sub.size()) return "-1";
			if (sub.size() < s) {
				// not the last symbol code
				decoded += symbols[sub.size()];
				bitstr = bitstr.substr(j+1);
			} else {
				// the last symbol code
				// sub.size() > s !!! (sub may also contains the next code!!!)
				decoded += symbols[s];
				bitstr = bitstr.substr(s);
			}
		}
		return decoded;
	}
	std::string ror(std::string &s, size_t i) {
		if (i == 0) return std::string(s);
		std::rotate(s.rbegin(), s.rbegin() + 1, s.rend());
		return ror(s, i-1);
	}
	std::string rol(std::string &s, size_t i) {
		if (i == 0) return std::string(s);
		std::rotate(s.begin(), s.begin() + 1, s.end());
		return rol(s, i-1);
	}


	static const char bwtspecial = (char)-127;
	static const char rlespecial = (char)-126;

	std::string compressRLE(std::string input) {
		assert(!input.empty());
		
		int sz = (int)input.size(), cnt = 1, i = 1;
		char prev = input[0];
		std::string res = "";

		while (i < sz) {
			if (input[i] == input[i-1]) cnt++;
			else {
				res += prev;
				if (cnt > 1) {
					if (cnt >= 3) {
						res += rlespecial;
						res += byte2char(cnt+2);
					}
					else res += std::string(1,prev)*(cnt-1);
				}
				if (input[i] == rlespecial) res += rlespecial;
				cnt = 1;
				prev = input[i];
			}
			i++;
		}

		res += prev;
		if (cnt > 1) {
			if (cnt >= 3) {
				res += rlespecial;
				res += byte2char(cnt+2);
			}
			else res += std::string(1,prev)*(cnt-1);
		}

		if (input[0] == rlespecial) res = rlespecial + res;

		if ((int)res.size() >= sz) return input;
		else return rlespecial + res;
	}
	std::string decompressRLE(std::string input) {
		assert(!input.empty());

		int pos = input.find_first_not_of(rlespecial, 0);
		if (pos <= 0) return input;

		std::string res = input.substr(1, pos-1);
		int i = pos;

		char last = input[i];
		int cnt = 1, sz = (int)input.size();

		while (i < sz) {
			if (input[i] == rlespecial) {
				if ((i+3 < sz) && (input[i+1] == rlespecial) && (input[i+2] == rlespecial) && (input[i+3] != rlespecial)) {
					res += std::string(1,rlespecial) * (char2byte(input[i+3])-2);
					i += 4;
				} else if ((i+2 < sz) && (input[i+1] == rlespecial) && (input[i+2] != rlespecial)) {
					res += rlespecial;
					i += 2;
				} else if ((i+1 < sz) && (input[i+1] != rlespecial)) {
					cnt = (char2byte(input[i+1])-2);
					res += std::string(1,last) * (cnt-1);
					i += 2;
				} else {
					res += rlespecial;
					i++;
				}
			} else {
				res += input[i];
				last = input[i];
				cnt = 1;
				i++;
			}
			if ((i >= sz-1) && input[sz-1] == rlespecial) break;
		}

		return res;
	}

	std::string compressMtFTransform(std::string input) {
		std::string res = "", alph = "";
		for (int i = 0; i < 256; i++) alph += byte2char((byte)i);
		for (int i = 0; i < (int)input.size(); i++) {
			char c = input[i];
			byte pos = (i == 0) ? char2byte(c) : (byte)alph.find_first_of(c);
			res += byte2char(pos);
			alph.erase(alph.begin()+pos);
			alph.insert(0, 1, c);
		}
		return res;
	}
	std::string decompressMtFTransform(std::string input) {
		std::string res = "", alph = "";
		for (int i = 0; i < 256; i++) alph += byte2char((byte)i);
		for (int i = 0; i < (int)input.size(); i++) {
			byte pos = char2byte(input[i]);
			char c = alph[pos];
			res += alph[pos];
			alph.erase(alph.begin()+pos);
			alph.insert(0, 1, c);
		}
		return res;
	}

	std::string compressBWTransform(std::string input) {
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
		return byte2char((byte)res.find_first_of(bwtspecial)) + res;
	}
	std::string decompressBWTransform(std::string input) {
		//
		// Burrows-Wheeler Transform
		// http://nuwen.net/bwtzip.html
		//
		int n = (int)char2byte(input[0]);
		input = input.substr(1);
		size_t sz = input.size();

		std::string F = input;
		std::sort(F.begin(), F.end());
		
		int *T = new int[sz];
		memset(T, 0, sz);

		int letters[256] = {-1};
		bool beg = false;
		
		for (size_t i = 0; i < sz; i++) {
			byte l = char2byte(F[i]);
			int pos = (int)input.find(F[i], letters[l]+1);
			if (!beg && input[0] == F[i]) {
				pos = 0;
				beg = true;
			}
			letters[l] = pos;
			T[i] = pos;
		}
		
		std::string res = "";
		for (size_t i = 0; i < input.size(); i++) {
			res += input[n];
			n = T[n];
		}
		
		delete[] T;
		return res.substr(1);
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
			int ind = (int)char2byte(input[i]);
			if (table[ind].first == 0) table[ind].first = ind;
			if (table[ind].second == 0) nullcnt--;
			table[ind].second++;
		}

		qsort(table, 256, sizeof(std::pair<int,int>), paircompare);

		int sz = 256-nullcnt;
		int *symbols = new int[sz];

		// header - sorted symbols array for decompressing
		std::string header = "";

		for (int i = 0; i < sz; i++) {
			symbols[i] = table[256-i-1].first;
			header += std::string(1, byte2char((byte)symbols[i]));
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
		for (size_t i = 0; i < input.size(); i++) compressed += alphabet[getIndex(symbols,sz,(int)char2byte(input[i]))];
		
		delete[] symbols;
		delete[] alphabet;
		return byte2char(header.size()) + header + encodeBitString(compressed);
	}
	std::string decompressHuffman(std::string input) {
		//
		// preform the compressing using Huffman algorithm:
		// http://www.cprogramming.com/tutorial/computersciencetheory/huffman.html
		//

		// 1. get the symbols table from the compressed string
		int symbolsSize = (int)char2byte(input[0]);
		std::string symbols = input.substr(1, symbolsSize);
		// remove header size + header
		input = input.substr(symbols.size()+1);
		// perform decompressing
		return decodeBitString(input, symbols);
	}
};