#include "RocheLobe.h"
#include <math.h>
#include <float.h>

/**
	calculates dimensionless Roche potential
*/
double rochePotential(double q, double x, double y)
{
	double a = 1.0/(1.0+q);
	double b = q/(1.0+q);

	return (a/sqrt(x*x+y*y)+b/sqrt((x-1.0)*(x-1.0)+y*y)+1.0/2.0*( (x-b)*(x-b)+y*y ));
}
/**
	find position of L1 point with bisection method
*/
double calculateL1Point(double q)
{
	double a = 1.0/(1.0+q);
	double b = q/(1.0+q);

	// position of L1 must be >0 and <1 as we have normalized coordinates
	double left = DBL_EPSILON;
	double right = 1.0-DBL_EPSILON;
	double center;

	// functions values ( derivative of rochePotential)
	double fLeft  = a/(left*left) - b/((left-1.0)*(left-1.0)) - (left-b);
	double fRight = a/(right*right) - b/((right-1.0)*(right-1.0)) - (right-b);
	double fCenter;

	while ( fabs(right-left) > 1e-5 ) {
		// calc center position
		center = 0.5*(right+left);
		// calc center function value
		fCenter = a/(center*center) - b/((center-1.0)*(center-1.0)) - (center-b);

		if (fCenter == 0.0)
			return center;
		
		if (fLeft > 0.0) {
			if (fCenter < 0.0) {
				fRight = fCenter;
				right = center;
			} else {
				fLeft = fCenter;
				left = center;
			}
		} else {
			if (fCenter > 0.0) {
				fRight = fCenter;
				right = center;
			} else {
				fLeft = fCenter;
				left = center;
			}
		}
	}

	return 0.5*(left+right);
}

/**
	calculate radius from star using bisection method
*/
double calculateRocheRadius(double q, double L1, double value, double phi)
{
	// position
	double outer = L1;
	double inner = DBL_EPSILON;
	double center;

	// function values
	double fOuter = rochePotential(q,outer*sin(phi),outer*cos(phi))-value;
	double fInner = rochePotential(q,inner*sin(phi),inner*cos(phi))-value;
	double fCenter;
	
	while ( fabs(outer-inner) > 1e-5 ) {
		// calc center position
		center = 0.5*(inner+outer);
		// calc center function value
		fCenter = rochePotential(q,center*sin(phi),center*cos(phi))-value;

		if (fCenter == 0)
			return center;

		if (fInner > 0.0) {
			if (fCenter < 0.0) {
				fOuter = fCenter;
				outer = center;
			} else {
				fInner = fCenter;
				inner = center;
			}
		} else {
			if (fCenter > 0.0) {
				fOuter = fCenter;
				outer = center;
			} else {
				fInner = fCenter;
				inner = center;
			}
		}
	}
	
	return 0.5*(inner+outer);
}