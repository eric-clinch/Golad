
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

#ifndef BOARD2_h
#define BOARD2_h

#include <sstream>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <math.h>
#include "Enums.h"
#include "Coordinate.h"
#include "Move.h"

using namespace std;

class Board2 {
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
	virtual inline void updateRegionStatus(unsigned char row, unsigned char col, Board2 &lastRoundBoard2);

	virtual inline void deleteBoard2();
	virtual inline void copyBoard2(char **blankBoard2);
	virtual inline void copyBoard2(Board2 &blankBoard2);

public:
	static char *simulationLookupTable;
	static const char EMPTY = '.';

	Board2();
	Board2(int width, int height);
	~Board2();
	bool operator== (Board2 &other);

	virtual void UpdateBoard2(stringstream &stream);
	virtual void initiateBoard2Positions(int aliveCells = 40);
	virtual Board2 *getCopy();
	virtual void copyInto(Board2 &result);

	virtual inline bool gameIsOver();
	virtual inline int getPlayerCellCount(Player playerID);
	virtual inline int getWidth();
	virtual inline int getHeight();
	
	virtual inline void nextRound();
	virtual void setNextRoundBoard2(Board2 &result);
	virtual Board2 *getNextRoundBoard2();

	virtual bool isLegal(Move &move, Player playerID);
	virtual Board2 *makeMove(Move &move, Player playerID);
	virtual void makeMove(Move &move, Player playerID, Board2 &result);
	virtual void makeMoveOnBoard2(Move &move, Player playerID);
	virtual void applyMove(Move &move, Player playerID, Board2 &nextRoundBoard2, Board2 &result);

	virtual string toString(bool showBoard2 = false);
	virtual string repr();

	virtual vector<Coordinate> GetCells(char type);
	virtual inline char getCoordinateType(Coordinate &c);
	virtual inline char getCoordinateType(int x, int y);
};

#endif
