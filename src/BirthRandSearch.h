
#ifdef _DEBUG
	#include <stdlib.h>
	#include <crtdbg.h>
	#define _CRTDBG_MAP_ALLOC
	#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
#endif

#ifndef BIRTHRANDSEARCH_h
#define BIRTHRANDSEARCH_h

#include <assert.h>
#include "Strategy.h"
#include "Move.h"
#include "Coordinate.h"
#include "Tools.h"

class BirthRandSearch : public Strategy
{
private:
	template <class T>
	inline T PopRandomElementFromVector(vector<T> &input);

	template <class T>
	inline T GetRandomElementFromVector(vector<T> &input);
	int getMoveScore(Board &board, Player playerID, Player enemyID, Move &move);
	Move getBestKillMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &enemyCells);
	Move getBestBirthMove(Board &board, Player playerID, Player enemyID, vector<Coordinate> &deadCells, vector<Coordinate> &myCells, int time);

public:
	BirthRandSearch();
	Move getMove(Board &board, Player playerID, Player enemyID, int time, int timePerMove);
};

#endif