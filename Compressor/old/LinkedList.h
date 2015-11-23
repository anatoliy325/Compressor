#pragma once

#include <iostream>
#include <string>
#include <assert.h>

template <class T> class LinkedList {
public:
	struct _node {
		T value;
		_node *next;
	};
	
protected:
	struct _node *head, *tail;
	int size;

private:
	struct _node* _tail(struct _node* head2) {
		while (head2 && head2->next) head2 = head2->next;
		return head2;
	}
	struct _node* _part(struct _node* head2, struct _node* end, struct _node** nhead, struct _node** nend) {
		struct _node *pivot = end, *prev = NULL, *ptr = head2, *tail2 = end;
		while (ptr != pivot) {
			if (ptr->value < pivot->value) {
				if (*nhead == NULL) *nhead = ptr;
				prev = ptr;
				ptr = ptr->next;
			} else {
				if (prev) prev->next = ptr->next;
				struct _node *tmp = ptr->next;
				ptr->next = NULL;
				tail2->next = ptr;
				tail2 = ptr;
				ptr = tmp;
			}
		}
		if (!(*nhead)) *nhead = pivot;
		*nend = tail2;
		return pivot;
	}
	struct _node* _qsortr(struct _node* head2, struct _node* end) {
		if (!head2 || head2 == end) return head2;
		struct _node *nhead = NULL, *nend = NULL;
		struct _node *pivot = this->_part(head2, end, &nhead, &nend);
		if (nhead != pivot) {
			struct _node *tmp = nhead;
			while (tmp->next != pivot) tmp = tmp->next;
			tmp->next = NULL;
			nhead = this->_qsortr(nhead, tmp);
			tmp = this->_tail(nhead);
			tmp->next = pivot;
		}
		pivot->next = this->_qsortr(pivot->next, nend);
		return nhead;
	}

public:
	LinkedList<T>(): head(NULL), tail(NULL), size(0) {}
	LinkedList<T>(const T& value) {
		this->size = 0;
		this->pushback(value);
	}
	LinkedList<T>(const LinkedList<T>& list) {
		this->size = 0;
		this->pushback(list);
	}
	LinkedList<T>(const T* array, int size) {
		this->size = 0;
		this->pushback(array, size);
	}
	~LinkedList<T>() {
		this->clear();
	}

	const bool empty() const { return (!this->head || !this->tail || this->size <= 0); }
	const int length() const { return this->size; }
	
	const T* data() const {
		if (this->empty()) return NULL;
		T* buff = new T[this->size];
		int i = 0;
		struct _node *ptr = this->head;
		while (ptr && i < this->size) {
			buff[i] = ptr->value;
			ptr = ptr->next;
			i++;
		}
		return buff;
	}

	const _node* front() const { return (!this->empty()) ? this->head : NULL; }
	_node* back() { return (!this->empty()) ? this->tail : NULL; }

	void pushback(const T* array, int size) {
		if (!array || size <= 0) return;
		for (int i = 0; i < size; i++) this->pushback(array[i]);
	}
	void pushback(const LinkedList<T>& list) {
		if (list.empty()) return;
		struct _node *ptr = list.head;
		int i = 0, sz = list.size;
		while (ptr && i < sz) {
			this->pushback(ptr->value);
			ptr = ptr->next;
			i++;
		}
	}
	void pushback(const T& value) {
		if (this->empty()) {
			this->head = new _node();
			this->head->next = NULL;
			this->head->value = value;
			this->tail = this->head;
			this->size = 1;
		} else {
			struct _node *n = new _node;
			n->value = value;
			n->next = NULL;
			this->tail->next = n;
			this->tail = n;
			this->size++;
		}
	}

	void insert(const T& value, int index) {
		if (this->empty() || index < 0 || index > this->size) return;
		struct _node *newnode = new _node;
		newnode->value = value;
		if (index == 0) {
			if (!this->head) {
				newnode->next = NULL;
				this->head = newnode;
				this->tail = newnode;
			} else {
				newnode->next = this->head;
				head = newnode;
			}
			this->size++;
		} else if (index == 1) {
			struct _node *tmp = this->head->next;
			this->head->next = newnode;
			newnode->next = tmp;
			this->size++;
		} else if (index == this->size) {
			this->pushback(value);
		} else {
			int i = 0;
			struct _node *ptr = this->head, *prev = NULL;
			while (ptr && i < (index-1)) {
				if (i == index) break;
				ptr = ptr->next;
				prev = ptr;
				i++;
			}
			if (prev && ptr) {
				struct _node *tmp = prev->next;
				prev->next = newnode;
				newnode->next = tmp;
				this->size++;
			}
		}
	}
	void insert(const T& value, int offset, bool ascending) {
		int index = this->find_first(value, offset, ascending);
		if (index == -1) return;
		this->insert(value, index);
	}

	void remove(int index) {
		if (this->empty() || index < 0 || index >= this->size) return;
		struct _node *ptr = this->head, *prev = NULL;
		if (index == 0) {
			this->head = this->head->next;
			delete ptr;
		} else {
			int i = 0;
			while (ptr) {
				if (i == index) break;
				prev = ptr;
				ptr = ptr->next;
				i++;
			}
			prev->next = ptr->next;
			delete ptr;
		}
		this->size--;
	}
	void remove(const T& value, int offset = 0) {
		int index = this->find(value, offset);
		while (index != -1) {
			this->remove(index);
			index = find(value, offset);
		}
	}

	void clear() {
		if (!this->head) return;
		struct _node *tmp = this->head;
		while (this->head) {
			this->head = this->head->next;
			delete tmp;
			tmp = this->head;
		}
		this->size = 0;
		this->head = NULL;
		this->tail = NULL;
	}

	int find(const T& value, int offset = 0) {
		if (this->empty()) return -1;
		int i = 0;
		struct _node *ptr = this->head;
		while (ptr) {
			if (i >= offset && ptr->value == value) return i;
			ptr = ptr->next;
			i++;
		}
		return -1;
	}
	int find_not(const T& value, int offset = 0) {
		if (this->empty()) return -1;
		int i = 0;
		struct _node *ptr = this->head;
		while (ptr) {
			if (i >= offset && ptr->value != value) return i;
			ptr = ptr->next;
			i++;
		}
		return -1;
	}
	int find_first(const T& value, int offset = 0, bool greater=true) {
		if (this->empty()) return -1;
		int i = 0;
		struct _node *ptr = this->head;
		while (ptr) {
			if (i >= offset && !(greater ^ (ptr->value > value))) return i;
			ptr = ptr->next;
			i++;
		}
		return -1;
	}

	int count(const T& value, int offset = 0) {
		if (this->empty()) return -1;
		int i = 0, cnt = 0;
		struct _node *ptr = this->head;
		while (ptr) {
			if (i >= offset && ptr->value == value) cnt++;
			ptr = ptr->next;
			i++;
		}
		return cnt;
	}

	const T min(int offset = 0) const {
		assert(!this->empty());
		int i = 0, minval = this->head->value;
		struct _node *ptr = this->head;
		while (ptr) {
			if (i >= offset && ptr->value < minval) minval = ptr->value;
			ptr = ptr->next;
			i++;
		}
		return minval;
	}
	const T max(int offset = 0) const {
		assert(!this->empty());
		int i = 0, maxval = this->head->value;
		struct _node *ptr = this->head;
		while (ptr) {
			if (i >= offset && ptr->value > maxval) maxval = ptr->value;
			ptr = ptr->next;
			i++;
		}
		return maxval;
	}

	void swap(int i, int j) {
		if (this->empty() || i < 0 || i >= this->size || j < 0 || j >= this->size || i == j) return;
		struct _node *ptr = this->head, *prev = NULL, *ti = NULL, *tiprev = NULL, *tj = NULL, *tjprev = NULL;
		int k = 0;
		while (ptr) {
			if (ti && tj) break;
			if (k == i) {
				ti = ptr;
				tiprev = prev;
			}
			if (k == j) {
				tj = ptr;
				tjprev = prev;
			}
			prev = ptr;
			ptr = ptr->next;
			k++;
		}
		if (ti && tj) {
			if (tiprev) tiprev->next = tj;
			if (tjprev) tjprev->next = ti;

			struct _node *tmp = ti->next;
			ti->next = tj->next;
			tj->next = tmp;

			if (i == 0) this->head = tj;
			if (j == 0) this->head = ti;
		}
	}

	void reverse() {
		if (this->empty() || this->size == 1) return;
		struct _node *ptr = this->head, *prev = NULL;
		while (ptr) {
			_node *tmp = ptr;
			ptr = ptr->next;
			tmp->next = prev;
			prev = tmp;
		}
		this->tail = this->head;
		this->head = prev;
	}

	void rotate(int positions) {
		if (this->empty() || positions % this->size == 0) return;
		positions = (positions < 0) ? (this->size + positions % this->size) : (positions % this->size);
		// make a circular list
		this->tail->next = this->head;
		// get the positions's node and make it as new head
		struct _node *ptr = this->head;
		int i = 0;
		while (ptr && i < positions-1) {
			ptr = ptr->next;
			i++;
		}
		// break the circular list at the position's node
		this->head = ptr->next;
		ptr->next = NULL;
		this->tail = ptr;
	}

	void qsort() {
		if (this->empty() || this->size == 1) return;
		this->head = this->_qsortr(this->head, this->tail);
		this->tail = this->_tail(this->head);
	}

	void to_front(int index) {
		if (this->empty() || index <= 0) return;
		int i = 0;
		struct _node *curr = this->head, *prev = NULL;
		while (curr) {
			if (i == index) break;
			prev = curr;
			curr = curr->next;
			i++;
		}
		if (curr && prev) {
			prev->next = curr->next;
			curr->next = head;
			this->head = curr;
		}
	}
	void to_front(const T& value, int offset) {
		int index = this->find(value, offset);
		if (index != -1) this->to_front(index);
	}

	const T& operator[](int index) const {
		if (this->empty() || index >= this->size) return NULL;
		struct _node *ptr = this->head;
		int i = 0;
		while (ptr) {
			if (i == index) break;
			ptr = ptr->next;
			i++;
		}
		return ptr->value;
	}
	T& operator[](int index) {
		assert(!this->empty() && index >= 0 && index < this->size);
		struct _node *ptr = this->head;
		int i = 0;
		while (ptr) {
			if (i == index) break;
			ptr = ptr->next;
			i++;
		}
		return ptr->value;
	}

	void operator=(const LinkedList<T>& list)
	{
		this->clear();
		this->pushback(list);
	}
	void operator=(const T& value)
	{
		this->clear();
		this->pushback(value);
	}

	LinkedList<T>& operator+=(const T& value) {
		this->pushback(value);
		return *this;
	}
	LinkedList<T>& operator+=(const LinkedList<T>& lst) {
		this->pushback(lst);		
		return *this;
	}
	LinkedList<T> operator+(const T& value)
	{
		LinkedList<T> lst = *this;
		lst += value;
		return lst;
	}
	LinkedList<T> operator+(const LinkedList<T>& lst)
	{
		LinkedList<T> tmp = *this;
		tmp += lst;
		return tmp;
	}

	inline bool operator==(const LinkedList<T>& list) const {
		if (this->empty()) return list.empty();
		if (this->size != list.size) return false;
		struct _node *t = this->head, *l = list.head;
		while (t && l) {
			if (t->value != l->value) return false;
			t = t->next;
			l = l->next;
		}
		return true;
	}
	inline bool operator!=(const LinkedList<T>& list)const { return !operator==(list); }

	friend std::ostream& operator<<(std::ostream &output, const LinkedList<T>& list)
	{
		if (!list.empty()) {
			struct _node *ptr = list.head;
			int i = 0;
			while (ptr && i < list.size) {
				output << (ptr->value);
				ptr = ptr->next;
				i++;
			}
		}
		return output;
	}
	friend std::istream& operator>>(std::istream& input, LinkedList<T>& list) {
		std::string str;
		while (std::getline(input, str)) {
			if (str.empty()) break;
			for (size_t i = 0; i < str.size(); i++)
				list.pushback(static_cast<T>(str[i]));
		}
		return input;
	}
};