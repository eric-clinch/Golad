
#include "CMABStrategy.h"

CMABStrategy::CMABStrategy(Evaluator *evaluator, MAB<MoveComponents> *moveMAB, MAB<Coordinate> *coordinateMAB, 
							 float greediness, float alpha, bool parallel) {
	this->evaluator = evaluator;
	this->moveMAB = moveMAB;
	this->coordinateMAB = coordinateMAB;
	this->greediness = greediness;
	this->alpha = alpha;
	this->previousTimeEnd = Tools::get_time();
	this->parallel = parallel;
	this->stateManager1 = NULL;
	this->stateManager2 = NULL;
}

CMABStrategy::~CMABStrategy() {
	delete evaluator;
	delete stateManager1;
	delete stateManager2;
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

struct DevelopTreeArgs {
	CMABState *MCTree;
	long timeToUse;
	float alpha;
	int *counter;
	Board *board;
	Player playerID;
	Player enemyID;

	DevelopTreeArgs(CMABState *MCTree, Board *board, long timeToUse, float alpha, int *count, Player playerID, Player enemyID) {
		this->MCTree = MCTree;
		this->timeToUse = timeToUse;
		this->alpha = alpha;
		this->counter = count;
		this->board = board;
		this->playerID = playerID;
		this->enemyID = enemyID;
	}
};

void *CMABStrategy::developTree(void *arg) {
	long startTime = Tools::get_time();
	DevelopTreeArgs *developTreeArgs = (DevelopTreeArgs*)arg;
	CMABState *MCTree = developTreeArgs->MCTree;
	long timeToUse = developTreeArgs->timeToUse;
	float alpha = developTreeArgs->alpha;
	int *counter = developTreeArgs->counter;
	Board &board = *developTreeArgs->board;
	Player playerID = developTreeArgs->playerID;
	Player enemyID = developTreeArgs->enemyID;

	Board copiedBoard(board.getWidth(), board.getHeight());
	Board emptyBoard(board.getWidth(), board.getHeight());

	int count = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < timeToUse; currentTime = Tools::get_time()) {
		board.copyInto(copiedBoard);
		float exploration = (float)1 / (1 + alpha * count);
		MCTree->setGreed(1 - exploration);
		MCTree->CMABRound(copiedBoard, emptyBoard, playerID, enemyID);
		count++;
	}

	*counter = count;
	return MCTree;
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
		assert(stateManager2 == NULL);
		stateManager1 = new CMABStateManager(board, evaluator, coordinateMAB, moveMAB, greediness, playerID, enemyID);
		stateManager2 = new CMABStateManager(board, evaluator, coordinateMAB, moveMAB, greediness, playerID, enemyID);
	}
	else {
		stateManager1->newRound(board, playerID, enemyID);
		stateManager2->newRound(board, playerID, enemyID);
	}

	CMABState *MCTree1 = stateManager1->rootNode;
	int counter1;
	DevelopTreeArgs developTreeArgs1(MCTree1, &board, timeToSearch, alpha, &counter1, playerID, enemyID);

	int movesExplored;
	int counter;
	float score;
	Move result;
	pthread_t thread;
	if (parallel) {
		CMABState *MCTree2 = stateManager2->rootNode;
		int counter2;
		Board *boardCopy = board.getCopy();
		DevelopTreeArgs developTreeArgs2(MCTree2, boardCopy, timeToSearch, alpha, &counter2, playerID, enemyID);

		pthread_create(&thread, NULL, &developTree, (void*)&developTreeArgs2);
		developTree((void*)&developTreeArgs1);
		pthread_join(thread, NULL);
		delete boardCopy;

		movesExplored = MCTree1->getMovesExplored();
		counter = counter1 + counter2;
		result = MCTree1->getBestMove(&score, MCTree2, board);
	}
	else {
		developTree((void*)&developTreeArgs1);
		movesExplored = MCTree1->getMovesExplored();
		counter = counter1;
		result = MCTree1->getBestMove(&score, board);
	}

	long currentTime = Tools::get_time();
	long timeUsed = currentTime - startTime;
	previousTimeEnd = currentTime;
	cerr << "CMAB round: " << round << " time to use: " << timeToUse << " time used: " << timeUsed << " time passed since last round: " <<
		timePassed << " counter: " << counter << " moves explored: " << movesExplored << " move score: " << score << "\n";

	return result;
}

void CMABStrategy::cleanUp() {
	delete stateManager1;
	delete stateManager2;
	stateManager1 = NULL;
	stateManager2 = NULL;
}

string CMABStrategy::toString() {
	ostringstream stringStream;
	stringStream << "CMABStrategy(" << evaluator->toString() << ", " << moveMAB->toString() << ", " <<
		coordinateMAB->toString() << ", " << greediness << ", " << alpha << ")";
	return stringStream.str();
}