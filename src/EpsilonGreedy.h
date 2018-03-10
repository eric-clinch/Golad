
#ifndef EPSILONGREEDY_h
#define EPSIILONGREEDY_h

#include <assert.h>
#include <random>
#include "MAB.h"

using namespace std;

// a MAB strategy that chooses greedily (picks the node with the highest expected utility) with probability epsilon
// and picks randomly with probability 1 - epsilon

template <class T> class EpsilonGreedy : public MAB {
private:
	float epsilon;
	default_random_engine generator;
	uniform_real_distribution<float> uniformRealDistribution(0.0, 1.0);

public:
	EpsilonGreedy(float epsilon) {
		assert(epsilon >= 0 && epsilon <= 1);
		this->epsilon = epsilon;
	}

	UtilityNode<T> getChoice(vector<UtilityNode<T>> nodes) {
		int numNodes = nodes.size();
		assert(numNodes > 0);
		if (uniformRealDistribution(generator) < epsilon) { // greedy case
			UtilityNode<T> result = nodes[0];
			float maxUtility = result.getAverageUtility();

			for (int i = 1; i < numNodes; i++) {
				UtilityNode<T> node = nodes[i];
				float utility = node.getAverageUtility();
				if (node.getAverageUtility() > maxUtility) {
					result = node;
					maxUtility = utility;
				}
			}
			return result;
		}
		else { // random case
			uniform_int_distribution<int> uniformIntDistribution(0, numNodes - 1);
			int randomIndex = uniformIntDistribution(generator);
			return nodes[randomIndex];
		}
	}
};

#endif