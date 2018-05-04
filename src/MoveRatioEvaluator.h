
#ifndef MOVERATIOEVALUATOR_h
#define MOVERATIOEVALUATOR_h

#include "Evaluator.h"
#include <math.h>

class MoveRatioEvaluator : public Evaluator {
public:

	float evaluate(Board &board, Player playerID, Player enemyID) {
		float numerator = pow(board.getPlayerCellCount(playerID), 2);
		float denominator = numerator + pow(board.getPlayerCellCount(enemyID), 2);
		return numerator / denominator;
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "RatioEvalator()";
		return stringStream.str();
	}
};

#endif