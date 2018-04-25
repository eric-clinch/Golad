
#ifndef RATIOEVALUATOR_h
#define RATIOEVALUATOR_h

#include "Evaluator.h"

class RatioEvaluator : public Evaluator {
public:

	float evaluate(Board &board, Player playerID, Player enemyID) {
		float numerator = board.getPlayerCellCount(playerID);
		float denominator = numerator + board.getPlayerCellCount(enemyID);
		return numerator / denominator;
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "RatioEvalator()";
		return stringStream.str();
	}
};

#endif