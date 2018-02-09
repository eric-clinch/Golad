
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
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"
#include "Tools.h"
#include "RandomVector.h"

class BirthRandSearch2 : public Strategy
{
private:
	static double max_score;
	int maxDepth;
	// len(adversarial) needs to be equal to maxDepth
	int *adversarialTrials;
	Board **trialBoards;
	Board **nextRoundBoards;

	struct MoveAndScore;

	virtual inline Move getTrialMove(vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells, int i);
	virtual inline Move getRandomBirth(vector<Coordinate> &deadCells, vector<Coordinate> &myCells);
	virtual Move getRandomMove(vector<MoveType> &availableMoveTypes, vector<Coordinate> &deadCells,
		vector<Coordinate> &myCells, vector<Coordinate> &enemyCells);
	virtual vector<MoveType> GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID);

	virtual MoveAndScore evaluateBoardMini(Board &board, Player playerID, Player enemyID, int trials, int depth, double alpha, double beta, Move &killerMove);
	virtual inline MoveAndScore getMoveScoreMini(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth,
		double alpha, double beta, Move &killerMove);

	virtual MoveAndScore evaluateBoardMaxi(Board &board, Player playerID, Player enemyID, int trials, int depth, double alpha, double beta, Move &killerMove);
	virtual inline MoveAndScore getMoveScoreMaxi(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth,
		double alpha, double beta, Move &killerMove);

	virtual MoveAndScore getBestKillMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &enemyCells,
		vector<Coordinate> &myCells, Board &nextRoundBoard);
	virtual MoveAndScore getBestBirthMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &deadCells,
		vector<Coordinate> &myCells, Board &nextRoundBoard, int time);

public:
	BirthRandSearch2(int maxDepth, int* adversarialTrials);
	~BirthRandSearch2();
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove, int round);
};

#endif