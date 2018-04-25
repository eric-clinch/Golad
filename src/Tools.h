#ifndef TOOLS_H
#define TOOLS_H

#include <chrono>
#include <vector>
#include "UtilityNode.h"

namespace Tools {
	inline long get_time();
	template <class T> inline void swap(vector<T> &v, int i, int j);

	template <class T> inline T GetRandomElementFromVector(std::vector<T> &input);
	template <class T> inline T PopRandomElementFromVector(std::vector<T> &input);

	template <class T> bool UtilityNodePointerComparator(UtilityNode<T> *x, UtilityNode<T> *y);
	template <class T> bool UtilityNodeComparator(UtilityNode<T> x, UtilityNode<T> y);
}

inline long Tools::get_time() {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
}

template <class T> inline void Tools::swap(vector<T> &v, int i, int j) {
	assert(i >= 0 && i < v.size());
	assert(j >= 0 && j < v.size());
	if (i != j) {
		T temp = v[i];
		v[i] = v[j];
		v[j] = temp;
	}
}

template <class T> inline T Tools::GetRandomElementFromVector(std::vector<T> &input) {
	assert(input.size() > 0);
	random_shuffle(input.begin(), input.end());
	return input.back();
}

template <class T> inline T Tools::PopRandomElementFromVector(std::vector<T> &input) {
	assert(input.size() > 0);
	T result = GetRandomElementFromVector(input);
	input.pop_back();
	return result;
}

template <class T> bool Tools::UtilityNodePointerComparator(UtilityNode<T> *x, UtilityNode<T> *y) {
	if (x->numTrials == y->numTrials) return x->getAverageUtility() > y->getAverageUtility();
	else return x->numTrials > y->numTrials;
}

template <class T> bool Tools::UtilityNodeComparator(UtilityNode<T> x, UtilityNode<T> y) {
	if (x.numTrials == y.numTrials) return x.getAverageUtility() > y.getAverageUtility();
	else return x.numTrials > y.numTrials;
}

#endif