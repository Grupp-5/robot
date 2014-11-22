/*
 * vector_math.c
 *
 * Created: 11/21/2014 12:58:07 PM
 *  Author: geoza435
 */

#include "vector_math.h"

/*
  Legend:
   v: vektor
   s: skalär
   m: matris
*/

Vector vector(double x, double y, double z) {
	Vector v = {x, y, z};
	return v;
}

// a - b
Vector v_diff(Vector a, Vector b) {
	return vector(a.x - b.x, a.y - b.y, a.z - b.z);
}

// a + b
Vector v_add(Vector a, Vector b) {
	return vector(a.x + b.x, a.y + b.y, a.z + b.z);
}

// |v|
double v_abs(Vector v) {
	return sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
}

// v/s
Vector v_div(Vector v, double s) {
	return vector(v.x/s, v.y/s, v.z/s);
}

// v*s
Vector v_mul(Vector v, double s) {
	return vector(v.x*s, v.y*s, v.z*s);
}

// v/|v|
Vector v_normal(Vector v) {
	return v_div(v, v_abs(v));
}

// m*v
Vector m_mul(double m[3][3], Vector v) {
	Vector u = {0, 0, 0};
	u.x = m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z;
	u.y = m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z;
	u.z = m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z;

	return u;
}

// Rotations-matriser
double (*get_rotation_x(double angle))[3] {
	static double m[3][3];
	// Wow, tack C
	m[0][0] = 1; m[0][1] =          0; m[0][2] =           0;
	m[1][0] = 0; m[1][1] = cos(angle); m[1][2] = -sin(angle);
	m[2][0] = 0; m[2][1] = sin(angle); m[2][2] =  cos(angle);
	return m;
}

double (*get_rotation_y(double angle))[3] {
	static double m[3][3];
	m[0][0] =  cos(angle); m[0][1] = 0; m[0][2] =  sin(angle);
	m[1][0] =           0; m[1][1] = 1; m[1][2] =           0;
	m[2][0] = -sin(angle); m[2][1] = 0; m[2][2] =  cos(angle);
	return m;
}

void make_rotation_z(double m[3][3], double angle) {
	m[0][0] =  cos(angle); m[0][1] = -sin(angle); m[0][2] =  0;
	m[1][0] =  sin(angle); m[1][1] =  cos(angle); m[1][2] =  0;
	m[2][0] =           0; m[2][1] =           0; m[2][2] =  1;
}

double (*get_unit())[3] {
	static double m[3][3];
	m[0][0] =  1; m[0][1] = 0; m[0][2] =  0;
	m[1][0] =  0; m[1][1] = 1; m[1][2] =  0;
	m[2][0] =  0; m[2][1] = 0; m[2][2] =  1;
	return m;
}