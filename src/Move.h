#ifndef MOVE_h
#define MOVE_h

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include "Coordinate.h"
#include "Enums.h"

class Move {
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
	virtual string toString() const;
	virtual bool operator== (Move &other);
	virtual bool operator== (const Move &other) const;
};

namespace std {
	template<> struct hash<Move> {
		size_t operator()(const Move &m) const {
			return ((hash<Coordinate>()(m.target) ^
				(hash<Coordinate>()(m.sacrifice1) << 1)) >> 1) ^
				(hash<Coordinate>()(m.sacrifice2) << 1);
		}
	};
}

#endif