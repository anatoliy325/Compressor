#pragma once

#include "ByteString.h"
#include <sstream>
#include <algorithm>

typedef unsigned char byte;

typedef enum : unsigned { RLE = 0x000F, MtFT = 0x00F0, BWT = 0x0F00, Huffman = 0xF000 } _compression_alg;

class Compressor {
public:
	Compressor() {}
	virtual ~Compressor() {}

	ByteString compress(ByteString &input, unsigned compression_alg = 0xFFFF) {
		ByteString compressed = input;
		if ((compression_alg & RLE) == RLE) compressed = compressRLE(compressed);
		if ((compression_alg & BWT) == BWT) compressed = compressBWTransform(compressed);
		if ((compression_alg & MtFT) == MtFT) compressed = compressMtFTransform(compressed);
		if ((compression_alg & RLE) == RLE) compressed = compressRLE(compressed);
		if ((compression_alg & Huffman) == Huffman) compressed = compressHuffman(compressed);
		return compressed;
	}
	ByteString decompress(ByteString &input, unsigned compression_alg = Huffman) {
		ByteString decompressed = input;
		if ((compression_alg & Huffman) == Huffman) decompressed = decompressHuffman(decompressed);
		if ((compression_alg & RLE) == RLE) decompressed = decompressRLE(decompressed);
		if ((compression_alg & MtFT) == MtFT) decompressed = decompressMtFTransform(decompressed);
		if ((compression_alg & BWT) == BWT) decompressed = decompressBWTransform(decompressed);
		if ((compression_alg & RLE) == RLE) decompressed = decompressRLE(decompressed);
		return decompressed;
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
	ByteString toBitString(ByteString &input) {
		byte *bits = new byte[input.length() * 8];
		memset(bits, 0, input.length() * 8);
		for (int i = 0; i < input.length(); ++i) {
			int c = (int)input[i];
			int j = 0;
			while (j < 8) {
				bits[8*i+j] = byte((c>>j) & 0x1);
				j++;
			}
		}
		return ByteString(bits, input.length() * 8);
	}
	ByteString encodeBitString(ByteString &bitstr) {
		size_t sz = bitstr.length();
		// adding padding
		size_t padd = (sz % 8) ? (8 - (sz % 8)) : (0);
		bitstr += ByteString(byte(1)) * padd;
		// convert bit string to byte-string
		byte *res = new byte[bitstr.length() / 8];
		memset(res, 0, bitstr.length()/8);
		for (int i = 0; i < bitstr.length(); i += 8) {
			byte b = 0;
			for (int k = 0; k < 8; k++) {
				if (bitstr[i+k] == byte(1))
					b |= byte(0x1 << k);
			}
			res[i/8] = b;
		}
		return ByteString(res, bitstr.length()/8);
	}
	ByteString decodeBitString(ByteString &bytestr, ByteString &symbols) {
		ByteString bitstr = toBitString(bytestr);
		size_t sz = bitstr.length();
		if (bitstr[sz-1] != byte(0)) {
			int k = 1;
			while (bitstr[sz-k] == byte(1)) k++;
			k--;

			if (k < 8) bitstr = bitstr.substring(0, sz-k);
			/*else {
				int delta = 0;
				if (k < ((int)symbols.size()-1)) delta = k;
				if (k > ((int)symbols.size()-1)) delta = k - ((int)symbols.size()-1);
				
				bitstr = bitstr.substr(0, sz-delta);
				std::cout << bitstr << "\t" << bitstr.size() << "\n";
			}*/
		}

		ByteString decoded;
		int s = symbols.length()-1;
		
		while (bitstr.length() > 0) {
			if (bitstr[0] == byte(0)) {
				decoded += symbols[0];
				bitstr = bitstr.substring(1);
				continue;
			}
			size_t j = bitstr.search(byte(0), 0);
			ByteString sub = bitstr.substring(0, j);
			if (sub != ByteString(1)*sub.length()) {
				std::cout << j << "\t" << sub << "\t" << sub.length() << "\t" << (ByteString(byte(1))*sub.length()) << "\n";
				return ByteString("-1");
			}
			if (sub.length() < s) {
				// not the last symbol code
				decoded += symbols[sub.length()];
				bitstr = bitstr.substring(j+1);
			} else {
				// the last symbol code
				// sub.size() > s !!! (sub may also contains the next code!!!)
				decoded += symbols[s];
				bitstr = bitstr.substring(s);
			}
		}
		return decoded;
	}

public:
	static const byte headerseparator = (byte)'|';
	static const byte bwtspecial = (byte)0;

	ByteString compressRLE(ByteString input) {
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
	ByteString decompressRLE(ByteString input) {
		return input;
	}

	ByteString compressMtFTransform(ByteString input) {
		ByteString res;
		ByteString alph_str(256, -1);
		LinkedList<byte> alph = alph_str.to_list();

		for (int i = 0; i < input.length(); i++) {
			byte c = input[i];
			int pos = (i == 0) ? (int)c : alph.find(c, 0);
			
			assert(pos >= 0);
			res += (byte)pos;
			
			alph.to_front(pos);
		}
		return res;
	}
	ByteString decompressMtFTransform(ByteString input) {
		ByteString res;
		ByteString alph_str(256, -1);
		LinkedList<byte> alph = alph_str.to_list();
		
		for (int i = 0; i < input.length(); i++) {
			byte pos = (byte)input[i];
			assert(pos >= 0);
			byte c = alph[pos];
			res += c;
			alph.to_front(pos);
		}
		return res;
	}

	ByteString compressBWTransform(ByteString input) {
		//
		// Burrows-Wheeler Transform
		// http://nuwen.net/bwtzip.html
		//
		int sz = input.length() + 1;
		ByteString *arr = new ByteString[sz];
		ByteString special(bwtspecial);

		for(int i = 0; i < sz; i++) arr[i] = special+input.substring(i);
		std::sort(arr, arr+sz, compare());

		ByteString res(sz, 0);
		for (int i = 0; i < sz; i++) res[i] = (special+input)[sz-arr[i].length()];

		delete[] arr;

		return ByteString(byte(res.search(bwtspecial,0))) + res;
	}
	ByteString decompressBWTransform(ByteString input) {
		//
		// Burrows-Wheeler Transform
		// http://nuwen.net/bwtzip.html
		//
		int n = (int)input[0];

		input = input.substring(1);
		int sz = input.length();

		ByteString F = input;
		//std::sort(F.begin(), F.end());
		F.qsort();

		int *T = new int[sz];
		memset(T, 0, sz);

		int letters[256] = {-1};
		bool beg = false;

		for (int i = 0; i < sz; i++) {
			byte l = (byte)F[i];
			int pos = input.search(l, letters[l]+1);
			if (!beg && input[0] == l) {
				pos = 0;
				beg = true;
			}
			letters[l] = pos;
			T[i] = pos;
			//std::cout << l << "\t" << (int)l << "\t" << pos << "\t" << T[i] << "\n";
		}

		ByteString res(input.length(), 0);
		
		for (int i = 0; i < input.length(); i++) {
			//std::cout << i << "\t" << n << "\t" << T[n] << "\n";
			res[i] = input[n];
			n = T[n];
		}
		
		delete[] T;
		return res;//.substring(1);
	}

	ByteString compressHuffman(ByteString input) {
		//
		// preform the compressing using Huffman algorithm:
		// http://www.cprogramming.com/tutorial/computersciencetheory/huffman.html
		//

		// 1. making the char. freq. index table
		std::pair<int,int> *table = new std::pair<int,int>[256];
		int nullcnt = 256;

		for (int i = 0; i < input.length(); i++) {
			if (table[(int)input[i]].first == 0) table[(int)input[i]].first = (int)input[i];
			if (table[(int)input[i]].second == 0) nullcnt--;
			table[(int)input[i]].second++;
		}

		qsort(table, 256, sizeof(std::pair<int,int>), paircompare);

		int sz = 256-nullcnt;
		int *symbols = new int[sz];

		// header - sorted symbols array for decompressing
		ByteString header(sz, 0);

		for (int i = 0; i < sz; i++) {
			symbols[i] = table[256-i-1].first;
			header[i] = byte(symbols[i]);
		}

		// 2. build the alphabet
		ByteString *alphabet = new ByteString[sz];
		int j = 1;

		alphabet[0] = ByteString(byte(0));
		for (int i = 1; i < sz; i++) {
			if (i < sz-1) {
				alphabet[i] = ByteString(byte(1));
				alphabet[i] *= j;
				alphabet[i] += byte(0);
			} else {
				alphabet[i] = ByteString(byte(1));
				alphabet[i] *= (j-1);
				alphabet[i] += byte(1);
			}
			j++;
		}

		// 3. compress the input string using alphabet
		ByteString compressed;

		for (int i = 0; i < input.length(); i++) {
			ByteString encoded = alphabet[getIndex(symbols,sz,(int)input[i])];
			compressed += encoded;
		}

		delete[] symbols;
		delete[] alphabet;

		return ByteString(headerseparator) + header + ByteString(headerseparator) + encodeBitString(compressed);
	}
	ByteString decompressHuffman(ByteString input) {
		//
		// preform the compressing using Huffman algorithm:
		// http://www.cprogramming.com/tutorial/computersciencetheory/huffman.html
		//

		// 1. get the symbols table from the compressed string
		// header is separated with headerseparator character from the both sides
		ByteString symbols = input.substring(1, input.search(headerseparator, 1)-1);
		// remove header
		input = input.substring(symbols.length()+2);

		std::cout << symbols << "\t" << input << "\n\n";

		// perform decompressing
		return decodeBitString(input, symbols);
	}
};