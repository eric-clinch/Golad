
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#endif

#ifndef INT_MAX
#define INT_MAX 100000
#endif

#ifndef INT_MIN
#define INT_MIN -100000
#endif

#ifndef BirthRandSearch_h
#define BirthRandSearch_h

#include <assert.h>
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"
#include "Tools.h"

class BirthRandSearch : public Strategy
{
private:
	int maxDepth;
	// len(adversarial) needs to be equal to maxDepth
	int *adversarialTrials;

	struct MoveAndScore;
	template <class T> inline T PopRandomElementFromVector(vector<T> &input);
	template <class T> inline T GetRandomElementFromVector(vector<T> &input);

	Move getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes,
		vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells);
	vector<MoveType> GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID);

	virtual double getBestMoveScore(Board &board, Player playerID, Player enemyID, int trials, int depth);
	virtual inline double getMoveScore(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &empytBoard, int depth);

	virtual MoveAndScore getBestKillMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &enemyCells, Board &nextRoundBoard);
	virtual MoveAndScore getBestBirthMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &deadCells, vector<Coordinate> &myCells,
		Board &nextRoundBoard, int time);

public:
	BirthRandSearch(int maxDepth, int* adversarialTrials);
	virtual Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove);
};

#endif