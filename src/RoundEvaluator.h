

#ifndef ROUNDEVALUATOR_h
#define ROUNDEVALUATOR_h

#include "Evaluator.h"
#include <math.h>

class RoundEvaluator : public Evaluator {
private:
	Board * emptyBoard;

public:

	RoundEvaluator() {
		this->emptyBoard = new Board(18, 16);
	}

	~RoundEvaluator() {
		delete emptyBoard;
	}

	float evaluate(Board &board, Player playerID, Player enemyID) {
		board.setNextRoundBoard(*emptyBoard);
		float playerCells = emptyBoard->getPlayerCellCount(playerID) + board.getPlayerCellCount(playerID);
		float enemyCells = emptyBoard->getPlayerCellCount(enemyID) + board.getPlayerCellCount(enemyID);
		return playerCells / (playerCells + enemyCells);
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "RoundEvalator()";
		return stringStream.str();
	}
};

#endif