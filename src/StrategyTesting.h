
#ifndef STRATEGYTESTING_h
#define STRATEGYTESTING_h

#include <random>
#include "Bot.h"
#include "Tools.h"
#include "Stats.h"
#include "UtilityNode.h"
#include "MAB.h"
#include "UCB1.h"
#include "EpsilonGreedy.h"
#include "Evaluator.h"
#include "RatioEvaluator.h"

namespace StrategyTesting {
	int playMatch(Bot bot0, Bot bot1, bool verbose = false);
	void playTournament(Bot bot0, Bot bot1, int rounds = 100, bool verbose = false);
	void optimizeParameters(int rounds);
}

int StrategyTesting::playMatch(Bot bot0, Bot bot1, bool verbose) {
	Board board(18, 16);
	board.initiateBoardPositions();
	bot0.SetBoard(&board);
	bot1.SetBoard(&board);
	bot0.SetYourBotId(P0);
	bot1.SetYourBotId(P1);

	int bot0Time = 10000;
	int bot1Time = 10000;
	int timePerMove = 170; // 170
	bot0.SetTimePerMove(timePerMove);
	bot1.SetTimePerMove(timePerMove);

	for (int round = 0; round < 50 && bot0Time >= 0 && bot1Time >= 0; round++) {
		if (verbose) cout << board.toString();
		int P0Count = board.getPlayerCellCount(P0);
		int P1Count = board.getPlayerCellCount(P1);
		if (P0Count == 0 && P1Count == 0) return -1; // tie
		else if (P0Count == 0) return 1;
		else if (P1Count == 0) return 0;
		Board *boardCopy = board.getCopy();

		// player 0's turn
		bot0Time = min(bot0Time + timePerMove, 10000);
		if (verbose) cout << "bot 0 time: " << bot0Time << "\n";
		long startTime = Tools::get_time();
		Move move = bot0.GetMove(bot0Time, round);
		assert(board == *boardCopy);
		int dt = Tools::get_time() - startTime;
		bot0Time -= dt;
		if (verbose) cout << "Player 0's move: " << move.toString() << " time to compute: " << dt << "\n\n";
		board.makeMoveOnBoard(move, P0);

		delete boardCopy;
		if (verbose) cout << board.toString();
		P0Count = board.getPlayerCellCount(P0);
		P1Count = board.getPlayerCellCount(P1);
		if (P0Count == 0 && P1Count == 0) return -1; // tie
		else if (P0Count == 0) return 1;
		else if (P1Count == 0) return 0;
		boardCopy = board.getCopy();

		// player 1's turn
		bot1Time = min(bot1Time + timePerMove, 10000);
		if (verbose) cout << "bot 1 time: " << bot1Time << "\n";
		startTime = Tools::get_time();
		move = bot1.GetMove(bot1Time, round);
		assert(board == *boardCopy);
		dt = Tools::get_time() - startTime;
		bot1Time -= dt;
		if (verbose) cout << "Player 1's move: " << move.toString() << " time to compute: " << dt << "\n\n";
		board.makeMoveOnBoard(move, P1);

		delete boardCopy;
	}

	if (bot0Time < 0) {
		cout << "bot 0 ran out of time\n";
		return 1;
	}
	else if (bot1Time < 0) {
		cout << "bot 1 ran out of time\n";
		return 0;
	}

	int P0Count = board.getPlayerCellCount(P0);
	int P1Count = board.getPlayerCellCount(P1);
	if (P0Count == P1Count) return -1; // tie
	return P0Count > P1Count ? 0 : 1;
}

void StrategyTesting::playTournament(Bot bot0, Bot bot1, int rounds, bool verbose) {
	int bot0Wins = 0;
	int bot1Wins = 0;

	for (int currentRound = 1; currentRound <= rounds; currentRound++) {
		int matchResult;
		if (currentRound % 2 == 1) {
			matchResult = StrategyTesting::playMatch(bot0, bot1, verbose);
		}
		else {
			matchResult = 1 - StrategyTesting::playMatch(bot1, bot0, verbose);
			if (matchResult == 2) matchResult = -1; // ties should be -1
		}
		if (matchResult == 0) bot0Wins++;
		else if (matchResult == 1) bot1Wins++;
		cout << "match winner: " << matchResult << "\n";
		if (currentRound % 10 == 0) {
			int ties = currentRound - bot0Wins - bot1Wins;
			cout << "bot 0 wins: " << bot0Wins << "\nbot 1 wins: " << bot1Wins << "\nties: " << ties << "\n";
			int numSamples = currentRound - ties;
			if (numSamples > 30) { // if the Central Limit Theorem Applies
				double probabilityBot1IsBetter = Stats::getProbabilitySuccessfulBernoulli(bot1Wins, numSamples);
				double probabilityBot0IsBetter = 1 - probabilityBot1IsBetter;
				cout << "probability that bot 0 is better: " << 100 * probabilityBot0IsBetter << "%\n";
				cout << "probability that bot 1 is better: " << 100 * probabilityBot1IsBetter << "%\n";
				if (probabilityBot0IsBetter > .995 || probabilityBot1IsBetter > .995) break; // the tournament is complete at this point
			}
		}
	}
}

struct CMABParameters {
	UtilityNode<MAB<Move>*> *moveMAB;
	UtilityNode<MAB<Coordinate>*> *coordinateMAB;
	UtilityNode<float> *greed;

	CMABParameters() {};

	CMABParameters(UtilityNode<MAB<Move>*> *m, UtilityNode<MAB<Coordinate>*> *c, UtilityNode<float> *g){
		moveMAB = m;
		coordinateMAB = c;
		greed = g;
	}

	void updateUtilities(float utility) {
		moveMAB->updateUtility(utility);
		coordinateMAB->updateUtility(utility);
		greed->updateUtility(utility);
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

UtilityNode<CMABParameters> *getParameters(vector<UtilityNode<MAB<Move>*>*> &moveMABs, vector<UtilityNode<MAB<Coordinate>*>*> &coordinateMABs,
	vector<UtilityNode<float>*> &greeds, vector<UtilityNode<CMABParameters>*> &parameters, default_random_engine &generator, float exploration, int round) {
	
	uniform_real_distribution<float> realRandom(0.0, 1.0);
	float rand = realRandom(generator);
	if (parameters.size() == 0 || rand < exploration) {
		random_shuffle(moveMABs.begin(), moveMABs.end());
		random_shuffle(coordinateMABs.begin(), coordinateMABs.end());
		random_shuffle(greeds.begin(), greeds.end());

		// explore case
		UCB1<MAB<Move>*> moveChooser(2);
		int moveMABIndex = moveChooser.getChoice(moveMABs, round);
		UtilityNode<MAB<Move>*> *moveMABNode = moveMABs[moveMABIndex];

		UCB1<MAB<Coordinate>*> coordinateChooser(2);
		int coordinateMABIndex = coordinateChooser.getChoice(coordinateMABs, round);
		UtilityNode<MAB<Coordinate>*> *coordinateMABNode = coordinateMABs[coordinateMABIndex];

		UCB1<float> greedChooser(2);
		int greedIndex = greedChooser.getChoice(greeds, round);
		UtilityNode<float> *greedNode = greeds[greedIndex];

		CMABParameters resultParameters(moveMABNode, coordinateMABNode, greedNode);
		UtilityNode<CMABParameters> *result = new UtilityNode<CMABParameters>(resultParameters);\
		return result;
	}
	else {
		random_shuffle(parameters.begin(), parameters.end());

		UCB1<CMABParameters> parametersChooser(2);
		int parametersIndex = parametersChooser.getChoice(parameters, round);
		UtilityNode<CMABParameters> *result = parameters[parametersIndex];
		remove(parameters, parametersIndex); // remove to ensure this isn't picked twice

		return result;
	}
}

void printParameters(UtilityNode<CMABParameters> parametersNode) {
	CMABParameters parameters = parametersNode.object;
	cerr << parameters.moveMAB->object->toString() << " " << parameters.coordinateMAB->object->toString() << " " << parameters.greed->object 
		 << " trials: " << parametersNode.numTrials << " score: " << parametersNode.getAverageUtility() << "\n";
	cout << parameters.moveMAB->object->toString() << " " << parameters.coordinateMAB->object->toString() << " " << parameters.greed->object
		 << " trials: " << parametersNode.numTrials << " score: " << parametersNode.getAverageUtility() << "\n";
}

void CMABRound(vector<UtilityNode<MAB<Move>*>*> &moveMABs, vector<UtilityNode<MAB<Coordinate>*>*> &coordinateMABs, vector<UtilityNode<float>*> &greeds,
			   vector<UtilityNode<CMABParameters>*> &parameters, default_random_engine &generator, float exploration, int round) {
	UtilityNode<CMABParameters> *parametersNode0 = getParameters(moveMABs, coordinateMABs, greeds, parameters, generator, exploration, round);
	UtilityNode<CMABParameters> *parametersNode1 = getParameters(moveMABs, coordinateMABs, greeds, parameters, generator, exploration, round);

	CMABParameters &parameters0 = parametersNode0->object;
	CMABParameters &parameters1 = parametersNode1->object;
	
	RatioEvaluator *evaluator0 = new RatioEvaluator();
	CMABStrategy bot0Strategy(evaluator0, parameters0.moveMAB->object, parameters0.coordinateMAB->object, parameters0.greed->object);
	RatioEvaluator *evaluator1 = new RatioEvaluator();
	CMABStrategy bot1Strategy(evaluator1, parameters1.moveMAB->object, parameters1.coordinateMAB->object, parameters1.greed->object);

	Bot bot0(&bot0Strategy);
	Bot bot1(&bot1Strategy);

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

	parameters.push_back(parametersNode0);
	parameters.push_back(parametersNode1);

	printParameters(*parametersNode0);
	printParameters(*parametersNode1);
	cerr << "\n";
	cout << "\n";
}

template <class T> bool UtilityNodeComparator(UtilityNode<T> *x, UtilityNode<T> *y) { return x->numTrials > y->numTrials; }

void StrategyTesting::optimizeParameters(int rounds) {
	vector<UtilityNode<MAB<Move>*>*> moveMABs;
	for (float epsilon = 0.1; epsilon < 0.95; epsilon += 0.1) {
		MAB<Move> *moveMAB = new EpsilonGreedy<Move>(epsilon);
		UtilityNode<MAB<Move>*> *moveMABNode = new UtilityNode<MAB<Move>*>(moveMAB);
		moveMABs.push_back(moveMABNode);
	}
	for (float confidence = 1; confidence < 3.1; confidence += 0.5) {
		MAB<Move> *moveMAB = new UCB1<Move>(confidence);
		UtilityNode<MAB<Move>*> *moveMABNode = new UtilityNode<MAB<Move>*>(moveMAB);
		moveMABs.push_back(moveMABNode);
	}

	vector<UtilityNode<MAB<Coordinate>*>*> coordinateMABs;
	for (float epsilon = 0.1; epsilon < 0.95; epsilon += 0.1) {
		MAB<Coordinate> *coordinateMAB = new EpsilonGreedy<Coordinate>(epsilon);
		UtilityNode<MAB<Coordinate>*> *coordinateMABNode = new UtilityNode<MAB<Coordinate>*>(coordinateMAB);
		coordinateMABs.push_back(coordinateMABNode);
	}
	for (float confidence = 1; confidence < 3.1; confidence += 0.5) {
		MAB<Coordinate> *coordinateMAB = new UCB1<Coordinate>(confidence);
		UtilityNode<MAB<Coordinate>*> *coordinateMABNode = new UtilityNode<MAB<Coordinate>*>(coordinateMAB);
		coordinateMABs.push_back(coordinateMABNode);
	}

	vector<UtilityNode<float>*> greeds;
	for (float greed = 0.1; greed < 0.95; greed += 0.1) {
		UtilityNode<float> *greedNode = new UtilityNode<float>(greed);
		greeds.push_back(greedNode);
	}

	vector<UtilityNode<CMABParameters>*> parameters;
	default_random_engine generator;
	random_device rd;
	generator.seed(rd());

	float exploration = 0.99;
	float explorationDecay = 0.991;

	// round += 2 and exploration *= explorationDecay * explorationDecay because each CMABRound
	// does 2 trials
	for (int round = 1; round <= rounds; round += 2) {
		CMABRound(moveMABs, coordinateMABs, greeds, parameters, generator, exploration, round);
		exploration *= explorationDecay * explorationDecay;

		if (round % 10 == 0) {
			cerr << "round: " << round << " exploration: " << exploration << " strategies tested: " << parameters.size() << "\n";
			cout << "round: " << round << " exploration: " << exploration << " strategies tested: " << parameters.size() << "\n";
			sort(parameters.begin(), parameters.end(), UtilityNodeComparator<CMABParameters>);
			for (int i = 0; i < 5 && i < parameters.size(); i++) {
				printParameters(*parameters[i]);
			}
			cerr << "\n";
			cout << "\n";
		}
	}
}

#endif