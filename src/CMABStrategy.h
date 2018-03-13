
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

#ifndef CMABSTRATEGY_h
#define CMABSTRATEGY_h

#include "Strategy.h"
#include "Evaluator.h"
#include "MAB.h"
#include "CMABState.h"
#include "Tools.h"

class CMABStrategy : public Strategy {
private:
	Evaluator *evaluator;
	MAB<Move> *moveMAB;
	MAB<Coordinate> *coordinateMAB;
	float greediness;

public:
	CMABStrategy(Evaluator *evaluator, MAB<Move> *moveMAB, MAB<Coordinate> *coordinateMAB, float greediness);
	~CMABStrategy();
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round);
};

#endif