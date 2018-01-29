#ifndef STRATEGY_h
#define STRATEGY_h

#include <algorithm>
#include "Move.h"
#include "Board.h"
#include "Enums.h"

using namespace std;

class Strategy
{
public:
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int totalTime, int timePerMove, int round) = 0;
};

#endif