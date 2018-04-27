
#include "CMABStateParallel.h"
#include "CMABStateParallelManager.h"

using namespace std;

struct CMABStateParallel::SharedData {
	Evaluator *evaluator;
	MAB<Coordinate> *coordinateMAB;
	MAB<MoveComponents> *moveMAB;
	float nonRootGreed;
	default_random_engine generator;
	uniform_real_distribution<float> uniformRealDistribution;
	CMABStateParallelManager *stateManager;

	SharedData() {};

	SharedData(Evaluator *e, MAB<Coordinate> *cMAB, MAB<MoveComponents> *mMAB, float greed, CMABStateParallelManager *stateManager) {
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

void CMABStateParallel::setStateManager(CMABStateParallelManager *stateManager) {
	this->sharedData->stateManager = stateManager;
}

// returns whether the given board matches what we believe it should look like.
// Each CMABStateParallel should have a unique board associated with it, and this checks
// whether or not the given board is that unique board.
bool CMABStateParallel::isCorrectBoard(Board &board, Player playerID) {
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

UtilityNode<Coordinate> *CMABStateParallel::getCoordinateNode(int index, Coordinate &coordinate) {
	if (index < coordinateNodes->size()) {
		UtilityNode<Coordinate> *result = (*coordinateNodes)[index];
		result->repurpose(coordinate);
		return result;
	}
	else {
		UtilityNode<Coordinate> *result = new UtilityNode<Coordinate>(coordinate);
		coordinateNodes->push_back(result);
		return result;
	}
}

MoveComponents CMABStateParallel::getTargetsAndSacrifices(Board &board, Player playerID, Player enemyID, bool *returnIsValid) {
	int width = board.getWidth();
	int height = board.getHeight();
	int numPlayerCells = board.getPlayerCellCount(playerID);

	if (lazySearchIndex >= width * height) {
		*returnIsValid = false;
		return MoveComponents();
	}

	*returnIsValid = true;
	UtilityNode<Coordinate> *sacrifice1 = NULL;
	UtilityNode<Coordinate> *sacrifice2 = NULL;
	UtilityNode<Coordinate> *birthTarget = NULL;

	while (lazySearchIndex < width * height) {
		int x = lazySearchIndex / height;
		int y = lazySearchIndex % height;
		lazySearchIndex++;

		Coordinate c = Coordinate(x, y);
		char type = board.getCoordinateType(x, y);
		if (type == playerID) {
			UtilityNode<Coordinate> *cNode = getCoordinateNode(lazySearchIndex - 1, c);
			sacrifices->push_back(cNode);
			if (sacrifice1 == NULL) sacrifice1 = cNode;
			else if (sacrifice2 == NULL) sacrifice2 = cNode;
			else if (birthTarget != NULL) return MoveComponents(birthTarget, sacrifice1, sacrifice2);
		}
		else {
			if (type == enemyID) {
				UtilityNode<Coordinate> *cNode = getCoordinateNode(lazySearchIndex - 1, c);
				targets->push_back(cNode);
				return MoveComponents(cNode);
			}
			else if (numPlayerCells > 2) {
				// The move is a sacrifice move, which is only legal/viable when the
				// number of player cells is greater than 2
				UtilityNode<Coordinate> *cNode = getCoordinateNode(lazySearchIndex - 1, c);
				targets->push_back(cNode);
				birthTarget = cNode;
				if (sacrifice2 != NULL) return MoveComponents(birthTarget, sacrifice1, sacrifice2);
			}
		}
	}

	*returnIsValid = false;
	return MoveComponents();
}

CMABStateParallel::CMABStateParallel(Board &board, SharedData *sharedData, Player playerID, Player enemyID) {
	this->sharedData = sharedData;
	this->moves = new vector<UtilityNode<MoveComponents>>();
	this->childrenStates = new unordered_map<Move, CMABStateParallel*>();
	this->nextRoundBoard = board.getNextRoundBoard();
	this->numTrials = 0;
	this->greediness = sharedData->nonRootGreed;
	this->coordinateNodes = new vector<UtilityNode<Coordinate>*>();
	this->sacrifices = new vector<UtilityNode<Coordinate>*>();
	this->targets = new vector<UtilityNode<Coordinate>*>();
	this->lazySearchIndex = 0;

	assert(isCorrectBoard(board, playerID));
}

CMABStateParallel::CMABStateParallel(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *moveMAB, float nonRootGreed,
	Player playerID, Player enemyID, CMABStateParallelManager *stateManager) {
	sharedData = new SharedData(evaluator, coordinateMAB, moveMAB, nonRootGreed, stateManager);
	this->moves = new vector<UtilityNode<MoveComponents>>();
	this->childrenStates = new unordered_map<Move, CMABStateParallel*>();
	this->nextRoundBoard = board.getNextRoundBoard();
	this->numTrials = 0;
	this->greediness = nonRootGreed;
	this->coordinateNodes = new vector<UtilityNode<Coordinate>*>();
	this->sacrifices = new vector<UtilityNode<Coordinate>*>();
	this->targets = new vector<UtilityNode<Coordinate>*>();
	this->lazySearchIndex = 0;

	assert(isCorrectBoard(board, playerID));
}

void CMABStateParallel::repurposeNode(Board &board, Player playerID, Player enemyID) {
	targets->clear();
	sacrifices->clear();

	childrenStates->clear();
	moves->clear();

	board.setNextRoundBoard(*this->nextRoundBoard);
	this->numTrials = 0;
	this->greediness = sharedData->nonRootGreed; // this might not be necessary
	this->lazySearchIndex = 0;
	assert(isCorrectBoard(board, playerID));
}

CMABStateParallel::~CMABStateParallel() {
	for (int i = 0; i < coordinateNodes->size(); i++) {
		delete (*coordinateNodes)[i];
	}
	delete coordinateNodes;

	delete targets;
	delete sacrifices;

	delete moves;

	delete childrenStates;
	delete nextRoundBoard;
}

void CMABStateParallel::freeShared() {
	delete sharedData;
}

float CMABStateParallel::CMABRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID) {
	numTrials++;
	assert(isCorrectBoard(board, playerID));
	if (board.gameIsOver()) {
		return sharedData->evaluator->evaluate(board, playerID, enemyID);
	}

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

float CMABStateParallel::exploreRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID) {
	// select the move to explore
	bool moveIsValid;
	MoveComponents moveComponents = getTargetsAndSacrifices(board, playerID, enemyID, &moveIsValid);

	if (!moveIsValid) {
		int targetIndex = sharedData->coordinateMAB->getChoice(*targets, numTrials);
		UtilityNode<Coordinate> *targetNode = (*targets)[targetIndex];

		if (board.getCoordinateType(targetNode->object) == Board::EMPTY) {
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
	}
	return exploreMove(board, moveResultBoard, playerID, enemyID, moveComponents);
}

float CMABStateParallel::exploreMove(Board &board, Board &moveResultBoard, Player playerID, Player enemyID, MoveComponents &moveComponents) {
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

float CMABStateParallel::exploitRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID) {
	return exploitRound(board, moveResultBoard, playerID, enemyID, sharedData->moveMAB);
}

float CMABStateParallel::exploitRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID, MAB<MoveComponents> *moveMAB) {
	assert(moves->size() > 0);
	int moveNodeIndex = moveMAB->getChoice(*moves, numTrials);
	UtilityNode<MoveComponents> &moveNode = (*moves)[moveNodeIndex];

	assert(board.isLegal(moveNode.object.move, playerID));
	Move move = moveNode.object.move;
	board.applyMove(move, playerID, *nextRoundBoard, moveResultBoard);

	assert(childrenStates->find(moveNode.object.move) != childrenStates->end());
	CMABStateParallel *childState = (*childrenStates)[move];
	if (childState == NULL) {
		childState = sharedData->stateManager->getState(moveResultBoard, sharedData, enemyID, playerID);
		(*childrenStates)[move] = childState;
	}
	float moveEvaluation = 1 - childState->CMABRound(moveResultBoard, board, enemyID, playerID);
	moveNode.updateUtility(moveEvaluation);
	moveNode.object.updateUtilities(moveEvaluation);

	return moveEvaluation;
}

Move CMABStateParallel::getBestMove(float *bestScore, Board &board) {
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

Move CMABStateParallel::getBestMove(float *bestScore, CMABStateParallel *other, Board &board) {
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

int CMABStateParallel::getMovesExplored() {
	return moves->size();
}

void CMABStateParallel::setGreed(float greed) {
	this->greediness = greed;
}

void CMABStateParallel::printTree(int depth) {
	for (UtilityNode<MoveComponents> moveNode : *moves) {
		for (int i = 0; i < depth; i++) {
			cerr << ".\t";
		}
		cerr << moveNode.numTrials << ", " << moveNode.getAverageUtility() << ", " << moveNode.object.move.toString() << "\n";
		CMABStateParallel *child = (*childrenStates)[moveNode.object.move];
		if (child != NULL) {
			child->printTree(depth + 1);
		}
	}
	if (depth == 0) {
		cerr << "---------------------------------\n";
	}
}

void CMABStateParallel::doAnalysis(CMABStateParallel *other) {
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

vector<Move> CMABStateParallel::getTopMoves(int numMoves) {
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

void CMABStateParallel::setMoves(vector<Move> topMoves, Board &board, Board &emptyBoard, Player playerID, Player enemyID) {
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
	vector<CMABStateParallel*> newChildren;
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