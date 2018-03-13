
#include "CMABState.h"

using namespace std;

struct CMABState::CoordinateAndType {
	Coordinate coordinate;
	char type;

	CoordinateAndType(Coordinate c, char t) {
		coordinate = c;
		type = t;
	}
};

// returns whether the given board matches what we believe it should look like.
// Each CMABState should have a unique board associated with it, and this checks
// whether or not the given board is that unique board.
bool CMABState::isCorrectBoard(Board &board, Player playerID) {
	for (int i = 0; i < targets->size(); i++) {
		UtilityNode<Coordinate> node = (*targets)[i];
		Coordinate c = node.object;
		// targets can only be enemy cells (in the case of the kill move)
		// or dead cells (in the case of the birth move)
		if (board.getCoordinateType(c) == to_string(playerID).at(0)) {
			return false;
		}
	}
	for (int i = 0; i < sacrifices->size(); i++) {
		UtilityNode<Coordinate> cNode = (*sacrifices)[i];
		Coordinate c = cNode.object;
		if (board.getCoordinateType(c) != to_string(playerID).at(0)) {
			return false;
		}
	}
	return true;
}

void CMABState::getTargetsAndSacrifices(Board &board, Player playerID, Player enemyID) {
	this->targets = new vector<UtilityNode<Coordinate>>();
	this->sacrifices = new vector<UtilityNode<Coordinate>>();

	int width = board.getWidth();
	int height = board.getHeight();
	int numPlayerCells = board.getPlayerCellCount(playerID); // used to determine whether sacrifice moves are legal
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Coordinate c = Coordinate(x, y);
			char type = board.getCoordinateType(x, y);
			if (type == to_string(playerID).at(0)) {
				sacrifices->push_back(c);
			}
			else {
				if (type == to_string(enemyID).at(0) || numPlayerCells > 2) {
					// if the cell is an enemy cell, then the move is a kill move which is always legal
					// otherwise the move is a sacrifice move, which is only legal/viable when the
					// number of player cells is greater than 2
					targets->push_back(c);
				}
			}
		}
	}
	random_shuffle(sacrifices->begin(), sacrifices->end());
	random_shuffle(targets->begin(), targets->end());
}

CMABState::CMABState(Board &board, SharedData *sharedData, Player playerID, Player enemyID) {
	this->sharedData = sharedData;
	this->moves = new vector<UtilityNode<Move>>();
	this->childrenStates = new unordered_map<Move, CMABState*>();
	this->numTrials = 0;
	getTargetsAndSacrifices(board, playerID, enemyID);
	assert(isCorrectBoard(board, playerID));
}

CMABState::CMABState(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<Move> *moveMAB, float greediness,
					 Player playerID, Player enemyID) {
	sharedData = new SharedData(evaluator, coordinateMAB, moveMAB, greediness);
	this->moves = new vector<UtilityNode<Move>>();
	this->childrenStates = new unordered_map<Move, CMABState*>();
	this->numTrials = 0;
	getTargetsAndSacrifices(board, playerID, enemyID);
	assert(isCorrectBoard(board, playerID));
}

CMABState::~CMABState() {
	delete targets;
	delete sacrifices;
	delete moves;

	unordered_map<Move, CMABState*>::iterator it = childrenStates->begin();
	for (; it != childrenStates->end(); it++) {
		delete it->second;
	}

	delete childrenStates;
}

void CMABState::freeShared() {
	free(sharedData);
}

float CMABState::CMABRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID) {
	numTrials++;
	assert(isCorrectBoard(board, playerID));
	if (board.gameIsOver()) {
		return sharedData->evaluator->evaluate(board, playerID, enemyID);
	}
	assert(targets->size() > 0);

	float rand = sharedData->uniformRealDistribution(sharedData->generator);
	if (moves->size() == 0 || rand > sharedData->greediness) {
		// explore case
		return exploreRound(board, emptyBoard, playerID, enemyID);
	}
	else {
		// exploit case
		return exploitRound(board, emptyBoard, playerID, enemyID);
	}
}

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

float CMABState::exploreRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID) {
	int targetIndex = sharedData->coordinateMAB->getChoice(*targets, numTrials);
	UtilityNode<Coordinate> &targetNode = (*targets)[targetIndex];

	if (board.getCoordinateType(targetNode.object) == '.') {
		// the move will be a sacrifice move
		int sacrificeIndex1 = sharedData->coordinateMAB->getChoice(*sacrifices, numTrials);
		UtilityNode<Coordinate> &sacrificeNode1 = remove(*sacrifices, sacrificeIndex1); // remove it so we don't choose it twice
		int sacrificeIndex2 = sharedData->coordinateMAB->getChoice(*sacrifices, numTrials);
		UtilityNode<Coordinate> &sacrificeNode2 = (*sacrifices)[sacrificeIndex2];
		sacrifices->push_back(sacrificeNode1);

		Move move = Move(targetNode.object, sacrificeNode1.object, sacrificeNode2.object);
		if (childrenStates->count(move) != 0) {
			// this move has already been explored, so just use this round as an exploit round
			return exploitRound(board, emptyBoard, playerID, enemyID);
		}
		float moveScore = exploreMove(board, emptyBoard, move, playerID, enemyID);

		targetNode.updateUtility(moveScore);
		sacrificeNode1.updateUtility(moveScore);
		sacrificeNode2.updateUtility(moveScore);
		return moveScore;
	}
	else {
		// the move will be a kill move
		Move move = Move(targetNode.object);
		if (childrenStates->count(move) != 0) {
			// this move has already been explored, so just use this round as an exploit round
			return exploitRound(board, emptyBoard, playerID, enemyID);
		}
		float moveScore = exploreMove(board, emptyBoard, move, playerID, enemyID);

		targetNode.updateUtility(moveScore);
		return moveScore;
	}
}

float CMABState::exploreMove(Board &board, Board &nextRoundBoard, Move &move, Player playerID, Player enemyID) {
	assert(board.isLegal(move, playerID));
	assert(childrenStates->count(move) == 0);
	board.makeMove(move, playerID, nextRoundBoard);
	UtilityNode<Move> moveNode(move);
	float moveEvaluation = sharedData->evaluator->evaluate(nextRoundBoard, playerID, enemyID);
	moveNode.updateUtility(moveEvaluation);
	moves->push_back(moveNode);
	(*childrenStates)[move] = NULL; // initially set the value to NULL to save space. 
									// If/when this node is explored again this value will be filled
	return moveEvaluation;
}

float CMABState::exploitRound(Board &board, Board &nextRoundBoard, Player playerID, Player enemyID) {
	assert(moves->size() > 0);
	int moveNodeIndex = sharedData->moveMAB->getChoice(*moves, numTrials);
	UtilityNode<Move> &moveNode = (*moves)[moveNodeIndex];

	assert(board.isLegal(moveNode.object, playerID));
	board.makeMove(moveNode.object, playerID, nextRoundBoard);

	assert(childrenStates->find(moveNode.object) != childrenStates->end());
	CMABState *childState = (*childrenStates)[moveNode.object];
	if (childState == NULL) {
		childState = new CMABState(nextRoundBoard, sharedData, enemyID, playerID);
		(*childrenStates)[moveNode.object] = childState;
	}
	float moveEvaluation = 1 - childState->CMABRound(nextRoundBoard, board, enemyID, playerID);
	moveNode.updateUtility(moveEvaluation);

	return moveEvaluation;
}

Move CMABState::getBestMove() {
	assert(moves->size() > 0);
	UtilityNode<Move> bestMoveNode = (*moves)[0];
	int highestCount = bestMoveNode.numTrials;
	for (int i = 1; i < moves->size(); i++) {
		UtilityNode<Move> moveNode = (*moves)[i];
		if (moveNode.numTrials > highestCount) {
			bestMoveNode = moveNode;
			highestCount = moveNode.numTrials;
		}
	}
	return bestMoveNode.object;
}

void CMABState::printTree(int depth) {
	for (UtilityNode<Move> moveNode : *moves) {
		for (int i = 0; i < depth; i++) {
			cerr << ".\t";
		}
		cerr << moveNode.numTrials << "," << moveNode.getAverageUtility() << "," << moveNode.object.toString() << "\n";
		CMABState *child = (*childrenStates)[moveNode.object];
		if (child != NULL) {
			child->printTree(depth + 1);
		}
	}
	if (depth == 0) {
		cerr << "---------------------------------\n";
		cerr << "num moves: " << moves->size() << "\n";
	}
}