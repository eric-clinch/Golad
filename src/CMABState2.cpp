
#include "CMABState2.h"
#include "CMABState2Manager.h"

using namespace std;

struct CMABState2::SharedData {
	Evaluator *evaluator;
	MAB<Coordinate> *coordinateMAB;
	MAB<MoveComponents> *moveMAB;
	float nonRootGreed;
	default_random_engine generator;
	uniform_real_distribution<float> uniformRealDistribution;
	CMABState2Manager *stateManager;

	SharedData() {};

	SharedData(Evaluator *e, MAB<Coordinate> *cMAB, MAB<MoveComponents> *mMAB, float greed, CMABState2Manager *stateManager) {
		evaluator = e;
		coordinateMAB = cMAB;
		moveMAB = mMAB;
		nonRootGreed = greed;

		generator = default_random_engine();
		random_device rd;
		generator.seed(rd());
		uniform_real_distribution<float> uniformRealDistribution(0.0, 1.0);
		this->stateManager = stateManager;
	}
};

void CMABState2::setStateManager(CMABState2Manager *stateManager) {
	this->sharedData->stateManager = stateManager;
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

void CMABState2::getTargetsAndSacrifices(Board &board, Player playerID, Player enemyID, bool allocateMemory) {
	int width = board.getWidth();
	int height = board.getHeight();
	int numPlayerCells = board.getPlayerCellCount(playerID);

	if (allocateMemory) {
		this->targets = new vector<UtilityNode<Coordinate>*>();
		this->sacrifices = new vector<UtilityNode<Coordinate>*>();
	}

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Coordinate c = Coordinate(x, y);
			char type = board.getCoordinateType(x, y);
			if (type == playerID) {
				UtilityNode<Coordinate> *cNode = coordinateNodes[x * height + y];
				cNode->repurpose(c);
				sacrifices->push_back(cNode);
			}
			else {
				if (type == enemyID || numPlayerCells > 2) {
					// if the cell is an enemy cell, then the move is a kill move which is always legal
					// otherwise the move is a sacrifice move, which is only legal/viable when the
					// number of player cells is greater than 2
					UtilityNode<Coordinate> *cNode = coordinateNodes[x * height + y];
					cNode->repurpose(c);
					targets->push_back(cNode);
				}
			}
		}
	}
	//this->targets->shrink_to_fit();
	//this->sacrifices->shrink_to_fit();
}

CMABState2::CMABState2(Board &board, SharedData *sharedData, Player playerID, Player enemyID) {
	this->sharedData = sharedData;
	this->moves = new vector<UtilityNode<MoveComponents>>();
	this->childrenStates = new unordered_map<Move, CMABState2*>();
	this->nextRoundBoard = board.getNextRoundBoard();
	this->numTrials = 0;
	this->greediness = sharedData->nonRootGreed;

	int cells = board.getHeight() * board.getWidth();
	this->coordinateNodes = new UtilityNode<Coordinate>*[cells];
	for (int i = 0; i < cells; i++) coordinateNodes[i] = new UtilityNode<Coordinate>();
	getTargetsAndSacrifices(board, playerID, enemyID, true);

	assert(isCorrectBoard(board, playerID));
}

CMABState2::CMABState2(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *moveMAB, float nonRootGreed,
					   Player playerID, Player enemyID, CMABState2Manager *stateManager) {
	sharedData = new SharedData(evaluator, coordinateMAB, moveMAB, nonRootGreed, stateManager);
	this->moves = new vector<UtilityNode<MoveComponents>>();
	this->childrenStates = new unordered_map<Move, CMABState2*>();
	this->nextRoundBoard = board.getNextRoundBoard();
	this->numTrials = 0;
	this->greediness = nonRootGreed;

	int cells = board.getHeight() * board.getWidth();
	this->coordinateNodes = new UtilityNode<Coordinate>*[cells];
	for (int i = 0; i < cells; i++) coordinateNodes[i] = new UtilityNode<Coordinate>();
	getTargetsAndSacrifices(board, playerID, enemyID, true);

	assert(isCorrectBoard(board, playerID));
}

void CMABState2::repurposeNode(Board &board, Player playerID, Player enemyID) {
	targets->clear();
	sacrifices->clear();
	childrenStates->clear();
	moves->clear();

	board.setNextRoundBoard(*this->nextRoundBoard);
	this->numTrials = 0;
	this->greediness = sharedData->nonRootGreed; // this might not be necessary
	getTargetsAndSacrifices(board, playerID, enemyID, false);
	assert(isCorrectBoard(board, playerID));
}

CMABState2::~CMABState2() {
	for (int i = 0; i < 16 * 18; i++) {
		delete coordinateNodes[i];
	}
	delete coordinateNodes;

	delete targets;
	delete sacrifices;
	delete moves;

	delete childrenStates;
	delete nextRoundBoard;
}

void CMABState2::freeShared() {
	delete sharedData;
}

float CMABState2::CMABRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID) {
	numTrials++;
	assert(isCorrectBoard(board, playerID));
	if (board.gameIsOver()) {
		return sharedData->evaluator->evaluate(board, playerID, enemyID);
	}
	assert(targets->size() > 0);

	float rand = sharedData->uniformRealDistribution(sharedData->generator);
	if (moves->size() == 0 || rand > greediness) {
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

float CMABState2::exploreRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID) {
	// select the move to explore
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

	return exploreMove(board, moveResultBoard, playerID, enemyID, moveComponents);
}

float CMABState2::exploreMove(Board &board, Board &moveResultBoard, Player playerID, Player enemyID, MoveComponents &moveComponents) {
	assert(board.isLegal(moveComponents.move, playerID));
	assert(childrenStates->count(moveComponents.move) == 0);
	board.applyMove(moveComponents.move, playerID, *nextRoundBoard, moveResultBoard);
	UtilityNode<MoveComponents> moveNode(moveComponents);
	float moveEvaluation = sharedData->evaluator->evaluate(moveResultBoard, playerID, enemyID);
	moveNode.updateUtility(moveEvaluation);
	moveComponents.updateUtilities(moveEvaluation);
	moves->push_back(moveNode);
	(*childrenStates)[moveComponents.move] = NULL; // initially set the value to NULL to save space. 
												   // If/when this node is explored again this value will be filled
	return moveEvaluation;
}

float CMABState2::exploitRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID) {
	return exploitRound(board, moveResultBoard, playerID, enemyID, sharedData->moveMAB);
}

float CMABState2::exploitRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID, MAB<MoveComponents> *moveMAB) {
	assert(moves->size() > 0);
	int moveNodeIndex = moveMAB->getChoice(*moves, numTrials);
	UtilityNode<MoveComponents> &moveNode = (*moves)[moveNodeIndex];

	assert(board.isLegal(moveNode.object.move, playerID));
	Move move = moveNode.object.move;
	board.applyMove(move, playerID, *nextRoundBoard, moveResultBoard);

	assert(childrenStates->find(moveNode.object.move) != childrenStates->end());
	CMABState2 *childState = (*childrenStates)[move];
	if (childState == NULL) {
		childState = sharedData->stateManager->getState(moveResultBoard, sharedData, enemyID, playerID);
		(*childrenStates)[move] = childState;
	}
	float moveEvaluation = 1 - childState->CMABRound(moveResultBoard, board, enemyID, playerID);
	moveNode.updateUtility(moveEvaluation);
	moveNode.object.updateUtilities(moveEvaluation);

	return moveEvaluation;
}

Move CMABState2::getBestMove(float *bestScore, Board &board) {
	assert(moves->size() > 0);
	UtilityNode<MoveComponents> bestMoveNode = (*moves)[0];
	int highestCount = bestMoveNode.numTrials;
	float highestWinrate = bestMoveNode.getAverageUtility();
	for (int i = 1; i < moves->size(); i++) {
		UtilityNode<MoveComponents> moveNode = (*moves)[i];
		if (moveNode.numTrials > highestCount) {
			bestMoveNode = moveNode;
			highestCount = moveNode.numTrials;
			highestWinrate = moveNode.getAverageUtility();
		}
	}
	if (bestScore != NULL) *bestScore = highestCount;
	return bestMoveNode.object.move;
}

Move CMABState2::getBestMove(float *bestScore, CMABState2 *other, Board &board) {
	unordered_map<Move, int> moveScores;
	int highestScore = 0;
	Move bestMove;

	for (int i = 0; i < moves->size(); i++) {
		UtilityNode<MoveComponents> moveNode = (*moves)[i];
		Move move = moveNode.object.move;
		int moveScore = moveNode.numTrials;
		moveScores[move] = moveScore;
		if (moveScore > highestScore) {
			highestScore = moveScore;
			bestMove = move;
		}
	}

	for (int i = 0; i < other->moves->size(); i++) {
		UtilityNode<MoveComponents> moveNode = (*other->moves)[i];
		Move move = moveNode.object.move;
		int moveScore = moveNode.numTrials;
		if (moveScores.count(move) != 0) moveScore += moveScores[move];
		if (moveScore > highestScore) {
			highestScore = moveScore;
			bestMove = move;
		}
	}

	*bestScore = highestScore;
	return bestMove;

	//float score1;
	//Move move1 = getBestMove(&score1, board);
	//float score2;
	//Move move2 = getBestMove(&score2, board);

	//*bestScore = score1 > score2 ? score1 : score2;
	//return move1;
}

int CMABState2::getMovesExplored() {
	return moves->size();
}

void CMABState2::setGreed(float greed) {
	this->greediness = greed;
}

void CMABState2::printTree(int depth) {
	for (UtilityNode<MoveComponents> moveNode : *moves) {
		for (int i = 0; i < depth; i++) {
			cerr << ".\t";
		}
		cerr << moveNode.numTrials << ", " << moveNode.getAverageUtility() << ", " << moveNode.object.move.toString() << "\n";
		CMABState2 *child = (*childrenStates)[moveNode.object.move];
		if (child != NULL) {
			child->printTree(depth + 1);
		}
	}
	if (depth == 0) {
		cerr << "---------------------------------\n";
	}
}

void CMABState2::doAnalysis(CMABState2 *other) {
	sort(targets->begin(), targets->end(), Tools::UtilityNodePointerComparator<Coordinate>);
	sort(other->targets->begin(), other->targets->end(), Tools::UtilityNodePointerComparator<Coordinate>);

	for (int i = 0; i < 5 && i < targets->size(); i++) {
		cerr << (*targets)[i]->object.toString() << " score: " << (*targets)[i]->numTrials << "\n";
	}
	cerr << "\n";
	for (int i = 0; i < 5 && i < other->targets->size(); i++) {
		cerr << (*other->targets)[i]->object.toString() << " score: " << (*other->targets)[i]->numTrials << "\n";
	}
	cerr << "\n\n";
}

vector<Move> CMABState2::getTopMoves(int numMoves) {
	sort(moves->begin(), moves->end(), Tools::UtilityNodeComparator<MoveComponents>);
	vector<Move> topMoves;
	for(int i = 0; i < numMoves && i < moves->size(); i++) {
		topMoves.push_back((*moves)[i].object.move);
	}
	return topMoves;
}

MoveComponents moveToMoveComponents(Move move, unordered_map<Coordinate, UtilityNode<Coordinate>*> coordinateNodes) {
	if (move.MoveType == BIRTH) {
		UtilityNode<Coordinate> *targetNode = coordinateNodes[move.target];
		UtilityNode<Coordinate> *sacrificeNode1 = coordinateNodes[move.sacrifice1];
		UtilityNode<Coordinate> *sacrificeNode2 = coordinateNodes[move.sacrifice2];
		return MoveComponents(targetNode, sacrificeNode1, sacrificeNode2);
	}
	else {
		UtilityNode<Coordinate> *targetNode = coordinateNodes[move.target];
		return MoveComponents(targetNode);
	}
}

void CMABState2::setMoves(vector<Move> topMoves, Board &board, Board &emptyBoard, Player playerID, Player enemyID) {
	// maps each coordinate to its corresponding utility node
	unordered_map<Coordinate, UtilityNode<Coordinate>*> coordinateNodes;
	for (int i = 0; i < sacrifices->size(); i++) {
		coordinateNodes[(*sacrifices)[i]->object] = (*sacrifices)[i];
	}
	for (int i = 0; i < targets->size(); i++) {
		coordinateNodes[(*targets)[i]->object] = (*targets)[i];
	}

	// remove all moves other than the top moves
	unordered_set<Move> topMovesSet;
	for (int i = 0; i < topMoves.size(); i++) {
		topMovesSet.emplace(topMoves[i]);
	}
	vector<UtilityNode<MoveComponents>> topMoveComponents;
	for (int i = 0; i < moves->size(); i++) {
		if (topMovesSet.count((*moves)[i].object.move) != 0) topMoveComponents.push_back((*moves)[i]);
	}
	moves->clear();
	for (int i = 0; i < topMoveComponents.size(); i++) {
		moves->push_back(topMoveComponents[i]);
	}

	// store the children states corresponding to the top moves
	vector<CMABState2*> newChildren;
	for (int i = 0; i < topMoves.size(); i++) {
		Move move = topMoves[i];
		MoveComponents moveComponents = moveToMoveComponents(move, coordinateNodes);
		if (childrenStates->count(move) == 0) {
			exploreMove(board, emptyBoard, playerID, enemyID, moveComponents);
		}
		assert(childrenStates->count(move) != 0);

		newChildren.push_back((*childrenStates)[move]);
	}

	// remove all the children states, and then readd the ones 
	// corresponding to the top moves
	childrenStates->clear();
	for (int i = 0; i < newChildren.size(); i++) {
		(*childrenStates)[topMoves[i]] = newChildren[i];
	}
}