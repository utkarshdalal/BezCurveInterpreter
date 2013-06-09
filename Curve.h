#ifndef CURVEH
#define CURVEH

#include "Point.h"
#include "math.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

class Curve{
	Point point1, point2, point3, point4, pointP, pointS;
	public:
		Curve(){
			point1 = Point();
			point2 = Point();
			point3 = Point();
			point4 = Point();
		}
		Curve(Point p1, Point p2, Point p3, Point p4){
			point1 = p1;
			point2 = p2;
			point3 = p3;
			point4 = p4;
		}

		Point getPoint1(){
			return point1;
		}
		Point getPoint2(){
			return point2;
		}
		Point getPoint3(){
			return point3;
		}
		Point getPoint4(){
			return point4;
		}

		Point getPointP(){
			return pointP;
		}
		Point getPointS(){
			return pointS;
		}

		void setPoint1(Point p){
			point1 = p;
		}
		void setPoint2(Point p){
			point2 = p;
		}
		void setPoint3(Point p){
			point3 = p;
		}
		void setPoint4(Point p){
			point4 = p;
		}

		void interpolate(float u) {
			Point a = point1.multiplyScalar(1-u).addPoint(point2.multiplyScalar(u));
			Point b = point2.multiplyScalar(1-u).addPoint(point3.multiplyScalar(u));
			Point c = point3.multiplyScalar(1-u).addPoint(point4.multiplyScalar(u));
			Point d = a.multiplyScalar(1-u).addPoint(b.multiplyScalar(u));
			Point e = b.multiplyScalar(1-u).addPoint(c.multiplyScalar(u));
			pointP = d.multiplyScalar(1-u).addPoint(e.multiplyScalar(u));
			pointS = (e.subPoint(d)).multiplyScalar(3);
		}
};

#endif
