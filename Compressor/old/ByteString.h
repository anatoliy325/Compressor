#pragma once

#include "LinkedList.h"

typedef unsigned char byte;

class ByteString {

protected:
	byte *buffer;
	int size;

public:
	ByteString() : buffer(NULL), size(-1) {}
	ByteString(const byte& b) {
		this->buffer = new byte[1];
		this->size = 1;
		this->buffer[0] = b;
	}
	ByteString(int len, int init = -1) {
		len = (len < 0) ? (-len) : (len);
		len = (init < 0) ? (256) : (len);
		init = (init > 255) ? (255) : (init);
		this->buffer = new byte[len];
		this->size = len;
		for (int i = 0; i < this->size; i++)
			this->buffer[i] = (init < 0) ? (byte)i : (byte)init;
	}
	ByteString(const byte* array, int size) {
		if (!array || size <= 0) {
			this->buffer = NULL;
			this->size = 0;
		}
		this->size = size;
		this->buffer = new byte[size];
		memcpy(this->buffer, array, size);
	}
	ByteString(const char* string) {
		this->size = strlen(string);
		this->buffer = new byte[this->size];
		memcpy(this->buffer, string, this->size);
	}
	ByteString(std::string& string) {
		this->size = string.size();
		this->buffer = new byte[this->size];
		memcpy(this->buffer, string.c_str(), this->size);
	}
	ByteString(LinkedList<byte>& list) {
		this->size = list.length();
		this->buffer = new byte[this->size];
		memset(this->buffer, 0, this->size);
		int i = 0;
		struct LinkedList<byte>::_node *it = (LinkedList<byte>::_node*)list.front();
		while (it) {
			this->buffer[i] = it->value;
			it = it->next;
			i++;
		}
	}
	ByteString(const ByteString& bs) {
		this->size = bs.size;
		this->buffer = new byte[this->size];
		memcpy(this->buffer, bs.buffer, this->size);
	}
	~ByteString() {
		if (this->buffer) delete[] this->buffer;
		this->size = 0;
	}

	const int length() const { return this->size; }
	const bool empty() const { return (!this->buffer || this->size <= 0); }

	const byte* data() const { return this->buffer; }

	void clear() {
		if (this->empty()) return;
		delete[] this->buffer;
		this->size = 0;
	}

	void swap(ByteString &bs) {
		if (!this->buffer || !bs.buffer) return;
		if (this->size == bs.size) {
			byte *buff = new byte[this->size];
			memcpy(buff, this->buffer, this->size);
			memcpy(this->buffer, bs.buffer, bs.size);
			memcpy(bs.buffer, buff, this->size);
		} else {
			byte *buff = new byte[this->size];
			memcpy(buff, this->buffer, this->size);
			
			delete[] this->buffer;
			this->buffer = new byte[bs.size];
			memcpy(this->buffer, bs.buffer, bs.size);
			
			delete[] bs.buffer;
			bs.buffer = new byte[this->size];
			memcpy(bs.buffer, buff, this->size);

			delete[] buff;

			int t = this->size;
			this->size = bs.size;
			bs.size = t;
		}
	}

	void qsort() {
		if (this->empty() || this->size <= 1) return;
		LinkedList<byte> list(this->buffer, this->size);
		list.qsort();
		*this = list;
		list.clear();
	}

	ByteString reverse() {
		if (this->empty()) return ByteString();
		ByteString rev = *this;
		for (int i = 0; i < rev.size; i++)
			rev.buffer[i] = this->buffer[this->size-i-1];
		return rev;
	}

	ByteString substring(int offset, int length = -1) {
		if (this->empty() || offset < 0 || offset >= this->size || length == 0) return ByteString();
		length = (length < 0 || offset+length >= this->size) ? (this->size - offset) : (length);
		byte *buff = new byte[length];
		memcpy(buff, this->buffer+offset, length);
		ByteString sub(buff, length);
		delete[] buff;
		return sub;
	}

	int search(const byte& value, int offset, int length = -1) {
		if (this->empty() || offset < 0 || offset >= this->size || length == 0) return -1;
		length = (length < 0 || offset+length >= this->size) ? (this->size - offset) : (length);
		for (int i = offset; i < offset+length; i++)
			if (this->buffer[i] == value)
				return i;
		return -1;
	}
	int search(ByteString& substring, int offset, int length = -1) {
		if (this->empty() || substring.empty() || offset < 0 || offset >= this->size || length == 0 || length > this->size) return -1;
		
		length = (length < 0 || offset+length >= this->size) ? (this->size - offset) : (length);

		ByteString newstr = (offset == 0) ? (*this) : (this->substring(offset, length));

		int first = 0;
		while (first != -1) {
			first = newstr.search(substring[0], first, length);
			if (first == -1 || (first+substring.size) > length) break;
			if (newstr.substring(first, substring.size) == substring) return first;
			offset += first;
		}

		return -1;
	}
	int search(const char* substring, int offset, int length = -1) {
		if (this->empty() || !substring || offset < 0 || offset > this->size || length == 0 || length > this->size) return -1;
		
		int sz = strlen(substring);
		length = (length < 0 || offset+length >= this->size) ? (this->size - offset) : (length);
		
		ByteString newstr = (offset == 0) ? (*this) : (this->substring(offset, length));

		int first = 0;
		while (first != -1) {
			first = newstr.search(byte(substring[0]), first, length);
			if (first == -1 || (first+sz) > length) break;
			if (newstr.substring(first, sz) == substring) return first;
			offset += first;
		}

		return -1;
	}
	int search(std::string& substring, int offset, int length = -1) {
		return this->search(substring.c_str(), offset, length);
	}

	void ror(int positions) {
		if (this->empty()) return;
		if (positions < 0) return this->rol(-positions);
		LinkedList<byte> list(this->buffer, this->size);
		list.rotate(-positions);
		*this = list;
		list.clear();
	}
	void rol(int positions) {
		if (this->empty()) return;
		if (positions < 0) return this->ror(-positions);
		LinkedList<byte> list(this->buffer, this->size);
		list.rotate(positions);
		*this = list;
		list.clear();
	}

	LinkedList<byte> to_list() {
		LinkedList<byte> list;
		list.pushback(this->data(), this->size);
		return list;
	}

	const byte& operator[](int index) const { return this->buffer[index]; }
	byte& operator[](int index) { return this->buffer[index]; }

	operator const byte*() const { return this->buffer; }

	void operator=(const ByteString& bs)
	{
		if (this->buffer) {
			if (this->size != bs.size) {
				delete[] this->buffer;
				this->buffer = new byte[bs.size];
			}
		} else {
			this->buffer = new byte[bs.size];
		}
		this->size = bs.size;
		memcpy(this->buffer, bs.buffer, bs.size);
	}
	void operator=(const char* string)
	{
		int sz = strlen(string);
		if (this->buffer) {
			if (this->size != sz) {
				delete[] this->buffer;
				this->buffer = new byte[sz];
			}
		} else {
			this->buffer = new byte[sz];
		}
		this->size = sz;
		memcpy(this->buffer, string, this->size);
	}
	void operator=(std::string& string) {
		if (this->buffer) {
			if (this->size != string.size()) {
				delete[] this->buffer;
				this->buffer = new byte[string.size()];
			}
		} else {
			this->buffer = new byte[string.size()];
		}
		this->size = string.size();
		memcpy(this->buffer, string.c_str(), this->size);
	}
	void operator=(const LinkedList<byte>& list) {
		if (this->buffer) {
			if (this->size != list.length()) {
				delete[] this->buffer;
				this->buffer = new byte[list.length()];
			}
		} else {
			this->buffer = new byte[list.length()];
		}
		this->size = list.length();
		memcpy(this->buffer, list.data(), this->size);
	}
	
	ByteString& operator+=(const ByteString& bs)
	{
		if (bs.empty()) return *this;
		else if (this->empty()) {
			*this = bs;
			return *this;
		}
		byte *buff = new byte[this->size + bs.size];
		memcpy(buff, this->buffer, this->size);
		memcpy(buff+this->size, bs.buffer, bs.size);
		delete[] this->buffer;
		this->size += bs.size;
		this->buffer = new byte[this->size];
		memcpy(this->buffer, buff, this->size);
		delete[] buff;
		return *this;
	}
	ByteString& operator+=(const char* string) {
		if (!string) return *this;
		else if (this->empty()) {
			*this = ByteString(string);
			return *this;
		}
		int sz = strlen(string);
		byte *buff = new byte[this->size + sz];
		memcpy(buff, this->buffer, this->size);
		memcpy(buff+this->size, string, sz);
		delete[] this->buffer;
		this->size += sz;
		this->buffer = new byte[this->size];
		memcpy(this->buffer, buff, this->size);
		delete[] buff;
		return *this;
	}
	ByteString& operator+=(std::string& string) {
		*this += string.c_str();
		return *this;
	}
	ByteString& operator+=(const byte& b) {
		if (this->empty()) {
			*this = ByteString(b);
			return *this;
		}
		byte *buff = new byte[this->size + 1];
		memcpy(buff, this->buffer, this->size);
		buff[this->size] = b;
		delete[] this->buffer;
		this->size++;
		this->buffer = new byte[this->size];
		memcpy(this->buffer, buff, this->size);
		delete[] buff;
		return *this;
	}

	ByteString operator+(const ByteString& bs)
	{
		ByteString bs_new = *this;
		bs_new += bs;
		return bs_new;
	}
	ByteString operator+(const char* string) {
		ByteString bs = *this;
		bs += string;
		return bs;
	}
	ByteString operator+(std::string& string) {
		ByteString bs = *this;
		bs += string;
		return bs;
	}
	ByteString operator+(const byte& b) {
		ByteString bs = *this;
		bs += b;
		return bs;
	}

	ByteString& operator*=(int n) {
		if (n <= 0) {
			this->clear();
			return *this;
		}
		ByteString s(*this);
		while (--n > 0) *this += s;
		return *this;
	}
	ByteString& operator*=(size_t n) {
		if (n == 0) {
			this->clear();
			return *this;
		}
		ByteString s(*this);
		while (--n > 0) *this += s;
		return *this;
	}
	
	ByteString operator*(int n) {
		if (n <= 0) return ByteString();
		ByteString s(*this);
		while (--n > 0) s += *this;
		return s;
	}
	ByteString operator*(size_t n) {
		if (n == 0) return ByteString();
		ByteString s(*this);
		while (--n > 0) s += *this;
		return s;
	}

	inline bool operator==(const ByteString& bs) const {
		return ((this->size == bs.size) && (memcmp(this->buffer, bs.buffer, this->size) == 0));
	}
	inline bool operator==(const char* str) const {
		return ((this->size == strlen(str)) && (memcmp(this->buffer, str, this->size) == 0));
	}
	inline bool operator==(std::string& str) const {
		return ((this->size == (int)str.size()) && (memcmp(this->buffer, str.c_str(), this->size) == 0));
	}
	inline bool operator!=(const ByteString& bs)const { return !operator==(bs); }

	friend std::ostream& operator<<(std::ostream &output, const ByteString& bs)
	{
		if (!bs.empty())
			for (int i = 0; i < bs.length(); i++)
				output << bs.buffer[i];
		return output;
	}
	friend std::istream& operator>>(std::istream& input, ByteString& bs) {
		std::string str;
		while (std::getline(input, str)) {
			if (str.empty()) break;
			bs += str;
		}
		return input;
	}
};

struct compare {
	bool operator()(const ByteString& bs1, const ByteString& bs2) {
		if (bs1.length() > bs2.length()) return true;
		if (bs1.length() < bs2.length()) return false;
		return std::lexicographical_compare(bs1.data(), bs1.data()+bs1.length(),
			bs2.data(), bs2.data()+bs2.length(), [](byte a, byte b){return a > b;});
	}
};