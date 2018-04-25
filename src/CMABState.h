
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define NDEBUG
#endif

#ifndef CMABState_h
#define CMABState_h

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include "UtilityNode.h"
#include "Coordinate.h"
#include "Move.h"
#include "Evaluator.h"
#include "MAB.h"
#include "MoveComponents.h"
#include "Tools.h"

using namespace std;

class CMABStateManager;

class CMABState {
	friend class CMABStateManager;
public:
	CMABState(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *moveMAB, float nonRootGreed,
			   Player playerID, Player enemyID, CMABStateManager *stateManager);
	~CMABState();
	void freeShared(); // a secondary deconstructor that should only be called on one of the CMABStates in the state tree when freeing the game tree

	void setStateManager(CMABStateManager *stateManager);
	float CMABRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID); // is a destructive function on both the board and empty board given
	Move getBestMove(float *bestScore, Board &board);
	Move getBestMove(float *bestScore, CMABState *other, Board &board);
	int getMovesExplored();
	void setGreed(float greed);
	void printTree(int depth = 0);
	void doAnalysis(CMABState *other);

private:
	struct SharedData;

	SharedData *sharedData;
	UtilityNode<Coordinate> **coordinateNodes;
	vector<UtilityNode<Coordinate>*> *targets;
	vector<UtilityNode<Coordinate>*> *sacrifices;
	vector<UtilityNode<MoveComponents>> *moves;
	unordered_map<Move, CMABState*> *childrenStates;
	Board *nextRoundBoard;
	int numTrials;
	float greediness;

	CMABState(Board &board, SharedData *sharedData, Player playerID, Player enemyID);
	void repurposeNode(Board &board, Player playerID, Player enemyID);
	void getTargetsAndSacrifices(Board &board, Player playerID, Player enemyID, bool allocateMemory); // constructor helper

	bool isCorrectBoard(Board &board, Player playerID);
	float exploreRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID);
	float exploreMove(Board &board, Board &moveResultBoard, Player playerID, Player enemyID, MoveComponents &moveComponents);
	float exploitRound(Board &board, Board &nextRoundBoard, Player playerID, Player enemyID);
};

#endif