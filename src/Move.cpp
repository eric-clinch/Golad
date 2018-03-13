#include "Move.h"
using namespace std;

Move::Move()
{
	this->MoveType = PASS;
}

Move::Move(Coordinate &target)
{
	this->MoveType = KILL;
	this->target = target;
}

Move::Move(Coordinate &target, Coordinate &sacrifice1, Coordinate &sacrifice2)
{
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

string Move::toString() {
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

string Move::toString() const {
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

bool Move::operator==(Move &other) {
	return MoveType == other.MoveType && target == other.target && sacrifice1 == other.sacrifice1 && sacrifice2 == other.sacrifice2;
}

bool Move::operator==(const Move &other) const {
	return MoveType == other.MoveType && target == other.target && sacrifice1 == other.sacrifice1 && sacrifice2 == other.sacrifice2;
}