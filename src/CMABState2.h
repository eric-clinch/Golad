
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

#ifndef CMABState2_h
#define CMABState2_h

#include <vector>
#include <unordered_map>
#include <random>
#include <pthread.h>
#include "UtilityNode.h"
#include "Coordinate.h"
#include "Move.h";
#include "Evaluator.h"
#include "MAB.h"
#include "MoveComponents.h"

using namespace std;

class CMABState2 {
public:
	CMABState2(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *moveMAB, float greediness,
		Player playerID, Player enemyID);
	~CMABState2();
	void freeShared(); // a secondary deconstructor that should only be called on one of the CMABState2s in the state tree when freeing the game tree
					   // is a destructive function on both the board and empty board given
	void topLevelCMABRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID);
	Move getBestMove();

	void printTree(int depth = 0);

private:
	struct SharedData;

	SharedData *sharedData;
	vector<UtilityNode<Coordinate>*> *targets;
	vector<UtilityNode<Coordinate>*> *sacrifices;
	vector<UtilityNode<MoveComponents>> *moves;
	unordered_map<Move, CMABState2*> *childrenStates;
	Board *nextRoundBoard;
	int numTrials;

	CMABState2() {};
	CMABState2(Board &board, SharedData *sharedData, Player playerID, Player enemyID);
	void getTargetsAndSacrifices(Board &board, Player playerID, Player enemyID); // constructor helper

	struct AsyncExploitMoveArgs;
	static void *asyncExploitMove(void *asyncExploitMoveArgs);
	void topLevelExploitRound(Board &board, Board &moveResultBoard, UtilityNode<MoveComponents> &moveNode, Player playerID, Player enemyID);
	void topLevelExploreRound(Board &board, Board &moveResultBoard, UtilityNode<MoveComponents> &moveNode, Player playerID, Player enemyID);
	UtilityNode<MoveComponents> selectMove(Board &board);
	UtilityNode<MoveComponents> selectExploreMove(Board &board);
	UtilityNode<MoveComponents> selectExploitMove(Board &board);

	float CMABRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID);
	bool isCorrectBoard(Board &board, Player playerID);
	float exploreRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID);
	float exploitRound(Board &board, Board &moveResultBoard, Player playerID, Player enemyID);
};

#endif