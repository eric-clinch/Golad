
#ifndef RATIOEVALUATOR_h
#define RATIOEVALUATOR_h

#include "Evaluator.h"

class RatioEvaluator : public Evaluator {
private:
	float maxscore = 288.0;

public:
	float evaluate(Board b, Player playerID, Player enemyID) {
		if (b.getPlayerCellCount(playerID) == 0) return -maxscore;
		if (b.getPlayerCellCount(enemyID) == 0) return maxscore;
		return (float)b.getPlayerCellCount(playerID) / b.getPlayerCellCount(enemyID);
	}
};

#endif