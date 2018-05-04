
#ifndef DISTANCEEVALUATOR2_h
#define DISTANCEEVALUATOR2_h

#include <math.h>
#include "Evaluator.h"

class DistanceEvaluator : public Evaluator {
private:
	static const int cols = 18;
	static const int rows = 16;
	float maxDist;
	float multiplier;
	float distanceDivisor;
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

	float getDistanceWeight(float x, float y) {
		float xDistSquared = pow(((float)cols / 2) - x, 2);
		float yDistSquared = pow(((float)rows / 2) - y, 2);
		float dist = sqrt(xDistSquared + yDistSquared);
		return (maxDist - dist) / maxDist;
	}

public:

	DistanceEvaluator(float multiplier, float distanceDivisor) {
		this->multiplier = multiplier;
		this->maxDist = sqrt((float)(rows*rows) / distanceDivisor + (float)(cols*cols) / distanceDivisor);
		this->distanceDivisor = distanceDivisor;
	}

	~DistanceEvaluator() {
	}

	float evaluate(Board &board, Player playerID, Player enemyID) {
		for (int i = 0; i < cols; i++) {
			for (int j = 0; j < rows; j++) {
				controlBoard[i][j] = Board::EMPTY;
			}
		}

		float playerScore = 0;
		float enemyScore = 0;

		vector<Coordinate> playerCells = board.GetCells(playerID);
		vector<Coordinate> enemyCells = board.GetCells(enemyID);

		int enemyQueueStart = 0;
		int enemyQueueEnd = 0;
		for (int i = 0; i < enemyCells.size(); i++) {
			Coordinate c = enemyCells[i];
			controlBoard[c.x][c.y] = enemyID;
			queue[enemyQueueEnd] = c;
			enemyQueueEnd++;
			enemyScore += getDistanceWeight(c.x, c.y);
		}

		int playerQueueStart = enemyQueueEnd;
		int playerQueueEnd = playerQueueStart;
		for (int i = 0; i < playerCells.size(); i++) {
			Coordinate c = playerCells[i];
			controlBoard[c.x][c.y] = playerID;
			queue[playerQueueEnd] = c;
			playerQueueEnd++;
			playerScore += getDistanceWeight(c.x, c.y);
		}

		float weight = 1.0;

		while (playerQueueEnd != playerQueueStart || enemyQueueEnd != enemyQueueStart) {
			//playerScore += (float)weight * currentPlayerScore;
			//enemyScore += (float)weight * currentEnemyScore;
			weight *= multiplier;

			assert(playerQueueEnd >= playerQueueStart && playerQueueStart == enemyQueueEnd && enemyQueueEnd >= enemyQueueStart);
			enemyQueueEnd = playerQueueEnd;
			for (; enemyQueueStart < playerQueueStart; enemyQueueStart++) {
				Coordinate c = queue[enemyQueueStart];
				unsigned char x = c.x;
				unsigned char y = c.y;

				int processResult = processCoordinate(x - 1, y, enemyQueueEnd, enemyID);
				enemyQueueEnd += processResult;
				if (processResult) enemyScore += (float)weight * processResult * getDistanceWeight(x - 1, y);

				processResult = processCoordinate(x + 1, y, enemyQueueEnd, enemyID);
				enemyQueueEnd += processResult;
				if (processResult) enemyScore += (float)weight * processResult * getDistanceWeight(x + 1, y);

				processResult = processCoordinate(x, y - 1, enemyQueueEnd, enemyID);
				enemyQueueEnd += processResult;
				if (processResult) enemyScore += (float)weight * processResult * getDistanceWeight(x, y - 1);

				processResult = processCoordinate(x, y + 1, enemyQueueEnd, enemyID);
				enemyQueueEnd += processResult;
				if (processResult) enemyScore += (float)weight * processResult * getDistanceWeight(x, y + 1);
			}
			enemyQueueStart = playerQueueEnd;

			assert(playerQueueEnd >= playerQueueStart && enemyQueueStart == playerQueueEnd && enemyQueueEnd >= enemyQueueStart);
			playerQueueEnd = enemyQueueEnd;
			for (; playerQueueStart < enemyQueueStart; playerQueueStart++) {
				Coordinate c = queue[playerQueueStart];
				unsigned char x = c.x;
				unsigned char y = c.y;
				int processResult = processCoordinate(x - 1, y, playerQueueEnd, playerID);
				playerQueueEnd += processResult;
				if (processResult) playerScore += (float)weight * processResult * getDistanceWeight(x - 1, y);

				processResult = processCoordinate(x + 1, y, playerQueueEnd, playerID);
				playerQueueEnd += processResult;
				if (processResult) playerScore += (float)weight * processResult * getDistanceWeight(x + 1, y);

				processResult = processCoordinate(x, y - 1, playerQueueEnd, playerID);
				playerQueueEnd += processResult;
				if (processResult) playerScore += (float)weight * processResult * getDistanceWeight(x, y - 1);

				processResult = processCoordinate(x, y + 1, playerQueueEnd, playerID);
				playerQueueEnd += processResult;
				if (processResult) playerScore += (float)weight * processResult * getDistanceWeight(x, y + 1);
			}
			playerQueueStart = enemyQueueEnd;
		}

		float result = (float)playerScore / (playerScore + enemyScore);
		return result;
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "ControlEvalator(" << multiplier << ", " << distanceDivisor << ")";
		return stringStream.str();
	}
};

#endif