
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

#ifndef MCDSStrategy_h
#define MCDSStrategy_h

#include <assert.h>
#include <math.h>
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"
#include "Tools.h"

class MCDSStrategy : public Strategy {
private:
	static double max_score;

	int maxDepth;
	// len(adversarial) needs to be equal to maxDepth
	int *adversarialTrials;
	double confidenceConstant;

	struct MoveAndScore;
	struct CoordinateAndScore;

	Move getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes,
		vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells);
	vector<MoveType> GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID);

	virtual double getBestMoveScore(Board &board, Player playerID, Player enemyID, int trials, int depth);
	virtual inline double getMoveScore(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth);

	virtual MoveAndScore getBestKillMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &enemyCells,
		vector<Coordinate> &myCells, Board &nextRoundBoard);

	virtual CoordinateAndScore* getBestSacrifices(vector<CoordinateAndScore> &myCells, int totalSimulations);
	virtual Coordinate* getResultSacrifices(vector <CoordinateAndScore> &myCells);
	virtual CoordinateAndScore getBestBirthTarget(vector<CoordinateAndScore> &deadCells, int totalSimulations);
	virtual Coordinate getResultBirthTarget(vector<CoordinateAndScore> &deadCells);

	virtual MoveAndScore getBestBirthMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &deadCells, vector<Coordinate> &myCells,
		Board &nextRoundBoard, int time);

public:
	MCDSStrategy(int maxDepth, int *adversarialTrials, double confidenceConstant = 2.0);
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove);
};

#endif