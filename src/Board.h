
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
#include <math.h>
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

	static inline int getLookupTableIndex(char **grid);
	static void getLookupTableHelper(char *lookupTable, char **grid, int x, int y, int bot0Neighbors, int bot1Neighbors);
	static char* getLookupTable();

	virtual inline char charToRepr(char c);
	virtual inline char reprToChar(char c);

	virtual int getCellIndex(unsigned char cellX, unsigned char cellY);
	virtual inline char getNextCellStatus(unsigned char cellX, unsigned char cellY);

	virtual inline void updateCellStatus(unsigned char row, unsigned char col, int cellIndex);
	virtual inline void updateRegionStatus(unsigned char row, unsigned char col, Board &lastRoundBoard);

	virtual inline void deleteBoard();
	virtual inline void copyBoard(char **blankBoard);
	virtual inline void copyBoard(Board &blankBoard);

public:
	static char *simulationLookupTable;
	static const char EMPTY = '.';

	Board();
	Board(int width, int height);
	~Board();
	bool operator== (Board &other);

	virtual void UpdateBoard(stringstream &stream);
	virtual void initiateBoardPositions(int aliveCells = 40);
	virtual Board *getCopy();
	virtual void copyInto(Board &result);

	virtual inline bool gameIsOver();
	virtual inline int getPlayerCellCount(Player playerID);
	virtual inline int getWidth();
	virtual inline int getHeight();
	
	virtual inline void nextRound();
	virtual void setNextRoundBoard(Board &result);
	virtual Board *getNextRoundBoard();

	virtual bool isLegal(Move &move, Player playerID);
	virtual Board *makeMove(Move &move, Player playerID);
	virtual void makeMove(Move &move, Player playerID, Board &result);
	virtual void makeMoveOnBoard(Move &move, Player playerID);
	virtual void applyMove(Move &move, Player playerID, Board &nextRoundBoard, Board &result);

	virtual string toString(bool showBoard = false);
	virtual string repr();

	virtual vector<Coordinate> GetCells(char type);
	virtual inline char getCoordinateType(Coordinate &c);
	virtual inline char getCoordinateType(int x, int y);
};

#endif
