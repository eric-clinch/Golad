
#ifndef EPSILONGREEDY_h
#define EPSILONGREEDY_h

#include <assert.h>
#include <random>
#include "MAB.h"

using namespace std;

// a MAB strategy that chooses greedily (picks the node with the highest expected utility) with probability epsilon
// and picks randomly with probability 1 - epsilon

template <class T> class EpsilonGreedy : public MAB<T> {
private:
	float epsilon;
	default_random_engine generator;
	uniform_real_distribution<float> uniformRealDistribution;

public:
	EpsilonGreedy(float epsilon) {
		assert(epsilon >= 0 && epsilon <= 1);
		this->epsilon = epsilon;
		this->uniformRealDistribution = uniform_real_distribution<float>(0.0, 1.0);
	}

	int getChoice(vector<UtilityNode<T>> &nodes, int numTrials) {
		int numNodes = nodes.size();
		assert(numNodes > 0);
		if (uniformRealDistribution(generator) < epsilon) { // greedy case
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
		else { // random case
			uniform_int_distribution<int> uniformIntDistribution(0, numNodes - 1);
			int randomIndex = uniformIntDistribution(generator);
			return randomIndex;
		}
	}

	int getChoice(vector<UtilityNode<T>*> &nodes, int numTrials) {
		int numNodes = nodes.size();
		assert(numNodes > 0);
		if (uniformRealDistribution(generator) < epsilon) { // greedy case
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
		else { // random case
			uniform_int_distribution<int> uniformIntDistribution(0, numNodes - 1);
			int randomIndex = uniformIntDistribution(generator);
			return randomIndex;
		}
	}

	int getChoice(UtilityHeap<T> &heap, int numTrials) {
		int numNodes = heap.size();
		assert(numNodes > 0);
		if (uniformRealDistribution(generator) < epsilon) { // greedy case
			return 0; // the highest priority node in the heap is at index 0
		}
		else { // random case
			uniform_int_distribution<int> uniformIntDistribution(0, numNodes - 1);
			int randomIndex = uniformIntDistribution(generator);
			return randomIndex;
		}
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "EpsilonGreedy(" << epsilon << ")";
		return stringStream.str();
	}
};

#endif