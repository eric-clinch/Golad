
#ifdef _DEBUG
	#include <stdlib.h>
	#include <crtdbg.h>
	#define _CRTDBG_MAP_ALLOC
	#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
#else
	#define _CrtDumpMemoryLeaks() {}
	#define NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <assert.h>
#include <math.h>
#include <random>
#include "Parser.h"
#include "Bot.h"
#include "Strategy.h"
#include "CMABStrategy.h"
#include "MoveComponents.h"
#include "Evaluator.h"
#include "DistanceEvaluator.h"
#include "MAB.h"
#include "EpsilonGreedy.h"
#include "UCBHybrid.h"
#include "Board.h"
#include "Coordinate.h"
#include "Move.h"
#include "StrategyTesting.h"
#include "ParameterOptimization.h"

using namespace std;

void test() {

}

void play() {
	Evaluator *evaluator = new DistanceEvaluator(0.5, 1.0);
	MAB<MoveComponents> *moveMAB0 = new UCBHybrid<MoveComponents>(4.5, 0.8);
	MAB<Coordinate> *coordinateMAB0 = new EpsilonGreedy<Coordinate>(0.6);
	CMABStrategy strategy(evaluator, moveMAB0, coordinateMAB0, 0.5, 0.000251328, 64, 128);
	Bot myBot = Bot(&strategy);
	Parser parser = Parser(myBot);
	parser.Parse();
}

void playTest() {
	Evaluator *evaluator0 = new DistanceEvaluator(0.5, 3.0);
	UCBHybrid<MoveComponents> *moveMAB0 = new UCBHybrid<MoveComponents>(4.5, 0.8);
	EpsilonGreedy<Coordinate> *coordinateMAB0 = new EpsilonGreedy<Coordinate>(0.6);
	CMABStrategy bot0Strategy(evaluator0, moveMAB0, coordinateMAB0, 0.5, 0.000251328, 64, 128);

	Bot bot0 = Bot(&bot0Strategy);
	
	Evaluator *evaluator1 = new DistanceEvaluator(0.5, 1.0);
	UCBHybrid<MoveComponents> *moveMAB1 = new UCBHybrid<MoveComponents>(4.5, 0.8);
	EpsilonGreedy<Coordinate> *coordinateMAB1 = new EpsilonGreedy<Coordinate>(0.6);
	CMABStrategy bot1Strategy(evaluator1, moveMAB1, coordinateMAB1, 0.5, 0.000251328, 64, 128);

	Bot bot1 = Bot(&bot1Strategy);

	StrategyTesting::playTournament(bot0, bot1, 1000);
	//StrategyTesting::playMatch(bot0, bot1, false);
}

int main() {
	// Initialize random number generator
	srand(time(NULL));

	//freopen("C:/Users/ericc/Desktop/golad/cerr_log.txt", "w", stderr);

	//playTest();
	//test();
	play();

	//ParameterOptimization::optimizeParameters(100000);

	delete Board::simulationLookupTable;
	_CrtDumpMemoryLeaks();
}