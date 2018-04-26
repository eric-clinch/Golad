
#include "CMABStrategyParallel.h"

CMABStrategyParallel::CMABStrategyParallel(Evaluator *evaluator, MAB<MoveComponents> *moveMAB, MAB<Coordinate> *coordinateMAB, 
							 MAB<MoveComponents> *secondaryMoveMAB, float greediness, float alpha, float exploreTimeRatio, int topMoveNum) {
	this->evaluator = evaluator;
	this->moveMAB = moveMAB;
	this->coordinateMAB = coordinateMAB;
	this->secondaryMoveMAB = secondaryMoveMAB;
	this->greediness = greediness;
	this->alpha = alpha;
	this->exploreTimeRatio = exploreTimeRatio;
	this->topMoveNum = topMoveNum;
	this->previousTimeEnd = Tools::get_time();
	this->stateManager1 = NULL;
	this->stateManager2 = NULL;
}

CMABStrategyParallel::~CMABStrategyParallel() {
	delete evaluator;
	delete stateManager1;
	delete stateManager2;
}

struct DevelopTreeArgs {
	CMABStateParallel *MCTree;
	long timeToUse;
	float alpha;
	int *counter;
	Board *board;
	Player playerID;
	Player enemyID;
	CMABStateParallelManager *stateManager;

	DevelopTreeArgs(CMABStateParallel *MCTree, Board *board, long timeToUse, float alpha, int *count, Player playerID, Player enemyID, 
					CMABStateParallelManager *stateManager) {
		this->MCTree = MCTree;
		this->timeToUse = timeToUse;
		this->alpha = alpha;
		this->counter = count;
		this->board = board;
		this->playerID = playerID;
		this->enemyID = enemyID;
		this->stateManager = stateManager;
	}
};

struct ExploitTreeArgs {
	CMABStateParallel *MCTree;
	Board *board;
	long timeToUse;
	int *counter;
	Player playerID;
	Player enemyID;
	MAB<MoveComponents> *moveMAB;
	CMABStateParallelManager *stateManager;

	ExploitTreeArgs(CMABStateParallel *MCTree, Board *board, long timeToUse, int *count, Player playerID, Player enemyID, MAB<MoveComponents> *moveMAB,
					CMABStateParallelManager *stateManager) {
		this->MCTree = MCTree;
		this->board = board;
		this->timeToUse = timeToUse;
		this->counter = count;
		this->playerID = playerID;
		this->enemyID = enemyID;
		this->moveMAB = moveMAB;
		this->stateManager = stateManager;
	}
};

void *CMABStrategyParallel::developTree(void *arg) {
	long startTime = Tools::get_time();
	DevelopTreeArgs *developTreeArgs = (DevelopTreeArgs*)arg;
	CMABStateParallel *MCTree = developTreeArgs->MCTree;
	long timeToUse = developTreeArgs->timeToUse;
	float alpha = developTreeArgs->alpha;
	int *counter = developTreeArgs->counter;
	Board &board = *developTreeArgs->board;
	Player playerID = developTreeArgs->playerID;
	Player enemyID = developTreeArgs->enemyID;
	CMABStateParallelManager *stateManager = developTreeArgs->stateManager;

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

void *CMABStrategyParallel::exploitTree(void *arg) {
	long startTime = Tools::get_time();
	ExploitTreeArgs *exploitTreeArgs = (ExploitTreeArgs*)arg;
	CMABStateParallel *MCTree = exploitTreeArgs->MCTree;
	long timeToUse = exploitTreeArgs->timeToUse;
	int *counter = exploitTreeArgs->counter;
	Board &board = *exploitTreeArgs->board;
	Player playerID = exploitTreeArgs->playerID;
	Player enemyID = exploitTreeArgs->enemyID;
	MAB<MoveComponents> *moveMAB = exploitTreeArgs->moveMAB;
	CMABStateParallelManager *stateManager = exploitTreeArgs->stateManager;

	Board copiedBoard(board.getWidth(), board.getHeight());
	Board emptyBoard(board.getWidth(), board.getHeight());

	int count = 0;
	for (long currentTime = Tools::get_time(); currentTime - startTime < timeToUse; currentTime = Tools::get_time()) {
		board.copyInto(copiedBoard);
		MCTree->exploitRound(copiedBoard, emptyBoard, playerID, enemyID, moveMAB);
		count++;
	}

	*counter = count;
	return MCTree;
}

void setTopMoves(CMABStateParallel *MCTree1, CMABStateParallel *MCTree2, Board &board, Player playerID, Player enemyID, int moveCount) {
	Board emptyBoard(board.getWidth(), board.getHeight());

	vector<Move> topMoves1 = MCTree1->getTopMoves(moveCount);
	vector<Move> topMoves2 = MCTree2->getTopMoves(moveCount);
	unordered_set<Move> topMovesSet;
	for (int i = 0; i < topMoves1.size(); i++) topMovesSet.emplace(topMoves1[i]);
	for (int i = 0; i < topMoves2.size(); i++) topMovesSet.emplace(topMoves2[i]);
	vector<Move> topMoves;
	for (unordered_set<Move>::iterator moveItr = topMovesSet.begin(); moveItr != topMovesSet.end(); moveItr++) {
		topMoves.push_back(*moveItr);
	}

	MCTree1->setMoves(topMoves, board, emptyBoard, playerID, enemyID);

	MCTree2->setMoves(topMoves, board, emptyBoard, playerID, enemyID);
}

Move CMABStrategyParallel::getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round) {
	long startTime = Tools::get_time();
	long timePassed = Tools::get_time() - previousTimeEnd;

	long roundsRemaining = 100 - round; // the maximum number of rounds that will be played after this round
	long extraTimeGiven = roundsRemaining * (timePerMove);
	long timeToUse = (time + extraTimeGiven) / (roundsRemaining + 1);
	long timeToSearch = timeToUse - 5;
	long explorationTime = timeToSearch * exploreTimeRatio;
	long exploitationTime = timeToSearch - explorationTime;

	if (round == 0) {
		timeToSearch = 3 * timeToSearch / 2;
	}

	if (stateManager1 == NULL) {
		assert(stateManager2 == NULL);
		stateManager1 = new CMABStateParallelManager(board, evaluator, coordinateMAB, moveMAB, greediness, playerID, enemyID);
		stateManager2 = new CMABStateParallelManager(board, evaluator, coordinateMAB, moveMAB, greediness, playerID, enemyID);
	}
	else {
		stateManager1->newRound(board, playerID, enemyID);
		stateManager2->newRound(board, playerID, enemyID);
	}

	CMABStateParallel *MCTree1 = stateManager1->rootNode;
	int counter1;
	DevelopTreeArgs developTreeArgs1(MCTree1, &board, explorationTime, alpha, &counter1, playerID, enemyID, stateManager1);
	ExploitTreeArgs exploitTreeArgs1(MCTree1, &board, exploitationTime, &counter1, playerID, enemyID, secondaryMoveMAB, stateManager1);

	CMABStateParallel *MCTree2 = stateManager2->rootNode;
	int counter2;
	Board *boardCopy = board.getCopy();
	DevelopTreeArgs developTreeArgs2(MCTree2, boardCopy, explorationTime, alpha, &counter2, playerID, enemyID, stateManager2);
	ExploitTreeArgs exploitTreeArgs2(MCTree2, boardCopy, exploitationTime, &counter2, playerID, enemyID, secondaryMoveMAB, stateManager2);

	pthread_t thread;
	pthread_create(&thread, NULL, &developTree, (void*)&developTreeArgs2);
	developTree((void*)&developTreeArgs1);
	pthread_join(thread, NULL);
	int counter = counter1 + counter2;

	setTopMoves(MCTree1, MCTree2, board, playerID, enemyID, topMoveNum);
	pthread_create(&thread, NULL, &exploitTree, (void*)&exploitTreeArgs2);
	exploitTree((void*)&exploitTreeArgs1);
	pthread_join(thread, NULL);

	delete boardCopy;

	int movesExplored = MCTree1->getMovesExplored();
	counter += counter1 + counter2;
	float score;
	Move result = MCTree1->getBestMove(&score, MCTree2, board);

	long currentTime = Tools::get_time();
	long timeUsed = currentTime - startTime;
	previousTimeEnd = currentTime;
	cerr << "total states: " << stateManager1->states->size() + stateManager2->states->size() << "\n";
	cerr << "CMABParallel round: " << round << " time to use: " << timeToUse << " time used: " << timeUsed << " time passed since last round: " <<
			timePassed << " counter: " << counter << " moves explored: " << movesExplored << " move score: " << score << "\n";
	return result;
}

void CMABStrategyParallel::cleanUp() {
	delete stateManager1;
	delete stateManager2;
	stateManager1 = NULL;
	stateManager2 = NULL;
}

string CMABStrategyParallel::toString() {
	ostringstream stringStream;
	stringStream << "CMABStrategyParallel(" << evaluator->toString() << ", " << moveMAB->toString() << ", " <<
		coordinateMAB->toString() << ", " << secondaryMoveMAB->toString() << ", " << greediness << ", " << 
		alpha << ", " << exploreTimeRatio << ", " << topMoveNum << ")";
	return stringStream.str();
}