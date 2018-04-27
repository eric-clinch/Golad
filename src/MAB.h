
#ifndef MAB_h
#define MAB_h

#include "UtilityNode.h"
#include "UtilityHeap.h"

template <class T> class MAB {
public:
	// returns the index of the chosen element
	virtual int getChoice(vector<UtilityNode<T>> &nodes, int numTrials) = 0;
	virtual int getChoice(vector<UtilityNode<T>*> &nodes, int numTrials) = 0;
	virtual int getChoice(UtilityHeap<T> &heap, int numTrials) = 0;
	virtual string toString() = 0;
};

#endif