
#include "CMABStrategy.h"

CMABStrategy::CMABStrategy(Evaluator *evaluator, MAB<Move> *moveMAB, MAB<Coordinate> *coordinateMAB, float greediness) {
	this->evaluator = evaluator;
	this->moveMAB = moveMAB;
	this->coordinateMAB = coordinateMAB;
	this->greediness = greediness;
}

CMABStrategy::~CMABStrategy() {
	free(evaluator);
	free(moveMAB);
	free(coordinateMAB);
}

Move CMABStrategy::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round) {
	long startTime = Tools::get_time();

	int roundsRemaining = 100 - round; // the maximum number of rounds that will be played after this round
	int extraTimeGiven = roundsRemaining * timePerMove;
	int timeToUse = (time + extraTimeGiven) / (roundsRemaining + 1);
	timeToUse -= 10;

	CMABState MCTSTree(board, evaluator, coordinateMAB, moveMAB, greediness, playerID, enemyID);
	Board copiedBoard(board.getWidth(), board.getHeight());
	Board emptyBoard(board.getWidth(), board.getHeight());

	int counter = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < timeToUse; currentTime = Tools::get_time()) {
		counter++;
		board.copyInto(copiedBoard);
		MCTSTree.CMABRound(copiedBoard, emptyBoard, playerID, enemyID);
	}

	//MCTSTree.printTree();
	Move result = MCTSTree.getBestMove();
	MCTSTree.freeShared();
	return result;
}