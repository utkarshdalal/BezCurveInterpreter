#ifndef POINTPAIRH
#define POINTPAIRH

#include "Point.h"

class PointPair{
public:
	Point p1, p2;
	PointPair(){
		p1 = Point();
		p2 = Point();
	}
	PointPair(Point point1, Point point2){
		p1 = point1;
		p2 = point2;
	}
	Point getPoint1(){
		return p1;
	}
	Point getPoint2(){
		return p2;
	}

};

#endif