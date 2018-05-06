
#ifndef DistanceEvaluator_h
#define DistanceEvaluator_h

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
	float distances[cols][rows];
	Coordinate playerQueue[cols * rows];
	Coordinate enemyQueue[cols * rows];

	bool processCoordinate(int x, int y, Player playerID) {
		if (x >= 0 && x < cols && y >= 0 && y < rows && controlBoard[x][y] == Board::EMPTY) {
			controlBoard[x][y] = playerID;
			return 1;
		}
		return 0;
	}

	void setDistanceWeight(int x, int y) {
		float xDistSquared = pow(((float)cols / 2) - x, 2);
		float yDistSquared = pow(((float)rows / 2) - y, 2);
		float dist = sqrt(xDistSquared + yDistSquared);
		distances[x][y] = (maxDist - dist) / maxDist;
	}

	inline float getDistanceWeight(int x, int y) {
		return distances[x][y];
	}

public:

	DistanceEvaluator(float multiplier, float distanceDivisor) {
		this->multiplier = multiplier;
		this->maxDist = sqrt((float)(rows*rows) / distanceDivisor + (float)(cols*cols) / distanceDivisor);
		this->distanceDivisor = distanceDivisor;
		for (int x = 0; x < cols; x++) {
			for (int y = 0; y < rows; y++) {
				setDistanceWeight(x, y);
			}
		}
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
			enemyQueue[enemyQueueEnd] = c;
			enemyQueueEnd++;
			enemyScore += getDistanceWeight(c.x, c.y);
		}

		int playerQueueStart = 0;
		int playerQueueEnd = 0;
		for (int i = 0; i < playerCells.size(); i++) {
			Coordinate c = playerCells[i];
			controlBoard[c.x][c.y] = playerID;
			playerQueue[playerQueueEnd] = c;
			playerQueueEnd++;
			playerScore += getDistanceWeight(c.x, c.y);
		}

		float weight = 1.0;

		while (playerQueueEnd != playerQueueStart || enemyQueueEnd != enemyQueueStart) {
			weight *= multiplier;

			assert(playerQueueEnd >= playerQueueStart && enemyQueueEnd >= enemyQueueStart);
			int oldEnemyQueueEnd = enemyQueueEnd;
			for (; enemyQueueStart < oldEnemyQueueEnd; enemyQueueStart++) {
				Coordinate c = enemyQueue[enemyQueueStart];
				unsigned char x = c.x;
				unsigned char y = c.y;

				bool processResult = processCoordinate(x - 1, y, enemyID);
				if (processResult) {
					enemyQueue[enemyQueueEnd] = Coordinate(x - 1, y);
					enemyQueueEnd++;
					enemyScore += (float)weight * getDistanceWeight(x - 1, y);
				}

				processResult = processCoordinate(x + 1, y, enemyID);
				if (processResult) {
					enemyQueue[enemyQueueEnd] = Coordinate(x + 1, y);
					enemyQueueEnd++;
					enemyScore += (float)weight * getDistanceWeight(x + 1, y);
				}

				processResult = processCoordinate(x, y - 1, enemyID);
				if (processResult) {
					enemyQueue[enemyQueueEnd] = Coordinate(x, y - 1);
					enemyQueueEnd++;
					enemyScore += (float)weight * getDistanceWeight(x, y - 1);
				}

				processResult = processCoordinate(x, y + 1, enemyID);
				if (processResult) {
					enemyQueue[enemyQueueEnd] = Coordinate(x, y + 1);
					enemyQueueEnd++;
					enemyScore += (float)weight * getDistanceWeight(x, y + 1);
				}
			}

			assert(playerQueueEnd >= playerQueueStart && enemyQueueEnd >= enemyQueueStart);
			int oldPlayerQueueEnd = playerQueueEnd;
			for (; playerQueueStart < oldPlayerQueueEnd; playerQueueStart++) {
				Coordinate c = playerQueue[playerQueueStart];
				unsigned char x = c.x;
				unsigned char y = c.y;

				bool processResult = processCoordinate(x - 1, y, playerID);
				if (processResult) {
					playerQueue[playerQueueEnd] = Coordinate(x - 1, y);
					playerQueueEnd++;
					playerScore += (float)weight * getDistanceWeight(x - 1, y);
				}

				processResult = processCoordinate(x + 1, y, playerID);
				if (processResult) {
					playerQueue[playerQueueEnd] = Coordinate(x + 1, y);
					playerQueueEnd++;
					playerScore += (float)weight * getDistanceWeight(x + 1, y);
				}

				processResult = processCoordinate(x, y - 1, playerID);
				if (processResult) {
					playerQueue[playerQueueEnd] = Coordinate(x, y - 1);
					playerQueueEnd++;
					playerScore += (float)weight * getDistanceWeight(x, y - 1);
				}

				processResult = processCoordinate(x, y + 1, playerID);
				if (processResult) {
					playerQueue[playerQueueEnd] = Coordinate(x, y + 1);
					playerQueueEnd++;
					playerScore += (float)weight * getDistanceWeight(x, y + 1);
				}
			}
		}

		float result = (float)playerScore / (playerScore + enemyScore);
		return result;
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "DistanceEvaluator(" << multiplier << ", " << distanceDivisor << ")";
		return stringStream.str();
	}
};

#endif