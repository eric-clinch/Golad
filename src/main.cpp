
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
#include "RandomStrategy.h"
#include "CMABStrategy.h"
#include "CMABStrategy2.h"
#include "MoveComponents.h"
#include "Evaluator.h"
#include "RatioEvaluator.h"
#include "AdversarialEvaluator.h"
#include "MAB.h"
#include "EpsilonGreedy.h"
#include "UCB1.h"
#include "UCBHybrid.h"
#include "Board.h"
#include "Board2.h"
#include "Coordinate.h"
#include "Move.h"
#include "StrategyTesting.h"
#include "ParameterOptimization.h"

using namespace std;

void test() {
	cout << sizeof(UtilityNode<Coordinate>) << "\n";
	//Strategy *randomStrategy = new RandomStrategy();
	//Board board(18, 16);
	//Board emptyBoard(18, 16);
	//Board2 board2(18, 16);
	//Board2 emptyBoard2(18, 16);

	//for (int i = 0; i < 1000; i++) {
	//	board.initiateBoardPositions();
	//	string s = board.repr();
	//	board2.UpdateBoard2(stringstream(s));
	//	assert(board.toString() == board2.toString());

	//	board2.initiateBoard2Positions();
	//	board.UpdateBoard(stringstream(board2.repr()));
	//	assert(board.toString() == board2.toString());
	//}

	//for (int i = 0; i < 1000; i++) {
	//	Player p1 = i % 2 == 0 ? P0 : P1;
	//	Player p2 = i % 2 == 0 ? P1 : P0;

	//	board.initiateBoardPositions();
	//	string s = board.repr();
	//	board2.UpdateBoard2(stringstream(s));
	//	Move m = randomStrategy->getMove(board, p1, p2, 0, 0, 0);

	//	Board *nextBoard = board.getNextRoundBoard();
	//	Board2 *nextBoard2 = board2.getNextRoundBoard2();
	//	assert(nextBoard->toString() == nextBoard2->toString());

	//	board.applyMove(m, p1, *nextBoard, emptyBoard);
	//	board2.applyMove(m, p1, *nextBoard2, emptyBoard2);
	//	assert(emptyBoard.toString() == emptyBoard2.toString());

	//	delete nextBoard;
	//	delete nextBoard2;

	//	nextBoard = board.makeMove(m, p1);
	//	nextBoard2 = board2.makeMove(m, p1);
	//	assert(nextBoard->toString() == nextBoard2->toString());

	//	delete nextBoard;
	//	delete nextBoard2;

	//	board.makeMoveOnBoard(m, p1);
	//	board2.makeMoveOnBoard2(m, p1);
	//	assert(board.toString() == board2.toString());

	//	board.setNextRoundBoard(emptyBoard);
	//	board2.setNextRoundBoard2(emptyBoard2);
	//	assert(emptyBoard.toString() == emptyBoard2.toString());

	//	board.copyInto(emptyBoard);
	//	board2.copyInto(emptyBoard2);
	//	assert(emptyBoard.toString() == emptyBoard2.toString());
	//}

	//cout << "passed all test cases :)\n";
}

void play() {
	RatioEvaluator *evaluator = new RatioEvaluator();
	UCBHybrid<MoveComponents> *moveMAB1 = new UCBHybrid<MoveComponents>(4.5, 0.9);
	EpsilonGreedy<Coordinate> *coordinateMAB1 = new EpsilonGreedy<Coordinate>(0.4);
	CMABStrategy strategy(evaluator, moveMAB1, coordinateMAB1, 0.5, 0.000175905);
	Bot myBot = Bot(&strategy);
	Parser parser = Parser(myBot);
	parser.Parse();
}

void playTest() {
	Evaluator *evaluator0 = new RatioEvaluator();
	//UCBHybrid<MoveComponents> *moveMAB0 = new UCBHybrid<MoveComponents>(4.5, 0.9);
	//EpsilonGreedy<Coordinate> *coordinateMAB0 = new EpsilonGreedy<Coordinate>(0.4);
	//CMABStrategy bot0Strategy(evaluator0, moveMAB0, coordinateMAB0, 0.5, 0.000175905);

	UCBHybrid<MoveComponents> *moveMAB0 = new UCBHybrid<MoveComponents>(4.5, 0.9);
	EpsilonGreedy<Coordinate> *coordinateMAB0 = new EpsilonGreedy<Coordinate>(0.4);
	UCB1<MoveComponents> *secondaryMoveMAB0 = new UCB1<MoveComponents>(2);
	CMABStrategy2 bot0Strategy(evaluator0, moveMAB0, coordinateMAB0, secondaryMoveMAB0, 0.5, 0.000175905, .5, 5);

	Bot bot0 = Bot(&bot0Strategy);
	
	Evaluator *evaluator1 = new RatioEvaluator();
	UCBHybrid<MoveComponents> *moveMAB1 = new UCBHybrid<MoveComponents>(2, 0.9);
	EpsilonGreedy<Coordinate> *coordinateMAB1 = new EpsilonGreedy<Coordinate>(0.5);
	UCBHybrid<MoveComponents> *secondaryMoveMAB1 = new UCBHybrid<MoveComponents>(3.5, 0.8);
	CMABStrategy2 bot1Strategy(evaluator1, moveMAB1, coordinateMAB1, secondaryMoveMAB1, 0.75, 0.000368, 0.8, 5);

	//UCBHybrid<MoveComponents> *moveMAB1 = new UCBHybrid<MoveComponents>(4.5, 0.9);
	//EpsilonGreedy<Coordinate> *coordinateMAB1 = new EpsilonGreedy<Coordinate>(0.4);
	//CMABStrategy bot1Strategy(evaluator1, moveMAB1, coordinateMAB1, 0.5, 0.000175905);

	Bot bot1 = Bot(&bot1Strategy);

	StrategyTesting::playTournament(bot0, bot1, 10);
	//StrategyTesting::playMatch(bot0, bot1, false);
}

int main() {
	// Initialize random number generator
	srand(time(NULL));

	//unsigned availableThreads = thread::hardware_concurrency();
	//cerr << "available threads: " << availableThreads << "\n";

	freopen("cerr_log.txt", "w", stderr);

	playTest();
	//test();
	//play();

	//ParameterOptimization::optimizeParameters(100000);

	delete Board::simulationLookupTable;
	_CrtDumpMemoryLeaks();
}