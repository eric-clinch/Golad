
#ifndef EVALUATOR_h
#define EVALUATOR_h

#include "Board.h"

class Evaluator {
public:
	virtual float evaluate(Board b, Player playerID, Player enemyID) = 0;
};

#endif