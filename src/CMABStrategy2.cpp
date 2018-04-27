
#include "CMABStrategy2.h"

CMABStrategy2::CMABStrategy2(Evaluator *evaluator, MAB<MoveComponents> *moveMAB, MAB<MoveComponents> *rootMAB, 
							 MAB<Coordinate> *coordinateMAB, float greediness, float alpha) {
	this->evaluator = evaluator;
	this->moveMAB = moveMAB;
	this->rootMAB = rootMAB;
	this->coordinateMAB = coordinateMAB;
	this->greediness = greediness;
	this->alpha = alpha;
	this->previousTimeEnd = Tools::get_time();
	this->stateManager1 = NULL;
}

CMABStrategy2::~CMABStrategy2() {
	delete evaluator;
	delete stateManager1;
}

struct FreeTreesArgs {
	CMABState2 *MCTree1;
	CMABState2 *MCTree2;

	FreeTreesArgs(CMABState2 *MCTree1, CMABState2 *MCTree2) {
		this->MCTree1 = MCTree1;
		this->MCTree2 = MCTree2;
	}
};

void *CMABStrategy2::freeTree(void *arg) {
	CMABState2 *MCTree = (CMABState2*)arg;
	MCTree->freeShared();
	delete MCTree;
	return NULL;
}

void *CMABStrategy2::freeTrees(void *arg) {
	FreeTreesArgs *freeTreeArgs = (FreeTreesArgs*)arg;

	freeTreeArgs->MCTree1->freeShared();
	delete freeTreeArgs->MCTree1;

	freeTreeArgs->MCTree2->freeShared();
	delete freeTreeArgs->MCTree2;

	delete freeTreeArgs;
	return NULL;
}

Move CMABStrategy2::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round) {
	long startTime = Tools::get_time();
	long timePassed = Tools::get_time() - previousTimeEnd;

	long roundsRemaining = 100 - round; // the maximum number of rounds that will be played after this round
	long extraTimeGiven = roundsRemaining * (timePerMove);
	long timeToUse = (time + extraTimeGiven) / (roundsRemaining + 1);
	long timeToSearch = timeToUse - 5;

	if (round == 0) {
		timeToSearch = 3 * timeToSearch / 2;
	}

	if (stateManager1 == NULL) {
		assert(stateManager2 == NULL);
		stateManager1 = new CMABState2Manager(board, evaluator, coordinateMAB, moveMAB, greediness, playerID, enemyID);
	}
	else {
		stateManager1->newRound(board, playerID, enemyID);
	}

	CMABState2 *MCTree = stateManager1->rootNode;

	Board copiedBoard(board.getWidth(), board.getHeight());
	Board emptyBoard(board.getWidth(), board.getHeight());

	int count = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < timeToUse; currentTime = Tools::get_time()) {
		board.copyInto(copiedBoard);
		float exploration = (float)1 / (1 + alpha * count);
		MCTree->setGreed(1 - exploration);
		MCTree->CMABRound(copiedBoard, emptyBoard, playerID, enemyID, rootMAB);
		count++;
	}

	float score;
	Move result = MCTree->getBestMove(&score, board);
	int movesExplored = MCTree->getMovesExplored();

	long currentTime = Tools::get_time();
	long timeUsed = currentTime - startTime;
	previousTimeEnd = currentTime;
	//cerr << "CMAB2 round: " << round << " time to use: " << timeToUse << " time used: " << timeUsed << " time passed since last round: " <<
	//	timePassed << " counter: " << count << " moves explored: " << movesExplored << " move score: " << score << "\n";

	return result;
}

void CMABStrategy2::cleanUp() {
	delete stateManager1;
	stateManager1 = NULL;
}

string CMABStrategy2::toString() {
	ostringstream stringStream;
	stringStream << "CMABStrategy2(" << evaluator->toString() << ", " << moveMAB->toString() << ", " << rootMAB->toString() << ", " <<
		coordinateMAB->toString() << ", " << greediness << ", " << alpha << ")";
	return stringStream.str();
}