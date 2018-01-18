
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
	int __width__, __height__; // the dimensions of the 2D char array used to represent the board internally
	int P0CellCount, P1CellCount;
	char **board;

	static char *simulationLookupTable;

	virtual int getCellIndex(int cellX, int cellY);
	virtual inline char getNextCellStatus(int cellX, int cellY);

	virtual inline void updateCellStatus(int row, int col, int cellIndex);
	virtual inline void updateRegionStatus(int row, int col, Board &lastRoundBoard);

	virtual inline void deleteBoard();
	virtual inline void copyBoard(char **blankBoard);
	virtual inline void copyBoard(Board &blankBoard);

public:
	Board(int width, int height);
	~Board();
	bool operator== (Board &other);

	virtual void UpdateBoard(stringstream &stream);
	virtual void initiateBoardPositions(int aliveCells = 40);
	virtual Board *getCopy();

	virtual int getPlayerCellCount(Player playerID);
	virtual void setPlayerCellCount(Player playerID, int cellCount);
	virtual int getWidth();
	virtual int getHeight();
	
	virtual inline void nextRound();
	virtual Board *getNextRoundBoard();
	virtual Board *makeMove(Move &move, Player playerID);
	virtual void makeMoveOnBoard(Move &move, Player playerID);
	virtual void applyMove(Move &move, Player playerID, Board &nextRoundBoard, Board &result);
	virtual string toString();

	virtual vector<Coordinate> GetCells(char type);
};

#endif
