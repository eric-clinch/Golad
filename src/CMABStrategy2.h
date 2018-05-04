
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

#ifndef CMABStrategy2_h
#define CMABStrategy2_h

#include <pthread.h>
#include "Strategy.h"
#include "Evaluator.h"
#include "MAB.h"
#include "CMABState2.h"
#include "CMABState2Manager.h"
#include "MoveComponents.h"
#include "Tools.h"
#include "BoardDensity.h"
#include "RatioEvaluator.h"

class CMABStrategy2 : public Strategy {
private:
	int lowerMoveBound;
	int upperMoveBound;

	Evaluator * evaluator;
	MAB<MoveComponents> *moveMAB;
	MAB<Coordinate> *coordinateMAB;
	float greediness;
	float alpha;
	long previousTimeEnd;
	static void *freeTree(void *arg);
	static void *freeTrees(void *arg);
	CMABState2Manager *stateManager1;


public:
	CMABStrategy2(Evaluator *evaluator, MAB<MoveComponents> *moveMAB, MAB<Coordinate> *coordinateMAB, 
				  float greediness, float alpha, int lowerMovesBound, int uppoerMovesBound);
	~CMABStrategy2();
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round);
	virtual void cleanUp();
	virtual string toString();
};

#endif