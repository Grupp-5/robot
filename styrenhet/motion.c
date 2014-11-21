/*
 * motion.c
 *
 * Created: 11/21/2014 4:31:47 AM
 *  Author: dansj232, geoza435
 */ 

#include "servo.h"
#include "motion.h"
#include <math.h>

#define F_CPU 16000000UL
#include <util/delay.h>

void SendCoxa(uint16_t value) {
	SetPositionAX(1, value);
	SetPositionAX(13, value);
	SetPositionAX(7, value);
	SetPositionAX(8, value);
	SetPositionAX(14, value);
	SetPositionAX(2, value);
}

void SendFemur(uint16_t value) {
	SetPositionAX(3, value);
	SetPositionAX(15, value);
	SetPositionAX(9, value);
	SetPositionAX(10, value);
	SetPositionAX(16,value);
	SetPositionAX(4, value);
}

void SendRFemur(uint16_t value) {
	SetPositionAX(15, value);
	SetPositionAX(10, value);
	SetPositionAX(4, value);
}

void SendLFemur(uint16_t value) {
	SetPositionAX(3, value);
	SetPositionAX(9, value);
	SetPositionAX(16, value);
}

void SendTibia(uint16_t value) {
	SetPositionAX(5, value);
	SetPositionAX(17, value);
	SetPositionAX(11, value);
	SetPositionAX(12, value);
	SetPositionAX(18, value);
	SetPositionAX(6, value);
}

void SetLayPosition() {
	SendCoxa(511);
	SendFemur(1023-200);
	SendTibia(1023-777);
}

void SetStartPosition() {
	SendFemur(1023-296);
	
	_delay_ms(1000);
	
	// 3, 9, 16
	uint16_t value;
	SendLFemur(POS_LEG_UP);
	
	value = 1023-360;
	SetPositionAX(7, value);
	
	value = 360;
	SetPositionAX(1, value);
	
	_delay_ms(500);
	value = 1023-836;
	SetPositionAX(11, value);
	SetPositionAX(18, value);
	SetPositionAX(5, value);
	value = 1023-250;
	SetPositionAX(9, value);
	SetPositionAX(16, value);
	SetPositionAX(3, value);
	
	
	// Andra sidan
	_delay_ms(1000);
	
	SendRFemur(POS_LEG_UP);
	
	value = 1023-360;
	SetPositionAX(8, value);
	
	value = 360;
	SetPositionAX(2, value);
	
	_delay_ms(500);
	value = 1023-836;
	SetPositionAX(12, value);
	SetPositionAX(17, value);
	SetPositionAX(6, value);
	value = 1023-250;
	SetPositionAX(4, value);
	SetPositionAX(15, value);
	SetPositionAX(10, value);
}

void SetForward(bool right) {
	if (right) {
		//40 framåt från start
		SetPositionAX(2, 360+STEP_LENGTH);
		SetPositionAX(13, 511+(STEP_LENGTH/2));
		SetPositionAX(8, 663);
		} else {
		//40 framåt från start
		SetPositionAX(1, 360+STEP_LENGTH);
		SetPositionAX(14, 511+(STEP_LENGTH/2));
		SetPositionAX(7, 663);
	}
}

void SetBackward(bool right) {
	if (right) {
		//40 framåt från start
		SetPositionAX(2, 360);
		SetPositionAX(13, 511-(STEP_LENGTH/2));
		SetPositionAX(8, 663-STEP_LENGTH);
		} else {
		//40 framåt från start
		SetPositionAX(1, 360);
		SetPositionAX(14, 511-(STEP_LENGTH/2));
		SetPositionAX(7, 663-STEP_LENGTH);
	}
}

void Walk() {
	//Ben 2,3,6 upp->framåt->ner
	SendRFemur(POS_LEG_UP);
	_delay_ms(500);
	while(1) {
		SetForward(true);
		_delay_ms(50);
		SendRFemur(POS_LEG_DOWN);
		_delay_ms(50);
		//Ben 1,4,5 upp->framåt
		SendLFemur(POS_LEG_UP);
		//_delay_ms(10);
		SetForward(false);
		//Ben 2,3,6 bakåt
		SetBackward(true);
		
		//Delay
		_delay_ms(50);
		//Ben 1,4,5 ned
		SendLFemur(POS_LEG_DOWN);
		
		_delay_ms(50);
		SendRFemur(POS_LEG_UP);
		//_delay_ms(10);
		SetBackward(false);
	}
}

// Konverterar radianer till grader som AX12 förstår
#define RAD_TO_AX_DEG 195.569594071321 // = ((1024/300)*360)/(pi*2)
// Längder på ben i cm
#define C 5.0
#define F 6.6
#define T 13.3

// Led-nummer i LEGS-arrayen
#define COXA  0
#define FEMUR 1
#define TIBIA 2

uint8_t LEGS[6][3] = {
	{1 ,  3,  5},
	{2 ,  4,  6},
	{13, 15, 17},
	{14, 16, 18},
	{7 ,  9, 11},
	{8 , 10, 12}
};

/**
 * Räknar ut vilka vinklar servona ska ha för att komma till
 * positionen (x, y, z) cm och flyttar dem dit.
 *
 * \param legid    ben (1..6)
 * \param x, y, z  koordinater i cm, gör inga bound-checkar
 */
void moveLegTo(uint8_t legid, double x, double y, double z)
{
	
	double gamma = atan2(y, x);
	double CoxaX = C * cos(gamma);
	double CoxaY = C * sin(gamma);

	double L = sqrt(pow(x-CoxaX,2) + pow(y-CoxaY,2) + pow(z,2));
	double alpha1;
	if (z == 0) {
		alpha1 = M_PI/2;
	} else {
		alpha1 = acos(-z/L);
	}
	double alpha2 = acos((pow(F,2)+pow(L,2)-pow(T,2))/(2*L*F));

	double alpha = alpha1 + alpha2;

	double beta = acos((pow(F,2) + pow(T,2) - pow(L,2))/(2*F*T));

	alpha -= M_PI/2;
	beta = M_PI - beta;
	
	uint16_t coxa_gamma = RAD_TO_AX_DEG*gamma + 511;
	uint16_t femur_alpha = RAD_TO_AX_DEG*alpha + 553;
	uint16_t tibia_beta = 624 - RAD_TO_AX_DEG*beta;
	
	SetPositionAX(LEGS[legid-1][COXA],  coxa_gamma );
	SetPositionAX(LEGS[legid-1][FEMUR], femur_alpha);
	SetPositionAX(LEGS[legid-1][TIBIA], tibia_beta );
}
