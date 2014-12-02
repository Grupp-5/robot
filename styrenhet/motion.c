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

#define start_z -12

// "Världs"-koordinater, sett från mitt på roboten.
// Positiva Y är framåt, positiva X är åt höger.
Vector INITIAL_POSITIONS[6] = {
	{-14,  21, start_z},
	{ 14,  21, start_z},
	{-23,   0, start_z},
	{ 23,   0, start_z},
	{-15, -23, start_z},
	{ 15, -23, start_z}
};


// Håll foten på marken en halv period
double z_step(double x) {
	double ret = sin(x*2*M_PI+3*M_PI_2);
	if(ret < 0) {
		ret = 0;
	}
	return ret;
}

// Gå framåt och bakåt (och vänster och höger) vid rätt tillfälle
double y_step(double x) {
	return -sin(x*2*M_PI);
}

// Snurra åt rätt håll
double rot_step(double x) {
	return sin(x*2*M_PI);
}

void stepAt(double at, double step_size_forward, double step_size_side, double rotation, double height_offset, double xrot, double yrot) {
	double max_step_height = 2.5;
	double max_forward_step = 6;
	double max_side_step = 5;
	double max_rotation = M_PI_4/3;
	double max_height_diff = 4;
	double max_x_rotation = M_PI_4/5;
	double max_y_rotation = M_PI_4/6;

	// Variabler som ökas/sänks över tid så att inga ryck sker
	static double
		left_step_f, right_step_f,
		left_step_s, right_step_s,
		cur_rotation, height, step_height,
		cur_x_rotation, cur_y_rotation;
	double interpolation = 20;
	double rot_interpolation = 20;

	// Börja mitt i ett steg
	at += 0.25;

	left_step_f  =  left_step_f + (step_size_forward-left_step_f)/interpolation;
	right_step_f = right_step_f + (step_size_forward-right_step_f)/interpolation;
	left_step_s	 =  left_step_s + (step_size_side-left_step_s)/interpolation;
	right_step_s = right_step_s + (step_size_side-right_step_s)/interpolation;
	cur_rotation = cur_rotation + (rotation-cur_rotation)/rot_interpolation;
	cur_x_rotation = cur_x_rotation + (xrot-cur_x_rotation)/rot_interpolation;
	cur_y_rotation = cur_y_rotation + (yrot-cur_y_rotation)/rot_interpolation;
	height = height + (height_offset-height)/interpolation;
	step_height = (sqrt(fabs(left_step_f)) + sqrt(fabs(right_step_s)) + sqrt(fabs(cur_rotation)))*max_step_height;

	Vector* positions = get_rotation_at(INITIAL_POSITIONS, cur_x_rotation*max_x_rotation, cur_y_rotation*max_y_rotation);

	positions = translate_set(
		positions,
		vector(
			y_step(at)*left_step_s*max_side_step,
			y_step(at)*left_step_f*max_forward_step,
			z_step(at)*step_height-height*max_height_diff),
		vector(
			y_step(at+0.5)*right_step_s*max_side_step,
			y_step(at+0.5)*right_step_f*max_forward_step,
			z_step(at+0.5)*step_height-height*max_height_diff)
	);

	Matrix left_rotation, right_rotation;
	make_rotation_z(left_rotation, rot_step(at)*cur_rotation*max_rotation);
	make_rotation_z(right_rotation, rot_step(at+0.5)*cur_rotation*max_rotation);

	positions = rotate_set(positions,
		left_rotation,
		right_rotation
	);

	for (byte legid = 0; legid < 6; legid++) {
		positions[legid] = world_to_local(legid+1, positions[legid]);
	}

	for (byte legid = 1; legid <= 6; legid++) {
		moveLegTo(legid, positions[legid-1].x, positions[legid-1].y, positions[legid-1].z);
	}
}

/** Fortsätt på stege(n/t)
 *
 * För att allt ska funka bra, bör man ta ett steg med 0 på alla
 * parametrar först.
 *
 * \param speed Hastighet från 0.0 till 1.0
 * \param speed_forward Steglängd framåt och bakåt -1.0 till 1.0
 * \param speed_sideway Steglängd höger och vänster -1.0 till 1.0
 * \param rotation Rotation medsols eller motsols -1.0 till 1.0
 * \param height_offsett Olika höjd på gången -1.0 till 1.0, 1.0 är så högt roboten kan komma
 * \param xrot Rotation i x-led (luta framåt eller bakåt)
 * \param yrot Rotation i y-led (luta vänster eller höger)
 */
void takeStep(double speed, double speed_forward, double speed_sideway, double rotation, double height_offset, double xrot, double yrot)
{
	uint16_t wait_delay = 0;
	double max_speed = 0.02;
	double delay = 0;
	static double step = 0;

	step += speed*max_speed;

	if (step > 1) { step = 0; _delay_ms(wait_delay); }

	stepAt(step, speed_forward, speed_sideway, rotation, height_offset, xrot, yrot);
	_delay_ms(delay);
}

void setStartPosition() {
	Vector positions[6];
	
	SetSpeedAX(ID_BROADCAST, 300);
	SetTorqueAX(ID_BROADCAST, 800);
	TorqueEnableAX(ID_BROADCAST);

	for (byte legid = 0; legid < 6; legid++) {
		positions[legid] = world_to_local(legid+1, INITIAL_POSITIONS[legid]);
	}

	for (byte legid = 1; legid <= 6; legid++) {
		moveLegTo(legid, positions[legid-1].x, positions[legid-1].y, positions[legid-1].z);
	}

	_delay_ms(2500);
}

void setLayPosition() {
	Vector* positions;

	double height = 3; // cm över marken

	positions = translate_set(
		INITIAL_POSITIONS,
		vector(0, 0, -start_z+height),
		vector(0, 0, -start_z+height)
	);

	// Flytta ut alla benen så att roboten inte försöker komma för nära med benen
	Matrix xy_offset = {
		{1.3,   0, 0},
		{  0, 1.3, 0},
		{  0,   0, 1}
	};

	positions = rotate_set(positions, xy_offset, xy_offset);

	SetSpeedAX(ID_BROADCAST, 50);
	SetTorqueAX(ID_BROADCAST, 600);
	TorqueEnableAX(ID_BROADCAST);

	for (byte legid = 0; legid < 6; legid++) {
		positions[legid] = world_to_local(legid+1, positions[legid]);
	}

	for (byte legid = 1; legid <= 6; legid++) {
		moveLegTo(legid, positions[legid-1].x, positions[legid-1].y, positions[legid-1].z);
	}
	_delay_ms(5000);
}
