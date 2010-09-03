#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <GL/gl.h>

class Matrix
{
	private:
		GLdouble mx[16];

		void multiplicate(GLdouble nm[16]);
	public:
		void rotateX(GLdouble alpha);
		void rotateY(GLdouble alpha);
		void rotateZ(GLdouble alpha);
		void translate(GLdouble xt, GLdouble yt, GLdouble zt);
		void scale(GLdouble xs, GLdouble ys, GLdouble zs);

		void clear(void);
		Matrix(void) { clear(); }
};

#endif
