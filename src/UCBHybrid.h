
#ifndef UCBHybrid_h
#define UCBHybrid_h

#include <assert.h>
#include <math.h>
#include "MAB.h"

using namespace std;

template <class T> class UCBHybrid : public MAB<T> {
private:
	float confidenceConstant;
	float greed;
	default_random_engine generator;
	uniform_real_distribution<float> uniformRealDistribution;

public:
	UCBHybrid(float confidenceConstant, float greed) {
		assert(greed >= 0 && greed <= 1);
		this->confidenceConstant = confidenceConstant;
		this->greed = greed;
		this->uniformRealDistribution = uniform_real_distribution<float>(0.0, 1.0);
	}

	int getChoice(vector<UtilityNode<T>> &nodes, int numTrials) {
		int numNodes = nodes.size();
		assert(numNodes > 0);
		if (uniformRealDistribution(generator) < greed) { // greedy case
			int result = 0;
			UtilityNode<T> node = nodes[result];
			float maxUtility = node.getAverageUtility();

			for (int i = 1; i < numNodes; i++) {
				UtilityNode<T> node = nodes[i];
				float utility = node.getAverageUtility();
				if (utility > maxUtility) {
					result = i;
					maxUtility = utility;
				}
			}
			return result;
		}
		else {
			float confidenceNumerator = log(numTrials) * confidenceConstant;
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
	}

	int getChoice(vector<UtilityNode<T>*> &nodes, int numTrials) {
		int numNodes = nodes.size();
		assert(numNodes > 0);
		if (uniformRealDistribution(generator) < greed) { // greedy case
			int result = 0;
			UtilityNode<T> *node = nodes[result];
			float maxUtility = node->getAverageUtility();

			for (int i = 1; i < numNodes; i++) {
				UtilityNode<T> *node = nodes[i];
				float utility = node->getAverageUtility();
				if (utility > maxUtility) {
					result = i;
					maxUtility = utility;
				}
			}
			return result;
		}
		else {
			float confidenceNumerator = log(numTrials) * confidenceConstant;
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
	}

	int getChoice(UtilityHeap<T> &heap, int numTrials) {
		int numNodes = heap.size();
		assert(numNodes > 0);
		if (uniformRealDistribution(generator) < greed) { // greedy case
			return 0; // the highest priority node in the heap is at index 0
		}
		else {
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
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "UCBHybrid(" << confidenceConstant << ", " << greed << ")";
		return stringStream.str();
	}
};

#endif