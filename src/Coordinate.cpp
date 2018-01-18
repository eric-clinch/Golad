#include "Coordinate.h"
using namespace std;

Coordinate::Coordinate() {}

Coordinate::Coordinate(int x, int y)
{
    this->x = x;
    this->y = y;
}

string Coordinate::toString()
{
    ostringstream stringStream;
    stringStream << x << "," << y;
    return stringStream.str();
}

bool Coordinate::operator==(Coordinate &other) {
	return x == other.x && y == other.y;
}