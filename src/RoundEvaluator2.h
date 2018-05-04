
#ifndef ROUNDEVALUATOR2_h
#define ROUNDEVALUATOR2_h

#include "Evaluator.h"
#include <math.h>

class RoundEvaluator2 : public Evaluator {
private:
	Board *emptyBoard;
	Board *emptyBoard2;

public:

	RoundEvaluator2() {
		this->emptyBoard = new Board(18, 16);
	}

	~RoundEvaluator2() {
		delete emptyBoard;
	}

	float evaluate(Board &board, Player playerID, Player enemyID) {
		board.setNextRoundBoard(*emptyBoard);
		float playerCells = board.getPlayerCellCount(playerID) + 2 * emptyBoard->getPlayerCellCount(playerID);
		float enemyCells = board.getPlayerCellCount(enemyID) + 2 * emptyBoard->getPlayerCellCount(enemyID);
		return playerCells / (playerCells + enemyCells);
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "EmptyEvalator2()";
		return stringStream.str();
	}
};

#endif