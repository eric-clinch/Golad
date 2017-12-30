
#ifdef _DEBUG
	#include <stdlib.h>
	#include <crtdbg.h>
	#define _CRTDBG_MAP_ALLOC
	#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
#endif

#ifndef RANDSEARCHSTRATEGY_h
#define RANDSEARCHSTRATEGY_h

#include <assert.h>
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"
#include "RandomStrategy.h"
#include "Tools.h"

class RandSearchStrategy : public Strategy {
private:
	template <class T>
	inline T PopRandomElementFromVector(vector<T> &input);

	template <class T>
	inline T GetRandomElementFromVector(vector<T> &input);

	Move getRandomMove(Board &board, Player playerID, Player enemyID, vector<MoveType> &availableMoveTypes, 
					   vector<Coordinate> &deadCells, vector<Coordinate> &myCells, vector<Coordinate> &enemyCells);
	vector<MoveType> GetAvailableMoveTypes(Board &board, Player playerID, Player enemyID);

public:
	RandSearchStrategy();
	Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove);
};

#endif