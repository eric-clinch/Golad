
#ifndef DISTANCEEVALUATOR2_h
#define DISTANCEEVALUATOR2_h

#include <math.h>
#include "Evaluator.h"

class DistanceEvaluator2 : public Evaluator {
private:
	static const int cols = 18;
	static const int rows = 16;
	float maxDist;
	float distanceDivisor;

	float getDistanceWeight(float x, float y) {
		float xDistSquared = pow(((float)cols / 2) - x, 2);
		float yDistSquared = pow(((float)rows / 2) - y, 2);
		float dist = sqrt(xDistSquared + yDistSquared);
		return (maxDist - dist) / maxDist;
	}

public:

	DistanceEvaluator2(float distanceDivisor) {
		this->maxDist = sqrt((float)(rows*rows) / distanceDivisor + (float)(cols*cols) / distanceDivisor);
		this->distanceDivisor = distanceDivisor;
	}

	~DistanceEvaluator2() {
	}

	float evaluate(Board &board, Player playerID, Player enemyID) {
		float playerScore = 0;
		vector<Coordinate> playerCells = board.GetCells(playerID);
		for (int i = 0; i < playerCells.size(); i++) {
			Coordinate c = playerCells[i];
			playerScore += getDistanceWeight(c.x, c.y);
		}


		float enemyScore = 0;
		vector<Coordinate> enemyCells = board.GetCells(enemyID);
		for (int i = 0; i < enemyCells.size(); i++) {
			Coordinate c = enemyCells[i];
			enemyScore += getDistanceWeight(c.x, c.y);
		}
		
		float result = (float)playerScore / (playerScore + enemyScore);
		return result;
	}

	string toString() {
		ostringstream stringStream;
		stringStream << "ControlEvalator(" << distanceDivisor << ")";
		return stringStream.str();
	}
};

#endif