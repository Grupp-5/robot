#include "ax12.h"
#include "servo.h"
#include <util/delay.h>

ResponsePacket SetTorqueAX(byte id, uint16_t value) {
	return Write16AX(id, AX_TORQUE_LIMIT_L, value, false);
}

ResponsePacket SetSpeedAX(byte id, uint16_t value) {
	return Write16AX(id, AX_GOAL_SPEED_L, value, false);
}

uint16_t FixMirroring(byte id, uint16_t value) {
	if (id%2) return 1023 - value;
	return value;
}

ResponsePacket SetPositionAX(byte id, uint16_t value) {
	return Write16AX(id, AX_GOAL_POSITION_L,
	                 FixMirroring(id, value), false);
}

ResponsePacket TorqueEnableAX(byte id) {
	return Write8AX(id, AX_TORQUE_ENABLE, 1, false);
}


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
