
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
#include <cstdio>
#include <assert.h>
#include <math.h>
#include <random>
#include "Parser.h"
#include "Bot.h"
#include "Strategy.h"
#include "RandomStrategy.h"
#include "RandSearchStrategy.h"
#include "BirthRandSearch.h"
#include "BirthRandSearch2.h"
#include "BirthRandSearchKiller.h"
#include "CMABStrategy.h"
#include "Evaluator.h"
#include "RatioEvaluator.h"
#include "MAB.h"
#include "EpsilonGreedy.h"
#include "UCB1.h"
#include "Board.h"
#include "Coordinate.h"
#include "Move.h"
#include "Tools.h"
#include "StrategyTesting.h"
#include "Stats.h"

using namespace std;

void test() {
}

void play() {
	int bot0AdversarialTrials[] = { 600 };
	BirthRandSearch2 strategy = BirthRandSearch2(1, bot0AdversarialTrials);
	Bot myBot = Bot(&strategy);
	Parser parser = Parser(myBot);
	parser.Parse();
}

void playTest() {

	//int bot0AdversarialTrials[] = { 600 };
	//BirthRandSearch bot0Strategy = BirthRandSearch(1, bot0AdversarialTrials);

	RatioEvaluator *evaluator1 = new RatioEvaluator();
	//UCB1<Move> *moveMAB1 = new UCB1<Move>(2.0);
	//UCB1<Coordinate> *coordinateMAB1 = new UCB1<Coordinate>(2.0);
	EpsilonGreedy<Move> *moveMAB1 = new EpsilonGreedy<Move>(0.5);
	EpsilonGreedy<Coordinate> *coordinateMAB1 = new EpsilonGreedy<Coordinate>(0.5);
	CMABStrategy bot0Strategy(evaluator1, moveMAB1, coordinateMAB1, 0.5);

	Bot bot0 = Bot(&bot0Strategy);

	//int bot1AdversarialTrials[] = { 600 };
	//BirthRandSearch2 bot1Strategy = BirthRandSearch2(1, bot1AdversarialTrials);

	RatioEvaluator *evaluator2 = new RatioEvaluator();
	UCB1<Move> *moveMAB2 = new UCB1<Move>(2.0);
	UCB1<Coordinate> *coordinateMAB2 = new UCB1<Coordinate>(2.0);
	CMABStrategy bot1Strategy(evaluator2, moveMAB2, coordinateMAB2, 0.5);

	Bot bot1 = Bot(&bot1Strategy);

	StrategyTesting::playTournament(bot0, bot1, 50);
	//playMatch(bot0, bot1, false);
}

int main() {
	// Initialize random number generator
	srand(time(NULL));

	//unsigned availableThreads = thread::hardware_concurrency();
	//cerr << "available threads: " << availableThreads << "\n";

	//freopen("cerr_log.txt", "w", stderr);

	playTest();
	//test();
	//play();
	
	delete Board::simulationLookupTable;
	_CrtDumpMemoryLeaks();
 }