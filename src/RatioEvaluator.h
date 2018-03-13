
#ifndef RATIOEVALUATOR_h
#define RATIOEVALUATOR_h

#include "Evaluator.h"

class RatioEvaluator : public Evaluator {
private:
	float maxscore = 288.0;

public:
	float evaluate(Board &b, Player playerID, Player enemyID) {
		float res = (float)b.getPlayerCellCount(playerID) / (b.getPlayerCellCount(playerID) + b.getPlayerCellCount(enemyID));
		return res;
	}
};

#endif