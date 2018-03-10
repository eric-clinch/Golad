
#ifndef MAB_h
#define MAB_h

#include "UtilityNode.h"

template <class T> class MAB {
public:
	virtual UtilityNode<T> getChoice(vector<UtilityNode<T>> nodes) = 0;
};

#endif