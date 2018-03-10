#ifndef _DEBUG
#define NDEBUG
#endif

#ifndef UTILITYNODE_h
#define UTILITYNODE_h

#include <assert.h>

template<class T> class UtilityNode{
private:
	float totalUtility;

public:
	int numTrials;
	T object;

	UtilityNode(T object) {
		this->object = object;
		this->numTrials = 0;
		this->totalUtility = 0;
	}

	float getAverageUtility() {
		assert(numTrials > 0);
		return totalUtility / numTrials;
	}

	void updateUtility(float trialUtility) {
		totalUtility += trialUtility;
		numTrials += 1;
	}
};

#endif