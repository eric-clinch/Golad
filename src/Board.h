
#ifdef _DEBUG
	#include <stdlib.h>
	#include <crtdbg.h>
	#define _CRTDBG_MAP_ALLOC
	#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
#endif

#ifndef BOARD_h
#define BOARD_h

#include <sstream>
#include <vector>
#include <assert.h>
#include <algorithm>
#include "Enums.h"
#include "Coordinate.h"
#include "Move.h"

using namespace std;

class Board {
private:
	int width, height;
	int P0CellCount, P1CellCount;
	char **board;
	virtual inline char getNextCellStatus(int row, int col);

	virtual inline void updateCellStatus(int row, int col, Board &lastRoundBoard);
	virtual inline void updateRegionStatus(int row, int col, Board &lastRoundBoard);

	virtual inline void deleteBoard();
	virtual inline void copyBoard(char **blankBoard);

public:
	Board(int width, int height);
	~Board();
	bool operator== (Board &other);

	virtual void UpdateBoard(stringstream &stream);
	virtual void initiateBoardPositions(int aliveCells = 40);
	virtual Board *getCopy();

	virtual void setPlayerCellCount(Player playerID, int count);
	virtual int getPlayerCellCount(Player playerID);

	virtual inline void nextRound();
	virtual Board *makeMove(Move &move, Player playerID);
	virtual void makeMoveOnBoard(Move &move, Player playerID);
	virtual Board *applyMove(Move &move, Player playerID, Board &nextRoundBoard);
	virtual string toString();

	virtual vector<Coordinate> GetCells(char type);
};

#endif
