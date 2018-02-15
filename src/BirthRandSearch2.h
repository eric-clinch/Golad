
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define NDEBUG
#endif

#ifndef BirthRandSearch2_h
#define BirthRandSearch2_h

#include <assert.h>
#include <math.h>
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"
#include "Tools.h"
#include "RandomVector.h"
#include "LinkedList.h"

class BirthRandSearch2 : public Strategy
{
private:
	static double max_score;
	int maxDepth;
	// len(adversarial) needs to be equal to maxDepth
	int *adversarialTrials;

	struct MoveAndScore;
	struct Data;

	virtual inline Move getRandomBirth(vector<Coordinate> &deadCells, vector<Coordinate> &myCells);
	virtual Move getRandomMove(vector<MoveType> &availableMoveTypes, vector<Coordinate> &deadCells,
		vector<Coordinate> &myCells, vector<Coordinate> &enemyCells);
	virtual vector<MoveType> GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID);

	virtual double evaluateBoardMini(Board &board, int depth, Data &data, double alpha, double beta);
	virtual inline double getMoveScoreMini(Board &board, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth, Data &data, double alpha, double beta);

	virtual double evaluateBoardMaxi(Board &board, int depth, Data &data, double alpha, double beta);
	virtual inline double getMoveScoreMaxi(Board &board, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth, Data &data, double alpha, double beta);

	virtual MoveAndScore getBestKillMove(Board &board, vector<Coordinate> &enemyCells, vector<Coordinate> &myCells, Board &nextRoundBoard, Data &data);
	virtual MoveAndScore getBestBirthMove(Board &board, vector<Coordinate> &deadCells, vector<Coordinate> &myCells, Board &nextRoundBoard, Data &data, int time);

public:
	BirthRandSearch2(int maxDepth, int* adversarialTrials);
	~BirthRandSearch2();
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round);
};

#endif