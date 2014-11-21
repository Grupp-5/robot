/*
 * motion.c
 *
 * Created: 11/21/2014 4:31:47 AM
 *  Author: dansj232, geoza435
 */ 

#include "servo.h"
#include "motion.h"
#include "ik.h"

#define F_CPU 16000000UL
#include <util/delay.h>

uint8_t LEGS[6][3] = LEGS_ARRAY;

// Konverterar radianer till grader som AX12 förstår
#define RAD_TO_AX_DEG 195.569594071321 // = ((1024/300)*360)/(pi*2)

/**
 * Räknar ut vilka vinklar servona ska ha för att komma till
 * positionen (x, y, z) cm och flyttar dem dit.
 *
 * \param legid    ben (1..6)
 * \param x, y, z  koordinater i cm, gör inga bound-checkar
 */
void moveLegTo(uint8_t legid, double x, double y, double z)
{
	Vector pos = {x, y, z};
	Rotation rot = ik(fix_leg_vector(pos));
	
	uint16_t coxa_gamma = RAD_TO_AX_DEG*rot.gamma + 511;
	uint16_t femur_alpha = RAD_TO_AX_DEG*rot.alpha + 553;
	uint16_t tibia_beta = 624 - RAD_TO_AX_DEG*rot.beta;
	
	SetPositionAX(LEGS[legid-1][COXA],  coxa_gamma );
	SetPositionAX(LEGS[legid-1][FEMUR], femur_alpha);
	SetPositionAX(LEGS[legid-1][TIBIA], tibia_beta );
}

void tiltTo(double angle) {
	Vector* positions = get_rotation_at(angle);
	for (byte legid = 0; legid < 6; legid++) {
		positions[legid] = world_to_local(legid+1, positions[legid]);
	}
	
	for (byte legid = 1; legid <= 6; legid++) {
		moveLegTo(legid, positions[legid-1].x, positions[legid-1].y, positions[legid-1].z);
	}
}

// "Kopia" av datat i ik.c
Vector sINITIAL_POSITIONS[6] = {
	{-20,  20, -8},
	{ 20,  20, -8},
	{-20,   0, -8},
	{ 20,   0, -8},
	{-20, -20, -8},
	{ 20, -20, -8}
};

void setStartPosition() {
	Vector positions[6];
	
	for (byte legid = 0; legid < 6; legid++) {
		positions[legid] = world_to_local(legid+1, sINITIAL_POSITIONS[legid]);
	}
	
	for (byte legid = 1; legid <= 6; legid++) {
		moveLegTo(legid, positions[legid-1].x, positions[legid-1].y, positions[legid-1].z);
	}
}
