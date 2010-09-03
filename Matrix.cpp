#include "Matrix.h"
#include <math.h>

void Matrix::clear(void)
{
	mx[0] = 1;	mx[4] = 0;	mx[8] = 0;	mx[12] = 0;
	mx[1] = 0;	mx[5] = 1;	mx[9] = 0;	mx[13] = 0;
	mx[2] = 0;	mx[6] = 0;	mx[10] = 1;	mx[14] = 0;
	mx[3] = 0;	mx[7] = 0;	mx[11] = 0;	mx[15] = 1;
}

void Matrix::multiplicate(GLdouble nm[16])
{
	int x, a, b;
	// temporary array
	GLdouble tm[16];

	for (x = 0; x<16; ++x) {
		a = x % 4;
		b = (x/4)*4;

		tm[x] = nm[a   ] * mx[b  ] +
			nm[a+ 4] * mx[b+1] +
			nm[a+ 8] * mx[b+2] +
			nm[a+12] * mx[b+3];
	}

	for (x = 0; x<16; ++x) {
		mx[x] = tm[x];
	}
}

void Matrix::rotateX(GLdouble alpha)
{
	GLdouble rm[16];

	const GLdouble pi = M_PI;
	GLdouble c = cos((pi*alpha)/180.0);
	GLdouble s = sin((pi*alpha)/180.0);

	rm[0] = 1;	rm[4] = 0;	rm[8] = 0;	rm[12] = 0;
	rm[1] = 0;	rm[5] = c;	rm[9] = -s;	rm[13] = 0;
	rm[2] = 0;	rm[6] = s;	rm[10] = c;	rm[14] = 0;
	rm[3] = 0;	rm[7] = 0;	rm[11] = 0;	rm[15] = 1;

	multiplicate(rm);
}

void Matrix::rotateY(GLdouble alpha)
{
	GLdouble rm[16];

	const GLdouble pi = M_PI;
	GLdouble c = cos((pi*alpha)/180.0);
	GLdouble s = sin((pi*alpha)/180.0);

	rm[0] = c;	rm[4] = 0;	rm[8] = s;	rm[12] = 0;
	rm[1] = 0;	rm[5] = 1;	rm[9] = 0;	rm[13] = 0;
	rm[2] = -s;	rm[6] = 0;	rm[10] = c;	rm[14] = 0;
	rm[3] = 0;	rm[7] = 0;	rm[11] = 0;	rm[15] = 1;

	multiplicate(rm);
}

void Matrix::rotateZ(GLdouble alpha)
{
	GLdouble rm[16];

	const GLdouble pi = M_PI;
	GLdouble c = cos((pi*alpha)/180.0);
	GLdouble s = sin((pi*alpha)/180.0);

	rm[0] = c;	rm[4] = -s;	rm[8] = 0;	rm[12] = 0;
	rm[1] = s;	rm[5] = c;	rm[9] = 0;	rm[13] = 0;
	rm[2] = 0;	rm[6] = 0;	rm[10] = 1;	rm[14] = 0;
	rm[3] = 0;	rm[7] = 0;	rm[11] = 0;	rm[15] = 1;

	multiplicate(rm);
}

void Matrix::translate(GLdouble xt, GLdouble yt, GLdouble zt)
{
	GLdouble tm[16];

	tm[0] = 1;	tm[4] = 0;	tm[8] = 0;	tm[12] = xt;
	tm[1] = 0;	tm[5] = 1;	tm[9] = 0;	tm[13] = yt;
	tm[2] = 0;	tm[6] = 0;	tm[10] = 1;	tm[14] = zt;
	tm[3] = 0;	tm[7] = 0;	tm[11] = 0;	tm[15] = 1;

	multiplicate(tm);
}

void Matrix::scale(GLdouble xs, GLdouble ys, GLdouble zs)
{
	GLdouble sm[16];

	sm[0] = xs;	sm[4] = 0;	sm[8] = 0;	sm[12] = 0;
	sm[1] = 0;	sm[5] = ys;	sm[9] = 0;	sm[13] = 0;
	sm[2] = 0;	sm[6] = 0;	sm[10] = zs;	sm[14] = 0;
	sm[3] = 0;	sm[7] = 0;	sm[11] = 0;	sm[15] = 0;

	multiplicate(sm);
}
