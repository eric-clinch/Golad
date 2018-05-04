
#ifndef CONTROLEVALUATOR_h
#define CONTROLEVALUATOR_h

#include "Evaluator.h"

class ControlEvaluator : public Evaluator {
private:
	static const int cols = 18;
	static const int rows = 16;
	float multiplier;
	char controlBoard[cols][rows];
	Coordinate queue[cols * rows];

	int processCoordinate(int x, int y, int i, Player playerID) {
		if (x >= 0 && x < cols && y >= 0 && y < rows && controlBoard[x][y] == Board::EMPTY) {
			controlBoard[x][y] = playerID;
			queue[i] = Coordinate(x, y);
			return 1;
		}
		return 0;
	}

public:

	ControlEvaluator(float multiplier) {
		this->multiplier = multiplier;
	}

	~ControlEvaluator() {
	}

	float evaluate(Board &board, Player playerID, Player enemyID) {
		for (int i = 0; i < cols; i++) {
			for (int j = 0; j < rows; j++) {
				controlBoard[i][j] = Board::EMPTY;
			}
		}

		vector<Coordinate> playerCells = board.GetCells(playerID);
		vector<Coordinate> enemyCells = board.GetCells(enemyID);

		int enemyQueueStart = 0;
		int enemyQueueEnd = 0;
		for (int i = 0; i < enemyCells.size(); i++) {
			Coordinate c = enemyCells[i];
			controlBoard[c.x][c.y] = enemyID;
			queue[enemyQueueEnd] = c;
			enemyQueueEnd++;
		}

		int playerQueueStart = enemyQueueEnd;
		int playerQueueEnd = playerQueueStart;
		for (int i = 0; i < playerCells.size(); i++) {
			Coordinate c = playerCells[i];
			controlBoard[c.x][c.y] = playerID;
			queue[playerQueueEnd] = c;
			playerQueueEnd++;
		}

		int playerScore = 0;
		int enemyScore = 0;
		float weight = 1.0;

		while (playerQueueEnd != playerQueueStart || enemyQueueEnd != enemyQueueStart) {
			int currentPlayerScore = playerQueueEnd - playerQueueStart;
			int currentEnemyScore = enemyQueueEnd - enemyQueueStart;
			playerScore += (float)weight * currentPlayerScore;
			enemyScore += (float)weight * currentEnemyScore;
			weight *= multiplier;

			assert(playerQueueEnd >= playerQueueStart && playerQueueStart == enemyQueueEnd && enemyQueueEnd >= enemyQueueStart);
			enemyQueueEnd = playerQueueEnd;
			for (; enemyQueueStart < playerQueueStart; enemyQueueStart++) {

				Coordinate c = queue[enemyQueueStart];
				unsigned char x = c.x;
				unsigned char y = c.y;
				enemyQueueEnd += processCoordinate(x - 1, y, enemyQueueEnd, enemyID);
				enemyQueueEnd += processCoordinate(x + 1, y, enemyQueueEnd, enemyID);
				enemyQueueEnd += processCoordinate(x, y - 1, enemyQueueEnd, enemyID);
				enemyQueueEnd += processCoordinate(x, y + 1, enemyQueueEnd, enemyID);
			}
			enemyQueueStart = playerQueueEnd;

			assert(playerQueueEnd >= playerQueueStart && enemyQueueStart == playerQueueEnd && enemyQueueEnd >= enemyQueueStart);
			playerQueueEnd = enemyQueueEnd;
			for (; playerQueueStart < enemyQueueStart; playerQueueStart++) {

				Coordinate c = queue[playerQueueStart];
				unsigned char x = c.x;
				unsigned char y = c.y;
				playerQueueEnd += processCoordinate(x - 1, y, playerQueueEnd, playerID);
				playerQueueEnd += processCoordinate(x + 1, y, playerQueueEnd, playerID);
				playerQueueEnd += processCoordinate(x, y - 1, playerQueueEnd, playerID);
				playerQueueEnd += processCoordinate(x, y + 1, playerQueueEnd, playerID);
			}
			playerQueueStart = enemyQueueEnd;
		}

		float result = (float)playerScore / (playerScore + enemyScore);
		return result;
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "ControlEvalator(" << multiplier << ")";
		return stringStream.str();
	}
};

#endif