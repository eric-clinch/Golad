#ifndef STRATEGY_h
#define STRATEGY_h

#include "Move.h"
#include "Board.h"

class Strategy {
public:
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int totalTime, int timePerMove, int round) = 0;
};

#endif