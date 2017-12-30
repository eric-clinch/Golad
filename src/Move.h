#ifndef MOVE_h
#define MOVE_h

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include "Coordinate.h"
#include "Enums.h"
using namespace std;

class Move
{
private:
	static Coordinate defaultCoordinate;

public:
	int MoveType;
	// if the move is not birth, then sacrifice1 and sacrifice2 can be NULL
	// if the move is pass, then target can be NULL as well
	Coordinate target;
	Coordinate sacrifice1;
	Coordinate sacrifice2;
	
	Move();
	Move(Coordinate &target);
	Move(Coordinate &target, Coordinate &sacrifice1, Coordinate &sacrifice2);

	virtual string toString();
};

#endif