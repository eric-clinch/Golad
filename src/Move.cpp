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
	this->sacrifice1 = sacrifice1;
	this->sacrifice2 = sacrifice2;
}

string Move::toString()
{
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