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
	
	Move() {
		this->MoveType = PASS;
	}

	Move(Coordinate &target) {
		this->MoveType = KILL;
		this->target = target;
	}

	Move(Coordinate &target, Coordinate &sacrifice1, Coordinate &sacrifice2) {
		this->MoveType = BIRTH;
		this->target = target;

		// place them in increasing order to guarantee that each
		// unique move only has one possible representation
		if (sacrifice1 < sacrifice2) {
			this->sacrifice1 = sacrifice1;
			this->sacrifice2 = sacrifice2;
		}
		else {
			this->sacrifice1 = sacrifice2;
			this->sacrifice2 = sacrifice1;
		}
	}

	virtual string toString() {
		if (this->MoveType == KILL) {
			ostringstream stringStream;
			stringStream << "kill " << this->target.toString();
			return stringStream.str();
		}
		else if (this->MoveType == BIRTH) {
			ostringstream stringStream;
			stringStream << "birth " << this->target.toString() << " " << this->sacrifice1.toString() << " " << this->sacrifice2.toString();
			return stringStream.str();
		}
		else {
			return "pass";
		}
	}

	virtual string toString() const {
		if (this->MoveType == KILL) {
			ostringstream stringStream;
			stringStream << "kill " << this->target.toString();
			return stringStream.str();
		}
		else if (this->MoveType == BIRTH) {
			ostringstream stringStream;
			stringStream << "birth " << this->target.toString() << " " << this->sacrifice1.toString() << " " << this->sacrifice2.toString();
			return stringStream.str();
		}
		else {
			return "pass";
		}
	}

	virtual bool operator== (Move &other) {
		return MoveType == other.MoveType && target == other.target && sacrifice1 == other.sacrifice1 && sacrifice2 == other.sacrifice2;
	}

	virtual bool operator== (const Move &other) const {
		return MoveType == other.MoveType && target == other.target && sacrifice1 == other.sacrifice1 && sacrifice2 == other.sacrifice2;
	}
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