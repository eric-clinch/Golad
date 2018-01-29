
#ifdef _DEBUG
	#include <stdlib.h>
	#include <crtdbg.h>
	#define _CRTDBG_MAP_ALLOC
	#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
#endif

#ifndef RANDOMSTRATEGY_h
#define RANDOMSTRATEGY_h

#include <assert.h>
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"

class RandomStrategy : public Strategy
{
private:
	template <class T>
	T RandomElementFromVector(vector<T> &input);
	vector<MoveType> GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID);

public:
	RandomStrategy();
	Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round);
};

#endif