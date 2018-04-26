#ifndef PARAMETEROPTIMIZATION_h
#define PARAMETEROPTIMIZATION_h

#include <math.h>
#include "Strategy.h"
#include "UtilityNode.h"
#include "MAB.h"
#include "EpsilonGreedy.h"
#include "UCB1.h"
#include "UCBHybrid.h"
#include "MoveComponents.h"
#include "StrategyTesting.h"
#include "CMABStrategy.h"
#include "CMABStrategy2.h"
#include "Evaluator.h"
#include "RatioEvaluator.h"

namespace ParameterOptimization {
	void optimizeParameters(int rounds);
}

struct AllParameters {
	vector<UtilityNode<MAB<MoveComponents>*>*> moveMABs;
	vector<UtilityNode<MAB<Coordinate>*>*> coordinateMABs;
	vector<UtilityNode<MAB<MoveComponents>*>*> secondaryMoveMabs;
	vector<UtilityNode<float>*> greeds;
	vector<UtilityNode<float>*> alphas;
	vector<UtilityNode<float>*> timeSplits;
	vector<UtilityNode<int>*> topMoveNums;

	AllParameters(vector<UtilityNode<MAB<MoveComponents>*>*> &moveMABs, vector<UtilityNode<MAB<Coordinate>*>*> &coordinateMABs, 
				  vector<UtilityNode<MAB<MoveComponents>*>*> &secondaryMoveMabs, vector<UtilityNode<float>*> &greeds, 
				  vector<UtilityNode<float>*> &alphas, vector<UtilityNode<float>*> timeSplits, vector<UtilityNode<int>*> topMoveNums) {
		this->moveMABs = moveMABs;
		this->coordinateMABs = coordinateMABs;
		this->secondaryMoveMabs = secondaryMoveMabs;
		this->greeds = greeds;
		this->alphas = alphas;
		this->timeSplits = timeSplits;
		this->topMoveNums = topMoveNums;
	}

	void shuffle() {
		random_shuffle(moveMABs.begin(), moveMABs.end());
		random_shuffle(coordinateMABs.begin(), coordinateMABs.end());
		random_shuffle(secondaryMoveMabs.begin(), secondaryMoveMabs.end());
		random_shuffle(alphas.begin(), alphas.end());
		random_shuffle(greeds.begin(), greeds.end());
		random_shuffle(timeSplits.begin(), timeSplits.end());
		random_shuffle(topMoveNums.begin(), topMoveNums.end());
	}
};

struct CMABParameters {
	UtilityNode<MAB<MoveComponents>*> *moveMAB;
	UtilityNode<MAB<Coordinate>*> *coordinateMAB;
	UtilityNode<MAB<MoveComponents>*> *secondaryMoveMAB;
	UtilityNode<float> *alpha;
	UtilityNode<float> *greed;
	UtilityNode<float> *timeSplit;
	UtilityNode<int> *topMoveNum;

	CMABParameters() {};

	CMABParameters(UtilityNode<MAB<MoveComponents>*> *m, UtilityNode<MAB<Coordinate>*> *c, UtilityNode<MAB<MoveComponents>*> *secondaryMoveMAB,
				   UtilityNode<float> *greed, UtilityNode<float> *alpha, UtilityNode<float> *timeSplit, UtilityNode<int> *topMoveNum) {
		this->moveMAB = m;
		this->coordinateMAB = c;
		this->secondaryMoveMAB = secondaryMoveMAB;
		this->greed = greed;
		this->alpha = alpha;
		this->timeSplit = timeSplit;
		this->topMoveNum = topMoveNum;
	}

	void updateUtilities(float utility) {
		moveMAB->updateUtility(utility);
		coordinateMAB->updateUtility(utility);
		secondaryMoveMAB->updateUtility(utility);
		alpha->updateUtility(utility);
		greed->updateUtility(utility);
		timeSplit->updateUtility(utility);
		topMoveNum->updateUtility(utility);
	}

	Strategy *getStrategy() {
		RatioEvaluator *evaluator = new RatioEvaluator();
		Strategy *strategy = new CMABStrategyParallel(evaluator, moveMAB->object, coordinateMAB->object, secondaryMoveMAB->object,
											   greed->object, alpha->object, timeSplit->object, topMoveNum->object);
		return strategy;
	}
};

// removes the element with index i from the given vector and returns
// the contents of the removed position in O(1).
// does not preserve ordering of the vector, should only be used on
// unordered vectors
template <class T> T remove(vector<T> &v, int i) {
	// moves the last element into the ith position, and then removes
	// the last element
	T res = v[i];
	v[i] = v.back();
	v.pop_back();
	return res;
}

UtilityNode<CMABParameters> *getParameters(AllParameters &possibleParameters, vector<UtilityNode<CMABParameters>*> &testedParameters, 
										   default_random_engine &generator, float exploration, int round) {

	uniform_real_distribution<float> realRandom(0.0, 1.0);
	float rand = realRandom(generator);
	if (testedParameters.size() == 0 || rand < exploration) {
		possibleParameters.shuffle();

		// explore case
		UCB1<MAB<MoveComponents>*> moveChooser(2);
		int moveMABIndex = moveChooser.getChoice(possibleParameters.moveMABs, round);
		UtilityNode<MAB<MoveComponents>*> *moveMABNode = possibleParameters.moveMABs[moveMABIndex];

		UCB1<MAB<Coordinate>*> coordinateChooser(2);
		int coordinateMABIndex = coordinateChooser.getChoice(possibleParameters.coordinateMABs, round);
		UtilityNode<MAB<Coordinate>*> *coordinateMABNode = possibleParameters.coordinateMABs[coordinateMABIndex];

		int secondaryMoveMABIndex = moveChooser.getChoice(possibleParameters.secondaryMoveMabs, round);
		UtilityNode<MAB<MoveComponents>*> *secondaryMoveMABNode = possibleParameters.secondaryMoveMabs[secondaryMoveMABIndex];

		UCB1<float> floatChooser(2);
		int greedNodeIndex = floatChooser.getChoice(possibleParameters.greeds, round);
		UtilityNode<float> *greedNode = possibleParameters.greeds[greedNodeIndex];

		int alphaNodeIndex = floatChooser.getChoice(possibleParameters.alphas, round);
		UtilityNode<float> *alphaNode = possibleParameters.alphas[alphaNodeIndex];

		int timeSplitIndex = floatChooser.getChoice(possibleParameters.timeSplits, round);
		UtilityNode<float> *timeSplitNode = possibleParameters.timeSplits[timeSplitIndex];

		UCB1<int> intChooser(2);
		int topMoveNumIndex = intChooser.getChoice(possibleParameters.topMoveNums, round);
		UtilityNode<int> *topMoveNumNode = possibleParameters.topMoveNums[topMoveNumIndex];

		CMABParameters resultParameters(moveMABNode, coordinateMABNode, secondaryMoveMABNode, greedNode, alphaNode, timeSplitNode, topMoveNumNode);
		UtilityNode<CMABParameters> *result = new UtilityNode<CMABParameters>(resultParameters);
		return result;
	}
	else {
		random_shuffle(testedParameters.begin(), testedParameters.end());

		UCB1<CMABParameters> parametersChooser(2);
		int parametersIndex = parametersChooser.getChoice(testedParameters, round);
		UtilityNode<CMABParameters> *result = testedParameters[parametersIndex];
		remove(testedParameters, parametersIndex); // remove to ensure this isn't picked twice
												   // it will be readded in the CMABRound function
		return result;
	}
}

void printParameters(UtilityNode<CMABParameters> parametersNode) {
	CMABParameters parameters = parametersNode.object;
	Strategy *strategy = parameters.getStrategy();
	cerr << strategy->toString() << " trials: " <<
			parametersNode.numTrials << " win rate: " << parametersNode.getAverageUtility() << "\n";
	cout << strategy->toString() << " trials: " <<
			parametersNode.numTrials << " win rate: " << parametersNode.getAverageUtility() << "\n";
	delete strategy;
}

void printBotInfo(UtilityNode<Bot> &botNode) {
	cerr << botNode.object.getStrategy()->toString() << " trials: " <<
		botNode.numTrials << " win rate: " << botNode.getAverageUtility() << "\n";
	cout << botNode.object.getStrategy()->toString() << " trials: " <<
		botNode.numTrials << " win rate: " << botNode.getAverageUtility() << "\n";
}

struct Bots {
	Bot bot0;
	Bot bot1;

	Bots(Bot &bot0, Bot &bot1) {
		this->bot0 = bot0;
		this->bot1 = bot1;
	}
};

void *parallelPlayMatch(void *args) {
	Bots *bots = (Bots*)args;
	int *res = new int();
	*res = StrategyTesting::playMatch(bots->bot0, bots->bot1);
	return res;
}

void evaluateParameters(UtilityNode<CMABParameters> *parametersNode0, UtilityNode<CMABParameters> *parametersNode1, 
						default_random_engine &generator) {
	CMABParameters &parameters0 = parametersNode0->object;
	CMABParameters &parameters1 = parametersNode1->object;

	Strategy *bot0Strategy = parameters0.getStrategy();
	Strategy *bot1Strategy = parameters1.getStrategy();

	Bot bot0(bot0Strategy);
	Bot bot1(bot1Strategy);

	int roundResult = StrategyTesting::playMatch(bot0, bot1);
	if (roundResult == -1) roundResult = 0.5;
	parametersNode0->updateUtility(1 - roundResult);
	parameters0.updateUtilities(1 - roundResult);
	parametersNode1->updateUtility(roundResult);
	parameters1.updateUtilities(roundResult);

	roundResult = StrategyTesting::playMatch(bot1, bot0);
	if (roundResult == -1) roundResult = 0.5;
	parametersNode0->updateUtility(roundResult);
	parameters0.updateUtilities(roundResult);
	parametersNode1->updateUtility(1 - roundResult);
	parameters1.updateUtilities(1 - roundResult);

	delete bot0Strategy;
	delete bot1Strategy;
}

void CMABRound(AllParameters &possibleParameters, vector<UtilityNode<CMABParameters>*> &parameters, default_random_engine &generator, 
			   float exploration, int round) {
	UtilityNode<CMABParameters> *parametersNode0 = getParameters(possibleParameters, parameters, generator, exploration, round);
	UtilityNode<CMABParameters> *parametersNode1 = getParameters(possibleParameters, parameters, generator, exploration, round);

	evaluateParameters(parametersNode0, parametersNode1, generator);

	parameters.push_back(parametersNode0);
	parameters.push_back(parametersNode1);

	printParameters(*parametersNode0);
	printParameters(*parametersNode1);
	cerr << "\n";
	cout << "\n";
}

double getExploration(int round) {
	return (float)1 / (1 + 0.015 * round);
}

void solveMAB(vector<UtilityNode<Bot>> &botNodes, int rounds) {
	UCB1<Bot> parametersChooser(2);

	for (int round = 0; round < rounds; round += 2) {
		random_shuffle(botNodes.begin(), botNodes.end());
		
		int parametersIndex = parametersChooser.getChoice(botNodes, round);
		UtilityNode<Bot> botNode0 = botNodes[parametersIndex];
		remove(botNodes, parametersIndex); // remove to ensure this isn't picked twice
		parametersIndex = parametersChooser.getChoice(botNodes, round);
		UtilityNode<Bot> botNode1 = botNodes[parametersIndex];
		remove(botNodes, parametersIndex);

		Bot bot0 = botNode0.object;
		Bot bot1 = botNode1.object;

		pthread_t thread;
		Bots bots(bot1, bot0);
		pthread_create(&thread, NULL, &parallelPlayMatch, (void*)&bots);

		int roundResult = StrategyTesting::playMatch(bot0, bot1);
		if (roundResult == -1) roundResult = 0.5;
		botNode0.updateUtility(1 - roundResult);
		botNode1.updateUtility(roundResult);

		int *roundResultPointer = new int();
		pthread_join(thread, (void**)&roundResultPointer);
		roundResult = *roundResultPointer;
		delete roundResultPointer;

		if (roundResult == -1) roundResult = 0.5;
		botNode0.updateUtility(roundResult);
		botNode1.updateUtility(1 - roundResult);

		botNodes.push_back(botNode0);
		botNodes.push_back(botNode1);
		printBotInfo(botNode0);
		printBotInfo(botNode1);
		cerr << "\n";
		cout << "\n";

		if (round % 20 == 0 && round > 0) {
			cerr << "round: " << round << "\n";
			cout << "round: " << round << "\n";
			sort(botNodes.begin(), botNodes.end(), Tools::UtilityNodeComparator<Bot>);
			for (int i = 0; i < botNodes.size(); i++) {
				printBotInfo(botNodes[i]);
			}
			cerr << "\n";
			cout << "\n";
		}
	}
}

double expectedExplored(int rounds, double alpha) {
	double result = 0;
	for (int i = 0; i < rounds; i++) {
		result += (float)1 / (1 + alpha * i);
	}
	return result;
}

double approximateAlpha(int rounds, int desiredExplored, int accuracy = 50) {
	double result = 0;

	for (int i = 0; i < accuracy; i++) {
		double newBit = 1;
		while (expectedExplored(rounds, result + newBit) < desiredExplored) {
			newBit /= 2;
		}
		result += newBit;
	}

	return result;
}

void ParameterOptimization::optimizeParameters(int rounds) {

	freopen("CMABParameters4.txt", "w", stderr);

	// CMAB Solver setup
	vector<UtilityNode<MAB<MoveComponents>*>*> moveMABs;
	for (float confidence = 1; confidence < 4.6; confidence += 0.5) {
		for (float epsilon = 0.4; epsilon < 1; epsilon += 0.1) {
			MAB<MoveComponents> *moveMAB = new UCBHybrid<MoveComponents>(confidence, epsilon);
			UtilityNode<MAB<MoveComponents>*> *moveMABNode = new UtilityNode<MAB<MoveComponents>*>(moveMAB);
			moveMABs.push_back(moveMABNode);
		}
	}

	vector<UtilityNode<MAB<Coordinate>*>*> coordinateMABs;
	for (float epsilon = 0.3; epsilon < 0.91; epsilon += 0.1) {
		MAB<Coordinate> *coordinateMAB = new EpsilonGreedy<Coordinate>(epsilon);
		UtilityNode<MAB<Coordinate>*> *coordinateMABNode = new UtilityNode<MAB<Coordinate>*>(coordinateMAB);
		coordinateMABs.push_back(coordinateMABNode);
	}

	vector<UtilityNode<MAB<MoveComponents>*>*> secondaryMoveMABs;
	for (float epsilon = 0.2; epsilon < 0.81; epsilon += 0.1) {
		MAB<MoveComponents> *moveMAB = new EpsilonGreedy<MoveComponents>(epsilon);
		UtilityNode<MAB<MoveComponents>*> *moveMABNode = new UtilityNode<MAB<MoveComponents>*>(moveMAB);
		secondaryMoveMABs.push_back(moveMABNode);
	}
	for (float confidence = 1; confidence < 4.6; confidence += 0.5) {
		for (float epsilon = 0.1; epsilon < 1; epsilon += 0.1) {
			MAB<MoveComponents> *moveMAB = new UCBHybrid<MoveComponents>(confidence, epsilon);
			UtilityNode<MAB<MoveComponents>*> *moveMABNode = new UtilityNode<MAB<MoveComponents>*>(moveMAB);
			secondaryMoveMABs.push_back(moveMABNode);
		}
	}
	for (float confidence = 1; confidence < 4.6; confidence += 0.5) {
		MAB<MoveComponents> *moveMAB = new UCB1<MoveComponents>(confidence);
		UtilityNode<MAB<MoveComponents>*> *moveMABNode = new UtilityNode<MAB<MoveComponents>*>(moveMAB);
		secondaryMoveMABs.push_back(moveMABNode);
	}

	vector<UtilityNode<float>*> greeds;
	for (float greed = 0.1; greed <= 0.9;  greed += 0.05) {
		UtilityNode<float> *greedNode = new UtilityNode<float>(greed);
		greeds.push_back(greedNode);
	}

	vector<UtilityNode<float>*> alphas;
	int expectedRounds = 22000;
	for (int desiredExplored = 3000; desiredExplored < 10000; desiredExplored += 500) {
		float alpha = approximateAlpha(expectedRounds, desiredExplored);
		UtilityNode<float> *alphaNode = new UtilityNode<float>(alpha);
		alphas.push_back(alphaNode);
	}

	vector<UtilityNode<float>*> timeSplits;
	for (float timeSplit = 0.1; timeSplit < 1.01; timeSplit += 0.1) {
		UtilityNode<float> *timeSplitNode = new UtilityNode<float>(timeSplit);
		timeSplits.push_back(timeSplitNode);
	}

	vector<UtilityNode<int>*> topMoveNums;
	for (int topMoveNum = 1; topMoveNum <= 32; topMoveNum *= 2) {
		UtilityNode<int> *topMoveNumNode = new UtilityNode<int>(topMoveNum);
		topMoveNums.push_back(topMoveNumNode);
	}

	AllParameters possibleParameters(moveMABs, coordinateMABs, secondaryMoveMABs, greeds, alphas, timeSplits, topMoveNums);
	vector<UtilityNode<CMABParameters>*> testedParameters;
	default_random_engine generator;
	random_device rd;
	generator.seed(rd());

	// round += 2 because each CMABRound does 2 trials
	for (int round = 0; round < rounds; round += 2) {
		float exploration = getExploration(round);
		CMABRound(possibleParameters, testedParameters, generator, exploration, round + 1); // for CMABRound, the round number should be 1-based indexed
		
		if (round % 20 == 0 && round > 0) {
			cerr << "round: " << round << " exploration: " << exploration << " strategies tested: " << testedParameters.size() << "\n";
			cout << "round: " << round << " exploration: " << exploration << " strategies tested: " << testedParameters.size() << "\n";
			sort(testedParameters.begin(), testedParameters.end(), Tools::UtilityNodePointerComparator<CMABParameters>);
			for (int i = 0; i < 5 && i < testedParameters.size(); i++) {
				printParameters(*testedParameters[i]);
			}
			cerr << "\n";
			cout << "\n";
		}
	}
}

#endif