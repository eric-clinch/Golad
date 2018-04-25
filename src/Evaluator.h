
#ifndef EVALUATOR_h
#define EVALUATOR_h

#include "Board.h"

class Evaluator {
public:
	virtual float evaluate(Board &board, Player playerID, Player enemyID) = 0;
	virtual string toString() { return "Evaluator"; }
};

#endif