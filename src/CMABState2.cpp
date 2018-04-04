
#include "CMABState2.h"

using namespace std;

struct CMABState2::SharedData {
	Evaluator *evaluator;
	MAB<Coordinate> *coordinateMAB;
	MAB<MoveComponents> *moveMAB;
	float greediness;
	default_random_engine generator;
	uniform_real_distribution<float> uniformRealDistribution;
	Board *boardCopy; // used by the async thread
	Board *moveResultBoardCopy; // used by the async thread

	SharedData() {};

	SharedData(Evaluator *e, MAB<Coordinate> *cMAB, MAB<MoveComponents> *mMAB, float greed) {
		evaluator = e;
		coordinateMAB = cMAB;
		moveMAB = mMAB;
		greediness = greed;

		generator = default_random_engine();
		random_device rd;
		generator.seed(rd());
		uniform_real_distribution<float> uniformRealDistribution(0.0, 1.0);

		boardCopy = new Board(18, 16);
		moveResultBoardCopy = new Board(18, 16);
	}

	~SharedData() {
		delete boardCopy;
		delete moveResultBoardCopy;
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

// returns whether the given board matches what we believe it should look like.
// Each CMABState2 should have a unique board associated with it, and this checks
// whether or not the given board is that unique board.
bool CMABState2::isCorrectBoard(Board &board, Player playerID) {
	for (int i = 0; i < targets->size(); i++) {
		UtilityNode<Coordinate> *node = (*targets)[i];
		Coordinate c = node->object;
		// targets can only be enemy cells (in the case of the kill move)
		// or dead cells (in the case of the birth move)
		if (board.getCoordinateType(c) == playerID) {
			return false;
		}
	}
	for (int i = 0; i < sacrifices->size(); i++) {
		UtilityNode<Coordinate> *cNode = (*sacrifices)[i];
		Coordinate c = cNode->object;
		if (board.getCoordinateType(c) != playerID) {
			return false;
		}
	}
	return true;
}

void CMABState2::getTargetsAndSacrifices(Board &board, Player playerID, Player enemyID) {
	int width = board.getWidth();
	int height = board.getHeight();
	int numPlayerCells = board.getPlayerCellCount(playerID);

	this->targets = new vector<UtilityNode<Coordinate>*>();
	this->sacrifices = new vector<UtilityNode<Coordinate>*>();
	this->targets->reserve(width * height - numPlayerCells);
	this->sacrifices->reserve(numPlayerCells);

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Coordinate c = Coordinate(x, y);
			char type = board.getCoordinateType(x, y);
			if (type == playerID) {
				UtilityNode<Coordinate> *cNode = new UtilityNode<Coordinate>(c);
				sacrifices->push_back(cNode);
			}
			else {
				if (type == enemyID || numPlayerCells > 2) {
					// if the cell is an enemy cell, then the move is a kill move which is always legal
					// otherwise the move is a sacrifice move, which is only legal/viable when the
					// number of player cells is greater than 2
					UtilityNode<Coordinate> *cNode = new UtilityNode<Coordinate>(c);
					targets->push_back(cNode);
				}
			}
		}
	}
	//random_shuffle(sacrifices->begin(), sacrifices->end());
	//random_shuffle(targets->begin(), targets->end());
}

CMABState2::CMABState2(Board &board, SharedData *sharedData, Player playerID, Player enemyID) {
	this->sharedData = sharedData;
	this->moves = new vector<UtilityNode<MoveComponents>>();
	this->childrenStates = new unordered_map<Move, CMABState2*>();
	this->nextRoundBoard = board.getNextRoundBoard();
	this->numTrials = 0;
	getTargetsAndSacrifices(board, playerID, enemyID);
	assert(isCorrectBoard(board, playerID));
}

CMABState2::CMABState2(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *moveMAB, float greediness,
	Player playerID, Player enemyID) {
	sharedData = new SharedData(evaluator, coordinateMAB, moveMAB, greediness);
	this->moves = new vector<UtilityNode<MoveComponents>>();
	this->childrenStates = new unordered_map<Move, CMABState2*>();
	this->nextRoundBoard = board.getNextRoundBoard();
	this->numTrials = 0;
	getTargetsAndSacrifices(board, playerID, enemyID);
	assert(isCorrectBoard(board, playerID));
}

CMABState2::~CMABState2() {
	for (int i = 0; i < targets->size(); i++) {
		delete (*targets)[i];
	}
	delete targets;

	for (int i = 0; i < sacrifices->size(); i++) {
		delete (*sacrifices)[i];
	}
	delete sacrifices;
	delete moves;

	unordered_map<Move, CMABState2*>::iterator it = childrenStates->begin();
	for (; it != childrenStates->end(); it++) {
		delete it->second;
	}

	delete childrenStates;
	delete nextRoundBoard;
}

void CMABState2::freeShared() {
	delete sharedData;
}

UtilityNode<MoveComponents> CMABState2::selectExploreMove(Board &board) {
	int targetIndex = sharedData->coordinateMAB->getChoice(*targets, numTrials);
	UtilityNode<Coordinate> *targetNode = (*targets)[targetIndex];
	MoveComponents moveComponents;
	if (board.getCoordinateType(targetNode->object) == '.') {
		// the move will be a sacrifice move
		int sacrificeIndex1 = sharedData->coordinateMAB->getChoice(*sacrifices, numTrials);
		UtilityNode<Coordinate> *sacrificeNode1 = remove(*sacrifices, sacrificeIndex1); // remove it so we don't choose it twice
		int sacrificeIndex2 = sharedData->coordinateMAB->getChoice(*sacrifices, numTrials);
		UtilityNode<Coordinate> *sacrificeNode2 = (*sacrifices)[sacrificeIndex2];
		sacrifices->push_back(sacrificeNode1);

		moveComponents = MoveComponents(targetNode, sacrificeNode1, sacrificeNode2);
	}
	else {
		// the move will be a kill move
		moveComponents = MoveComponents(targetNode);
	}

	if (childrenStates->count(moveComponents.move) != 0) {
		// this move has already been explored, so just exploit a move instead
		return selectExploitMove(board);
	}

	return UtilityNode<MoveComponents>(moveComponents);
}

UtilityNode<MoveComponents> CMABState2::selectExploitMove(Board &board) {
	assert(moves->size() > 0);
	int moveNodeIndex = sharedData->moveMAB->getChoice(*moves, numTrials);
	UtilityNode<MoveComponents> result = (*moves)[moveNodeIndex];
	remove(*moves, moveNodeIndex); // remove this move so that it's not selected twice
	return result;
}

UtilityNode<MoveComponents> CMABState2::selectMove(Board &board) {
	float rand = sharedData->uniformRealDistribution(sharedData->generator);
	if (moves->size() == 0 || rand > sharedData->greediness) {
		return selectExploreMove(board);
	}
	else {
		return selectExploitMove(board);
	}
}

struct CMABState2::AsyncExploitMoveArgs {
	Board board;
	Board moveResultBoard;
	UtilityNode<MoveComponents> moveNode;
	Player playerID;
	Player enemyID;
	CMABState2 *childState;

	AsyncExploitMoveArgs(Board &board, Board &moveResultBoard, UtilityNode<MoveComponents> &moveNode, Player playerID, Player enemyID,
						 CMABState2 *childState) {
		this->board = board;
		this->moveResultBoard = moveResultBoard;
		this->moveNode = moveNode;
		this->playerID = playerID;
		this->enemyID = enemyID;
		this->childState = childState;
	}
};

void *CMABState2::asyncExploitMove(void *args) {
	AsyncExploitMoveArgs *asyncExploitMoveArgs = (AsyncExploitMoveArgs*)args;
	Board &board = asyncExploitMoveArgs->board;
	Board &moveResultBoard = asyncExploitMoveArgs->moveResultBoard;
	UtilityNode<MoveComponents> moveNode = asyncExploitMoveArgs->moveNode;
	Player playerID = asyncExploitMoveArgs->playerID;
	Player enemyID = asyncExploitMoveArgs->enemyID;
	CMABState2 *childState = asyncExploitMoveArgs->childState;

	assert(board.isLegal(moveNode.object.move, playerID));
	assert(*board.makeMove(moveNode.object.move, playerID) == moveResultBoard);
	Move move = moveNode.object.move;

	float moveEvaluation = 1 - childState->CMABRound(moveResultBoard, board, enemyID, playerID);
	moveNode.updateUtility(moveEvaluation);
	moveNode.object.updateUtilities(moveEvaluation);

	return NULL;
}

void CMABState2::topLevelExploitRound(Board &board, Board &moveResultBoard, UtilityNode<MoveComponents> &moveNode,
									  Player playerID, Player enemyID) {
	assert(board.isLegal(moveNode.object.move, playerID));
	Move move = moveNode.object.move;

	assert(childrenStates->find(moveNode.object.move) != childrenStates->end());
	CMABState2 *childState = (*childrenStates)[move];
	assert(childState != NULL);
	float moveEvaluation = 1 - childState->CMABRound(moveResultBoard, board, enemyID, playerID);
	moveNode.updateUtility(moveEvaluation);
	moveNode.object.updateUtilities(moveEvaluation);
}

void CMABState2::topLevelExploreRound(Board &board, Board &moveResultBoard, UtilityNode<MoveComponents> &moveNode,
									  Player playerID, Player enemyID) {
	// evaluate the move
	assert(board.isLegal(moveNode.object.move, playerID));
	assert((*childrenStates)[moveNode.object.move] == NULL);
	MoveComponents moveComponents = moveNode.object;
	float moveEvaluation = sharedData->evaluator->evaluate(moveResultBoard, playerID, enemyID);
	moveNode.updateUtility(moveEvaluation);
	moveComponents.updateUtilities(moveEvaluation);
}

void CMABState2::topLevelCMABRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID) {
	//CMABRound(board, moveResultBoard, playerID, enemyID);
	numTrials++;

	UtilityNode<MoveComponents> moveNode = selectMove(board);
	MoveComponents moveComponents = moveNode.object;

	bool isExploreMove = false;
	if (childrenStates->count(moveComponents.move) != 0) {
		// this is an exploitation move
		// we need to guarantee that the childState is non-NULL before topLevelExploitRound can be called
		CMABState2 *childState = (*childrenStates)[moveComponents.move];
		// we also need to guarantee that the moveResultBoard is the result of making the move on the current
		// board before topLevelExploitRound can be called. This is to avoid recomputation of the moveResultBoard
		board.applyMove(moveComponents.move, playerID, *nextRoundBoard, moveResultBoard);
		if (childState == NULL) {
			childState = new CMABState2(moveResultBoard, sharedData, enemyID, playerID);
			(*childrenStates)[moveComponents.move] = childState;
		}
	}
	else {
		isExploreMove = true;
		(*childrenStates)[moveComponents.move] = NULL; // initially set the value to NULL to save space. 
													   // If/when this node is explored again this value will be filled
	}

	// here we can start the async exploit thread
	pthread_t thread;
	bool threadUsed = false;
	if (moves->size() > 0) {
		numTrials++;
		UtilityNode<MoveComponents> asyncMoveNode = selectExploitMove(board);
		MoveComponents asyncMoveComponents = asyncMoveNode.object;
		board.copyInto(*sharedData->boardCopy);
		board.applyMove(asyncMoveComponents.move, playerID, *nextRoundBoard, *sharedData->moveResultBoardCopy);

		// we need to guarantee that the childState is non-NULL before asyncExploitMove can be called
		CMABState2 *childState = (*childrenStates)[asyncMoveComponents.move];
		if (childState == NULL) {
			childState = new CMABState2(*sharedData->moveResultBoardCopy, sharedData, enemyID, playerID);
			(*childrenStates)[asyncMoveComponents.move] = childState;
		}

		AsyncExploitMoveArgs *asyncExploitMoveArgs = new AsyncExploitMoveArgs(*sharedData->boardCopy, *sharedData->moveResultBoardCopy, 
																			  asyncMoveNode, playerID, enemyID, childState);
		pthread_create(&thread, NULL, &asyncExploitMove, (void*)asyncExploitMoveArgs);
		threadUsed = true;
		//asyncExploitMove((void*)asyncExploitMoveArgs);
		moves->push_back(asyncMoveNode);
	}

	if (isExploreMove) {
		topLevelExploreRound(board, moveResultBoard, moveNode, playerID, enemyID);
	}
	else {
		topLevelExploitRound(board, moveResultBoard, moveNode, playerID, enemyID);
	}

	moves->push_back(moveNode);
	if (threadUsed) pthread_join(thread, NULL);
}

float CMABState2::CMABRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID) {
	numTrials++;
	assert(isCorrectBoard(board, playerID));
	if (board.gameIsOver()) {
		return sharedData->evaluator->evaluate(board, playerID, enemyID);
	}
	assert(targets->size() > 0);

	float rand = sharedData->uniformRealDistribution(sharedData->generator);
	if (moves->size() == 0 || rand > sharedData->greediness) {
		return exploreRound(board, emptyBoard, playerID, enemyID);
	}
	else {
		return exploitRound(board, emptyBoard, playerID, enemyID);
	}
}

float CMABState2::exploreRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID) {
	// select which move to explore
	int targetIndex = sharedData->coordinateMAB->getChoice(*targets, numTrials);
	UtilityNode<Coordinate> *targetNode = (*targets)[targetIndex];
	MoveComponents moveComponents;
	if (board.getCoordinateType(targetNode->object) == '.') {
		// the move will be a sacrifice move
		int sacrificeIndex1 = sharedData->coordinateMAB->getChoice(*sacrifices, numTrials);
		UtilityNode<Coordinate> *sacrificeNode1 = remove(*sacrifices, sacrificeIndex1); // remove it so we don't choose it twice
		int sacrificeIndex2 = sharedData->coordinateMAB->getChoice(*sacrifices, numTrials);
		UtilityNode<Coordinate> *sacrificeNode2 = (*sacrifices)[sacrificeIndex2];
		sacrifices->push_back(sacrificeNode1);

		moveComponents = MoveComponents(targetNode, sacrificeNode1, sacrificeNode2);
	}
	else {
		// the move will be a kill move
		moveComponents = MoveComponents(targetNode);
	}

	if (childrenStates->count(moveComponents.move) != 0) {
		// this move has already been explored, so just use this round as an exploit round
		return exploitRound(board, moveResultBoard, playerID, enemyID);
	}

	// evaluate the move
	assert(board.isLegal(moveComponents.move, playerID));
	assert(childrenStates->count(moveComponents.move) == 0);
	board.applyMove(moveComponents.move, playerID, *nextRoundBoard, moveResultBoard);
	float moveEvaluation = sharedData->evaluator->evaluate(moveResultBoard, playerID, enemyID);
	UtilityNode<MoveComponents> moveNode(moveComponents);
	moveNode.updateUtility(moveEvaluation);
	moveComponents.updateUtilities(moveEvaluation);
	moves->push_back(moveNode);
	(*childrenStates)[moveComponents.move] = NULL; // initially set the value to NULL to save space. 
												   // If/when this node is explored again this value will be filled
	return moveEvaluation;
}

float CMABState2::exploitRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID) {
	// select which move to exploit
	assert(moves->size() > 0);
	int moveNodeIndex = sharedData->moveMAB->getChoice(*moves, numTrials);
	UtilityNode<MoveComponents> &moveNode = (*moves)[moveNodeIndex];

	// evaluate the move
	assert(board.isLegal(moveNode.object.move, playerID));
	Move move = moveNode.object.move;
	board.applyMove(move, playerID, *nextRoundBoard, moveResultBoard);

	assert(childrenStates->find(moveNode.object.move) != childrenStates->end());
	CMABState2 *childState = (*childrenStates)[move];
	if (childState == NULL) {
		childState = new CMABState2(moveResultBoard, sharedData, enemyID, playerID);
		(*childrenStates)[move] = childState;
	}
	float moveEvaluation = 1 - childState->CMABRound(moveResultBoard, board, enemyID, playerID);
	moveNode.updateUtility(moveEvaluation);
	moveNode.object.updateUtilities(moveEvaluation);
	return moveEvaluation;
}

Move CMABState2::getBestMove() {
	assert(moves->size() > 0);
	UtilityNode<MoveComponents> bestMoveNode = (*moves)[0];
	int highestCount = bestMoveNode.numTrials;
	for (int i = 1; i < moves->size(); i++) {
		UtilityNode<MoveComponents> moveNode = (*moves)[i];
		if (moveNode.numTrials > highestCount) {
			bestMoveNode = moveNode;
			highestCount = moveNode.numTrials;
		}
	}
	return bestMoveNode.object.move;
}

void CMABState2::printTree(int depth) {
	for (UtilityNode<MoveComponents> moveNode : *moves) {
		for (int i = 0; i < depth; i++) {
			cerr << ".\t";
		}
		cerr << moveNode.numTrials << "," << moveNode.getAverageUtility() << "," << moveNode.object.move.toString() << "\n";
		CMABState2 *child = (*childrenStates)[moveNode.object.move];
		if (child != NULL) {
			child->printTree(depth + 1);
		}
	}
	if (depth == 0) {
		cerr << "---------------------------------\n";
	}
}