
#ifndef UCB1_h
#define UCB1_h

#include <assert.h>
#include <math.h>
#include "MAB.h"

using namespace std;

template <class T> class UCB1 : public MAB<T> {
private:
	float confidenceConstant;

public:
	UCB1(float confidenceConstant) {
		this->confidenceConstant = confidenceConstant;
	}

	int getChoice(vector<UtilityNode<T>> &nodes, int numTrials) {
		float confidenceNumerator = log(numTrials) * confidenceConstant;
		assert(nodes.size() > 0);
		int bestNodeIndex = 0;
		float bestScore = nodes[0].getAverageUtility() + sqrt(confidenceNumerator / nodes[0].numTrials);
		for (int i = 1; i < nodes.size(); i++) {
			float nodeScore = nodes[i].getAverageUtility() + sqrt(confidenceNumerator / nodes[i].numTrials);
			if (nodeScore > bestScore) {
				bestScore = nodeScore;
				bestNodeIndex = i;
			}
		}
		return bestNodeIndex;
	}

	int getChoice(vector<UtilityNode<T>*> &nodes, int numTrials) {
		float confidenceNumerator = log(numTrials) * confidenceConstant;
		assert(nodes.size() > 0);
		int bestNodeIndex = 0;
		float bestScore = nodes[0]->getAverageUtility() + sqrt(confidenceNumerator / nodes[0]->numTrials);
		for (int i = 1; i < nodes.size(); i++) {
			float nodeScore = nodes[i]->getAverageUtility() + sqrt(confidenceNumerator / nodes[i]->numTrials);
			if (nodeScore > bestScore) {
				bestScore = nodeScore;
				bestNodeIndex = i;
			}
		}
		return bestNodeIndex;
	}

	int getChoice(UtilityHeap<T> &heap, int numTrials) {
		float confidenceNumerator = log(numTrials) * confidenceConstant;
		int bestNodeIndex = 0;
		UtilityNode<T> node = heap.peak(0);
		float bestScore = node.getAverageUtility() + sqrt(confidenceNumerator / node.numTrials);
		for (int i = 1; i < heap.size(); i++) {
			node = heap.peak(i);
			float nodeScore = node.getAverageUtility() + sqrt(confidenceNumerator / node.numTrials);
			if (nodeScore > bestScore) {
				bestScore = nodeScore;
				bestNodeIndex = i;
			}
		}
		return bestNodeIndex;
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "UCB1(" << confidenceConstant << ")";
		return stringStream.str();
	}
};

#endif