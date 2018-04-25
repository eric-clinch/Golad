#ifndef _DEBUG
#define NDEBUG
#endif

#ifndef UTILITYNODE_h
#define UTILITYNODE_h

#include <assert.h>

using namespace std;

template<class T> class UtilityNode{
private:

public:
	float totalUtility;
	T object;
	unsigned short numTrials;

	UtilityNode() {};

	UtilityNode(T &object) {
		this->object = object;
		this->numTrials = 0;
		this->totalUtility = 0;
	}

	float getAverageUtility() {
		if (numTrials == 0) return numeric_limits<float>::infinity();
		return totalUtility / numTrials;
	}

	void updateUtility(float trialUtility) {
		totalUtility += trialUtility;
		numTrials += 1;
	}

	void repurpose(T &object) {
		this->object = object;
		this->numTrials = 0;
		this->totalUtility = 0;
	}
};

#endif