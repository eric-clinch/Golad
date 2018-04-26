
#ifndef UTILITYHEAP_h
#define UTILITYHEAP_h

#include <vector>
#include <assert.h>
#include "UtilityNode.h"

using namespace std;

template <class T> class UtilityHeap {
private:
	vector<UtilityNode<T>*> heap;

	static inline int getParentIndex(int index) {
		return (index - 1) / 2;
	}

	static inline int getChildIndex(int index) {
		return index * 2 + 1;
	}

	void print() {
		for (int i = 0; i < size(); i++) {
			cerr << heap[i]->getAverageUtility() << " ";
		}
		cerr << "\n";
	}

	void heapifyUp(int index) {
		assert(index >= 0 && index < size());
		UtilityNode<T> *currentElement = heap[index];
		int parentIndex = getParentIndex(index);
		UtilityNode<T> *parentElement = heap[parentIndex];
		while (currentElement->getAverageUtility() > parentElement->getAverageUtility()) {
			heap[index] = parentElement;
			heap[parentIndex] = currentElement;

			index = parentIndex;
			parentIndex = getParentIndex(index);
			parentElement = heap[parentIndex];
		}
		assert(isHeap());
	}

	void heapifyDown(int index) {
		assert(index >= 0 && index < size());
		int leftChildIndex = getChildIndex(index);
		int rightChildIndex = leftChildIndex + 1;
		int largest = index;
		int heapSize = heap.size();
		if (leftChildIndex < heapSize && heap[leftChildIndex]->getAverageUtility() > heap[largest]->getAverageUtility()) {
			largest = leftChildIndex;
		}
		if (rightChildIndex < heapSize && heap[rightChildIndex]->getAverageUtility() > heap[largest]->getAverageUtility()) {
			largest = rightChildIndex;
		}
		if (largest != index) {
			UtilityNode<T> *temp = heap[index];
			heap[index] = heap[largest];
			heap[largest] = temp;
			heapifyDown(largest);
		}
		assert(isHeap());
	}

	bool isHeap(int index = 0) {
		if (index >= size()) return true;
		int leftChildIndex = getChildIndex(index);
		int rightChildIndex = leftChildIndex + 1;
		if (leftChildIndex < size() && heap[leftChildIndex]->getAverageUtility() > heap[index]->getAverageUtility()) {
			return false;
		}
		if (rightChildIndex < size() && heap[rightChildIndex]->getAverageUtility() > heap[index]->getAverageUtility()) {
			return false;
		}
		return isHeap(leftChildIndex) && isHeap(rightChildIndex);
	}

public:
	UtilityHeap(vector<UtilityNode<T>*> &v) {
		for (int i = 0; i < v.size(); i++) {
			push(v[i]);
		}
		assert(isHeap());
	}

	UtilityHeap() {
		assert(isHeap());
	}

	~UtilityHeap() {}

	UtilityNode<T> *peak(int index = 0) {
		assert(index >= 0 && index < size());
		if (!isHeap()) {
			print();
			cerr << "LOL\n";
		}
		assert(isHeap());
		return heap[index];
	}

	UtilityNode<T> *pop() {
		assert(!this->empty());
		assert(isHeap());
		UtilityNode<T> *res = heap[0];
		heap[0] = heap.back();
		heap.pop_back();
		if (!heap.empty()) heapifyDown(0);
		assert(isHeap());
		return res;
	}

	UtilityNode<T> *pop(int index) {
		assert(index >= 0 && index < size());
		assert(isHeap());
		UtilityNode<T> *res = heap[index];
		UtilityNode<T> *back = heap.back();
		heap[index] = back;
		heap.pop_back();

		int parentIndex = getParentIndex(index);
		if (index < heap.size()) {
			if (back->getAverageUtility() > heap[parentIndex]->getAverageUtility()) {
				heapifyUp(index);
			}
			else {
				heapifyDown(index);
			}
		}

		assert(isHeap());
		return res;
	}

	void push(UtilityNode<T> *element) {
		assert(isHeap());
		heap.push_back(element);
		heapifyUp(heap.size() - 1);
		assert(isHeap());
	}

	inline bool empty() {
		return heap.empty();
	}

	inline int size() {
		return heap.size();
	}
};

#endif