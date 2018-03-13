#ifndef COORDINATE_H
#define COORDINATE_H

#include <iostream>
#include <sstream>
#include <stdio.h>
using namespace std;

class Coordinate
{
public:
    int x, y;
	Coordinate();
    Coordinate(int x, int y);
    virtual string toString();
	virtual string toString() const;
	virtual bool operator== (Coordinate &other);
	virtual bool operator== (const Coordinate &other) const;
	virtual bool operator< (Coordinate &other);
};

namespace std {
	template<> struct hash<Coordinate> {
		size_t operator()(const Coordinate &c) const {
			return (19 + c.x) * 19 + c.y;
		}
	};
}

#endif