/*
 * ik.c
 *
 * Created: 11/21/2014 5:40:10 AM
 *  Author: geoza435
 */ 

#include "ik.h"
#include "vector_math.h"

// Färdig-uträknade rotationsmatriser
// 90°
Matrix CW_1 = {
	{0.707106781187, -0.707106781187, 0.0},
	{0.707106781187,  0.707106781187, 0.0},
	{           0.0,             0.0, 1.0}
};
// 180°+90°
Matrix CW_2 = {
	{-0.707106781187, -0.707106781187, 0.0},
	{ 0.707106781187, -0.707106781187, 0.0},
	{            0.0,             0.0, 1.0}
};
// -90°
Matrix CCW_1 = {
	{ 0.707106781187, 0.707106781187, 0.0},
	{-0.707106781187, 0.707106781187, 0.0},
	{            0.0,            0.0, 1.0}
};
// -180°+90°
Matrix CCW_2 = {
	{-0.707106781187,  0.707106781187, 0.0},
	{-0.707106781187, -0.707106781187, 0.0},
	{            0.0,             0.0, 1.0}
};
Matrix MIRROR_XY = {
	{-1,  0, 0},
	{ 0, -1, 0},
	{ 0,  0, 1}
};

#define M_X_OFFSET 10
#define O_X_OFFSET 5.55
#define O_Y_OFFSET 12



#define LEFT_LEGS 0
#define RIGHT_LEGS 1
byte TRIPOD[2][3] = {
	{1, 4, 5},
	{2, 3, 6}
};

Vector* translate_set(Vector* from, Vector left, Vector right) {
	static Vector positions[6];

	// Resetta alla värden
	for (byte id = 0; id < 6; id++) {
		positions[id] = from[id];
	}

	// Gör en translate på benen som specifieras i legs
	for (byte id = 0; id < 3; id++) {
		positions[TRIPOD[LEFT_LEGS][id]-1] = v_add(positions[TRIPOD[LEFT_LEGS][id]-1], left);
	}

	// Gör en translate på benen som specifieras i legs
	for (byte id = 0; id < 3; id++) {
		positions[TRIPOD[RIGHT_LEGS][id]-1] = v_add(positions[TRIPOD[RIGHT_LEGS][id]-1], right);
	}
	return positions;
}

Vector* rotate_set(Vector* from, Matrix left, Matrix right)
{
	static Vector positions[6];

	// Resetta alla värden
	for (byte id = 0; id < 6; id++) {
		positions[id] = from[id];
	}

	for (byte id = 0; id < 3; id++) {
		positions[TRIPOD[LEFT_LEGS][id]-1] = m_mul(left, positions[TRIPOD[LEFT_LEGS][id]-1]);
	}

	for (byte id = 0; id < 3; id++) {
		positions[TRIPOD[RIGHT_LEGS][id]-1] = m_mul(right, positions[TRIPOD[RIGHT_LEGS][id]-1]);
	}

	return positions;
}

// Konverterar världs-koordinater till referens-ramar för varje ben
Vector world_to_local(byte legid, Vector pos) {
	switch(legid) {
	case 1:
		return m_mul(CCW_2, v_diff(pos, vector(-O_X_OFFSET, O_Y_OFFSET, 0)));
	case 2:
		return m_mul(CCW_1, v_diff(pos, vector( O_X_OFFSET, O_Y_OFFSET, 0)));
	case 3:
		return v_diff(m_mul(MIRROR_XY, pos), vector(M_X_OFFSET, 0, 0));
	case 4:
		return v_diff(pos, vector(M_X_OFFSET, 0, 0));
	case 5:
		return m_mul(CW_2, v_diff(pos, vector(-O_X_OFFSET, -O_Y_OFFSET, 0)));
	case 6:
		return m_mul(CW_1, v_diff(pos, vector( O_X_OFFSET, -O_Y_OFFSET, 0)));
	default:
		return pos; // Error?!
	}
}

// Returnerar start-positionerna roterat runt x-axeln ANGLE radianer
Vector* get_rotation_at(Vector* from, double angle) {
	static Vector positions[6]; // Static för att pekaren returnas

	for (int legid = 0; legid < 6; legid++) {
		positions[legid] = m_mul(get_rotation_x(angle), from[legid]);
	}

	return positions;
}

// Utökar definitionsmängden på ik() genom att "trycka tillbaka"
// vektorer så att de hamnar där ik() fungerar.
Vector fix_leg_vector(Vector pos) {
	if ((fabs(pos.y) > F+T-C && pos.x < 0) || pos.x == 0) {
		pos.x = 0.01; // Marginal för att det lätt blir odefinerat vid 0
	}
	
	double max_negative_height = -2; // Höftat värde
	
	// Spetsen på tibia försöker gå igenom robotens kropp underifrån
	if (pos.x < 0 && pos.z > max_negative_height) {
		pos.z = max_negative_height;
	}
	
	Rotation rot;
	
	if (pos.x == 0 && pos.y == 0) {
		rot.gamma = 0;
	} else if (pos.x < 0) {
		rot.gamma = atan2(-pos.y, -pos.x);
	} else {
		rot.gamma = atan2(pos.y, pos.x);
	}
	
	//new_pos är pos, fast från spetsen av Coxa
	Vector coxa_pos = {C*cos(rot.gamma), C*sin(rot.gamma), 0};
	Vector new_pos = v_diff(pos, coxa_pos);
	
	//Femur och Coxa ombedda att sträcka sig längre än de är långa
	if(v_abs(new_pos) > F+T) {
		new_pos = v_mul(v_normal(new_pos),(F+T)-0.001); // Liten marginal
	}
	
	// Tibia är ombedd att komma närmare Coxa än Femur är lång
	if(v_abs(new_pos) < T-F) {
		new_pos = v_mul(v_normal(new_pos), T-F+0.001);
	}
	
	return v_add(new_pos, coxa_pos);
}

Rotation ik(Vector pos) {
	Rotation rot;
	
	if (pos.x == 0 && pos.y == 0) {
		rot.gamma = 0;
	} else if (pos.x > 0) {
		rot.gamma = atan2(pos.y, pos.x);
	} else {
		rot.gamma = atan2(-pos.y, -pos.x);
	}
	
	double CoxaX = C * cos(rot.gamma);
	double CoxaY = C * sin(rot.gamma);

	double L = sqrt(pow(pos.x-CoxaX,2) + pow(pos.y-CoxaY,2) + pow(pos.z,2));
	double alpha1;
	if (pos.z == 0) {
		alpha1 = M_PI/2;
	} else {
		alpha1 = acos(-pos.z/L);
	}
	double alpha2 = acos((pow(F,2)+pow(L,2)-pow(T,2))/(2*L*F));

	if (sqrt(pow(pos.x, 2) + pow(pos.y, 2)) > C && pos.x > 0) {
		rot.alpha = alpha1 + alpha2;
	} else {
		rot.alpha = -alpha1 + alpha2;
	}

	rot.beta = acos((pow(F,2) + pow(T,2) - pow(L,2))/(2*F*T));

	rot.alpha -= M_PI/2;
	rot.beta = M_PI - rot.beta;
	
	return rot;
}
