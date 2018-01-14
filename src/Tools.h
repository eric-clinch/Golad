#ifndef TOOLS_H
#define TOOLS_H

#include <chrono>
#include <vector>

namespace Tools {
	inline long get_time();

	template <class T> inline T GetRandomElementFromVector(std::vector<T> &input);
	template <class T> inline T PopRandomElementFromVector(std::vector<T> &input);
}

inline long Tools::get_time() {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
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

#endif