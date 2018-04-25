#ifndef COORDINATE_H
#define COORDINATE_H

#include <iostream>
#include <sstream>
#include <stdio.h>
using namespace std;

class Coordinate {
public:
    unsigned char x, y;

	Coordinate() {};

	Coordinate(unsigned char x, unsigned char y) {
		this->x = x;
		this->y = y;
	}

	string toString() {
		ostringstream stringStream;
		stringStream << (int) x << "," << (int) y;
		return stringStream.str();
	}

	string toString() const {
		ostringstream stringStream;
		stringStream << (int) x << "," << (int) y;
		return stringStream.str();
	}

	bool operator== (Coordinate &other) {
		return x == other.x && y == other.y;
	}

	bool operator== (const Coordinate &other) const {
		return x == other.x && y == other.y;
	}

	bool operator< (Coordinate &other) {
		return x < other.x || (x == other.x && y < other.y);
	}
};

namespace std {
	template<> struct hash<Coordinate> {
		size_t operator()(const Coordinate &c) const {
			return (19 + c.x) * 19 + c.y;
		}
	};
}

#endif