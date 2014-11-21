/*
 * vector_math.h
 *
 * Created: 11/21/2014 12:57:53 PM
 *  Author: geoza435
 */ 


#ifndef VECTOR_MATH_H_
#define VECTOR_MATH_H_

#include <math.h>

typedef struct {
	double x;
	double y;
	double z;
} Vector;

typedef double Matrix[3][3];

Vector vector(double x, double y, double z);
Vector v_diff(Vector a, Vector b);
Vector v_add(Vector a, Vector b);
double v_abs(Vector v);
Vector v_div(Vector v, double s);
Vector v_mul(Vector v, double s);
Vector v_normal(Vector v);
Vector m_mul(double m[3][3], Vector v);
double (*get_rotation_x(double angle))[3];
double (*get_rotation_y(double angle))[3];
double (*get_rotation_z(double angle))[3];


#endif /* VECTOR_MATH_H_ */
