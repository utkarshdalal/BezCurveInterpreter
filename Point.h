#ifndef POINTH
#define POINTH

#include <math.h>
#include <iostream>
class Point{
	double x, y, z;
	public:
		Point(){
			x = 0;
			y = 0;
			z = 0;
		}
		Point(double i, double j, double k){
			x = i;
			y = j;
			z = k;
		}
		Point get(){
			return Point(x, y, z);
		}
		double getX(){
			return x;
		}
		double getY(){
			return y;
		}
		double getZ(){
			return z;
		}
		Point multiplyScalar(float f) {
			double a = f * x;
			double b = f * y;
			double c = f * z;
			return Point(a,b,c);
		}
		Point addPoint(Point p) {
			double a = x+p.getX();
			double b = y+p.getY();
			double c = z+p.getZ();
			return Point(a,b,c);
		}
		Point subPoint(Point p) {
			double a = x-p.getX();
			double b = y-p.getY();
			double c = z-p.getZ();
			return Point(a,b,c);
		}
		Point normalize() {
			double mag = sqrt((x*x)+(y*y)+(z*z));
			return Point(x/mag, y/mag, z/mag);
		}
		double magnitude(){
			return sqrt((x*x)+(y*y)+(z*z));
		}
		Point getCrossProduct(Point p) {
			return Point(y*p.getZ() - z*p.getY(), z*p.getX() - x*p.getZ(), x*p.getY() - y*p.getX());
		}

};

#endif
