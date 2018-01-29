
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

#ifndef BirthRandSearch_h
#define BirthRandSearch_h

#include <assert.h>
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"
#include "Tools.h"
#include "RandomVector.h"

class BirthRandSearch : public Strategy
{
private:
	static double max_score;
	int maxDepth;
	// len(adversarial) needs to be equal to maxDepth
	int *adversarialTrials;
	Board **trialBoards;
	Board **nextRoundBoards;

	struct MoveAndScore;

	Move getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes,
		vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells);
	vector<MoveType> GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID);

	virtual double evaluateBoardMini(Board &board, Player playerID, Player enemyID, int trials, int depth, double alpha, double beta);
	virtual inline double getMoveScoreMini(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth,
		double alpha, double beta);

	virtual double evaluateBoardMaxi(Board &board, Player playerID, Player enemyID, int trials, int depth, double alpha, double beta);
	virtual inline double getMoveScoreMaxi(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth,
		double alpha, double beta);

	virtual MoveAndScore getBestKillMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &enemyCells,
		vector<Coordinate> &myCells, Board &nextRoundBoard);
	virtual MoveAndScore getBestBirthMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &deadCells, vector<Coordinate> &myCells,
		Board &nextRoundBoard, int time);

public:
	BirthRandSearch(int maxDepth, int* adversarialTrials);
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round);
};

#endif