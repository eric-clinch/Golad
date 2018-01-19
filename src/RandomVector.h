#ifndef _DEBUG
	#define NDEBUG
#endif

#ifndef RANDOMVECTOR_h
#define RANDOMVECTOR_h

template <class T> class RandomVector {
private:
	vector<T> currentVector;
	vector<T> nextVector;

public:
	RandomVector<T>(vector<T> v);

	T pop_back();
	void push_back(T &t);

	size_t size();
};

template <class T> RandomVector<T>::RandomVector(vector<T> v) {
	currentVector = v;
	nextVector = vector<T>();

	random_shuffle(currentVector.begin(), currentVector.end());
}

template <class T> T RandomVector<T>::pop_back() {
	assert(currentVector.size() > 0);
	T result = currentVector.back();
	currentVector.pop_back();
	
	if (currentVector.size() == 0) {
		currentVector = nextVector;
		nextVector = vector<T>();
		random_shuffle(currentVector.begin(), currentVector.end());
	}
	return result;
}

template <class T> void RandomVector<T>::push_back(T &t) {
	nextVector.push_back(t);

	if (currentVector.size() == 0) {
		currentVector = nextVector;
		nextVector = vector<T>();
		random_shuffle(currentVector.begin(), currentVector.end());
	}
}

template <class T> size_t RandomVector<T>::size() {
	return currentVector.size() + nextVector.size();
};

#endif