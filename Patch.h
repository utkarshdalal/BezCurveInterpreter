#ifndef PATCHH
#define PATCHH

#include "Point.h"
#include "Curve.h"

#include "math.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

class Patch{
	public:
		Point * cPts[4][4];
		Patch() {

		}
		Curve getCurve1(){
			Curve c(*cPts[0][0], *cPts[0][1], *cPts[0][2], *cPts[0][3]);
			return c;
		}
		Curve getCurve2(){
			Curve c(*cPts[1][0], *cPts[1][1], *cPts[1][2], *cPts[1][3]);
			return c;
		}
		Curve getCurve3(){
			Curve c(*cPts[2][0], *cPts[2][1], *cPts[2][2], *cPts[2][3]);
			return c;
		}
		Curve getCurve4(){
			Curve c(*cPts[3][0], *cPts[3][1], *cPts[3][2], *cPts[3][3]);
			return c;
		}
		Curve getVertCurve1(){
			Curve c(*cPts[0][0], *cPts[1][0], *cPts[2][0], *cPts[3][0]);
			return c;
		}
		Curve getVertCurve2(){
			Curve c(*cPts[0][1], *cPts[1][1], *cPts[2][1], *cPts[3][1]);
			return c;
		}
		Curve getVertCurve3(){
			Curve c(*cPts[0][2], *cPts[1][2], *cPts[2][2], *cPts[3][2]);
			return c;
		}
		Curve getVertCurve4(){
			Curve c(*cPts[0][3], *cPts[1][3], *cPts[2][3], *cPts[3][3]);
			return c;
		}
};


#endif
