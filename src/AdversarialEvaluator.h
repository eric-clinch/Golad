
#ifndef ADVERSARIALEVALUATOR_h
#define ADVERSARIALEVALUATOR_h

#include "Evaluator.h"
#include "Tools.h"

class AdversarialEvaluator : public Evaluator {
private:
	int numTrials;
	Board *nextRoundBoard;
	Board *moveBoard;

	inline vector<MoveType> getAvailableMoveTypes(Board &board, Player playerID, Player enemyID) {
		vector<MoveType> availableMoves;

		if (board.getPlayerCellCount(enemyID) > 0) {
			availableMoves.push_back(KILL);
		}

		if (board.getPlayerCellCount(playerID) > 1) {
			availableMoves.push_back(BIRTH);
		}

		assert(availableMoves.size() > 0);
		return availableMoves;
	}

	inline Move getRandomBirth(vector<Coordinate> &deadCells, vector<Coordinate> &myCells) {
		if (myCells.size() < 2) return Move();
		// randomly choose a target dead cell and two of my own cells to sacrifice
		Coordinate target = Tools::GetRandomElementFromVector(deadCells);
		Coordinate sacrifice1 = Tools::PopRandomElementFromVector(myCells); // pop the element so we don't choose the same element twice
		Coordinate sacrifice2 = Tools::GetRandomElementFromVector(myCells);
		myCells.push_back(sacrifice1);
		return Move(target, sacrifice1, sacrifice2);
	}

	inline Move getRandomMove(vector<MoveType> &availableMoveTypes, vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells) {
		assert(availableMoveTypes.size() > 0);
		int moveType = Tools::GetRandomElementFromVector(availableMoveTypes);

		if (moveType == BIRTH) {
			if (myCells.size() < 2) {
				return Move();
			}
			return getRandomBirth(deadCells, myCells);
		}
		else {
			assert(moveType == KILL);
			Coordinate target = Tools::GetRandomElementFromVector(enemyCells);
			return Move(target);
		}
	}

	inline float ratioEvaluate(Board &board, Player playerID, Player enemyID) {
		return (float)board.getPlayerCellCount(playerID) / (board.getPlayerCellCount(playerID) + board.getPlayerCellCount(enemyID));
	}

public:
	AdversarialEvaluator(int numTrials) {
		this->numTrials = numTrials;
		this->nextRoundBoard = new Board(18, 16);
		this->moveBoard = new Board(18, 16);
	}

	~AdversarialEvaluator() {
		delete nextRoundBoard;
		delete moveBoard;
	}

	float evaluate(Board &board, Player playerID, Player enemyID) {
		if (board.getPlayerCellCount(playerID) == 0) {
			return 0;
		}
		else if (board.getPlayerCellCount(enemyID) == 0) {
			return 1;
		}

		board.setNextRoundBoard(*nextRoundBoard);
		vector<Coordinate> deadCells = board.GetCells('.');
		vector<Coordinate> myCells = board.GetCells(playerID);
		vector<Coordinate> enemyCells = board.GetCells(enemyID);
		vector<MoveType> availableMoveTypes = getAvailableMoveTypes(board, playerID, enemyID);

		float minScore = 1;

		for (int trial = 0; trial < numTrials; trial++) {
			Move trialMove = getRandomMove(availableMoveTypes, deadCells, myCells, enemyCells);
			board.applyMove(trialMove, playerID, *nextRoundBoard, *moveBoard);
			float score = ratioEvaluate(*moveBoard, playerID, enemyID);
			if (score < minScore) {
				minScore = score;
			}
		}
		return minScore;
	}
};

#endif // !ADVERSARIALEVALUATOR_h
