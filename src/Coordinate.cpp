#include "Coordinate.h"
using namespace std;

Coordinate::Coordinate() {}

Coordinate::Coordinate(int x, int y) {
    this->x = x;
    this->y = y;
}

string Coordinate::toString() {
    ostringstream stringStream;
    stringStream << x << "," << y;
    return stringStream.str();
}

string Coordinate::toString() const {
	ostringstream stringStream;
	stringStream << x << "," << y;
	return stringStream.str();
}

bool Coordinate::operator==(Coordinate &other) {
	return x == other.x && y == other.y;
}

bool Coordinate::operator==(const Coordinate &other) const {
	return x == other.x && y == other.y;
}

// an arbitrary comparison function. Its purpose is just to
// define an ordering over coordinates
bool Coordinate::operator<(Coordinate &other) {
	return x < other.x || (x == other.x && y < other.y);
}