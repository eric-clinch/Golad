
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

#ifndef CMABSTATE_h
#define CMABSTATE_h

#include <vector>
#include <unordered_map>
#include <random>
#include "UtilityNode.h"
#include "Coordinate.h"
#include "Move.h";
#include "Evaluator.h"
#include "MAB.h"

using namespace std;

struct SharedData {
	Evaluator *evaluator;
	MAB<Coordinate> *coordinateMAB;
	MAB<Move> *moveMAB;
	float greediness;
	default_random_engine generator;
	uniform_real_distribution<float> uniformRealDistribution;

	SharedData() {};

	SharedData(Evaluator *e, MAB<Coordinate> *cMAB, MAB<Move> *mMAB, float greed) {
		evaluator = e;
		coordinateMAB = cMAB;
		moveMAB = mMAB;
		greediness = greed;

		generator = default_random_engine();
		random_device rd;
		generator.seed(rd());
		uniform_real_distribution<float> uniformRealDistribution(0.0, 1.0);
	}
};

class CMABState {
private:
	struct CoordinateAndType;

	SharedData *sharedData;
	vector<UtilityNode<Coordinate>> *targets;
	vector<UtilityNode<Coordinate>> *sacrifices;
	vector<UtilityNode<Move>> *moves;
	unordered_map<Move, CMABState*> *childrenStates;
	int numTrials;

	void getTargetsAndSacrifices(Board &board, Player playerID, Player enemyID); // constructor helper

	bool isCorrectBoard(Board &board, Player playerID);
	float exploreRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID);
	float exploreMove(Board &board, Board &nextRoundBoard, Move &move, Player playerID, Player enemyID);
	float exploitRound(Board &board, Board &nextRoundBoard, Player playerID, Player enemyID);

public:
	CMABState(Board &board, SharedData *sharedData, Player playerID, Player enemyID);
	CMABState(Board &board, Evaluator *evaluator, MAB<Coordinate> *coordinateMAB, MAB<Move> *moveMAB, float greediness, 
			  Player playerID, Player enemyID);
	~CMABState();
	void freeShared(); // a secondary deconstructor that should only be called on one of the CMABStates in the state tree when freeing the game tree

	// is a destructive function on both the board and empty board given
	float CMABRound(Board &board, Board &emptyBoard, Player playerID, Player enemyID);
	Move getBestMove();

	void printTree(int depth = 0);
};

#endif