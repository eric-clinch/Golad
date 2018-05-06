
#include "CMABStrategy.h"

CMABStrategy::CMABStrategy(Evaluator *evaluator, MAB<MoveComponents> *moveMAB, 
							 MAB<Coordinate> *coordinateMAB, float greediness, float alpha, int lowerMovesBound, int upperMovesBound) {
	this->evaluator = evaluator;
	this->moveMAB = moveMAB;
	this->coordinateMAB = coordinateMAB;
	this->greediness = greediness;
	this->alpha = alpha;
	this->previousTimeEnd = Tools::get_time();
	this->stateManager1 = NULL;
	assert(lowerMoveBound <= upperMoveBound);
	this->lowerMoveBound = lowerMovesBound;
	this->upperMoveBound = upperMovesBound;
}

CMABStrategy::~CMABStrategy() {
	delete evaluator;
	delete stateManager1;
}

struct FreeTreesArgs {
	CMABState *MCTree1;
	CMABState *MCTree2;

	FreeTreesArgs(CMABState *MCTree1, CMABState *MCTree2) {
		this->MCTree1 = MCTree1;
		this->MCTree2 = MCTree2;
	}
};

void *CMABStrategy::freeTree(void *arg) {
	CMABState *MCTree = (CMABState*)arg;
	MCTree->freeShared();
	delete MCTree;
	return NULL;
}

void *CMABStrategy::freeTrees(void *arg) {
	FreeTreesArgs *freeTreeArgs = (FreeTreesArgs*)arg;

	freeTreeArgs->MCTree1->freeShared();
	delete freeTreeArgs->MCTree1;

	freeTreeArgs->MCTree2->freeShared();
	delete freeTreeArgs->MCTree2;

	delete freeTreeArgs;
	return NULL;
}

Move CMABStrategy::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round) {
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
		stateManager1 = new CMABStateManager(board, evaluator, coordinateMAB, moveMAB, greediness, playerID, enemyID);
	}
	else {
		stateManager1->newRound(board, playerID, enemyID);
	}

	CMABState *MCTree = stateManager1->rootNode;

	Board copiedBoard(board.getWidth(), board.getHeight());
	Board emptyBoard(board.getWidth(), board.getHeight());

	int count = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < timeToUse; currentTime = Tools::get_time()) {
		board.copyInto(copiedBoard);
		float exploration = (float)1 / (1 + alpha * count);
		MCTree->setGreed(1 - exploration);
		MCTree->CMABRound(copiedBoard, emptyBoard, playerID, enemyID);
		count++;
		if (MCTree->getMovesExplored() > upperMoveBound) MCTree->prune(lowerMoveBound);
	}

	float score;
	Move result = MCTree->getBestMove(&score, board);
	int movesExplored = MCTree->getMovesExplored();

	long currentTime = Tools::get_time();
	long timeUsed = currentTime - startTime;
	previousTimeEnd = currentTime;

	cerr << "CMAB round: " << round << " time to use: " << timeToUse << " time used: " << timeUsed << " time passed since last round: " <<
		timePassed << " counter: " << count << " moves explored: " << movesExplored << " move score: " << score << "\n";

	return result;
}

void CMABStrategy::cleanUp() {
	delete stateManager1;
	stateManager1 = NULL;
}

string CMABStrategy::toString() {
	ostringstream stringStream;
	stringStream << "CMABStrategy(" << evaluator->toString() << ", " << moveMAB->toString() << ", " <<
		coordinateMAB->toString() << ", " << greediness << ", " << alpha << ", " << lowerMoveBound << ", " << upperMoveBound <<  ")";
	return stringStream.str();
}