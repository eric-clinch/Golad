
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

#ifndef ADVERSARIALSTRATEGY_h
#define ADVERSARIALSTRATEGY_h

#include <assert.h>
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"
#include "RandomStrategy.h"
#include "Tools.h"

class AdversarialStrategy : public Strategy {
private:
	int adversarialTrials;

	template <class T> inline T PopRandomElementFromVector(vector<T> &input);

	template <class T> inline T GetRandomElementFromVector(vector<T> &input);

	Move getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes,
					   vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells);
	vector<MoveType> GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID);

	virtual inline int getSimpleMoveScore(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &empytBoard);
	virtual int getBestMoveScore(Board &board, Player playerID, Player enemyID, int trials);
	virtual inline int getMoveScore(Board &board, Player playerID, Player enemyID, Move &move, Board &nextRoundBoard, Board &empytBoard);

public:
	AdversarialStrategy(int adversarialTrials);
	Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove);
};

#endif