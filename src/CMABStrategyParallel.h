
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

#ifndef CMABStrategyParallel_h
#define CMABStrategyParallel_h

#include <pthread.h>
#include <unordered_set>
#include "Strategy.h"
#include "Evaluator.h"
#include "MAB.h"
#include "CMABStateParallel.h"
#include "CMABStateParallelManager.h"
#include "MoveComponents.h"
#include "Tools.h"

class CMABStrategyParallel : public Strategy {
private:
	Evaluator * evaluator;
	MAB<MoveComponents> *moveMAB;
	MAB<MoveComponents> *secondaryMoveMAB;
	MAB<Coordinate> *coordinateMAB;
	float greediness;
	float alpha;
	float exploreTimeRatio;
	int topMoveNum;
	long previousTimeEnd;
	static void *developTree(void *arg);
	static void *exploitTree(void *arg);
	CMABStateParallelManager *stateManager1;
	CMABStateParallelManager *stateManager2;

public:
	CMABStrategyParallel(Evaluator *evaluator, MAB<MoveComponents> *moveMAB, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *secondaryMoveMAB,
				  float greediness, float alpha, float exploreTimeRatio, int topMoveNum);
	~CMABStrategyParallel();
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round);
	virtual void cleanUp();
	virtual string toString();
};

#endif