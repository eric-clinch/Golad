
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
#include <unordered_set>
#include "Strategy.h"
#include "Evaluator.h"
#include "MAB.h"
#include "CMABState2.h"
#include "CMABState2Manager.h"
#include "MoveComponents.h"
#include "Tools.h"

class CMABStrategy2 : public Strategy {
private:
	static const int maxStates = 10000;
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
	CMABState2Manager *stateManager1;
	CMABState2Manager *stateManager2;

public:
	CMABStrategy2(Evaluator *evaluator, MAB<MoveComponents> *moveMAB, MAB<Coordinate> *coordinateMAB, MAB<MoveComponents> *secondaryMoveMAB,
				  float greediness, float alpha, float exploreTimeRatio, int topMoveNum);
	~CMABStrategy2();
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round);
	virtual void cleanUp();
	virtual string toString();
};

#endif