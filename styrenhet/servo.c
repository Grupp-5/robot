#include "ax12.h"

ResponsePacket SetTorqueAX(byte id, uint16_t value) {
	return Write16AX(id, AX_TORQUE_LIMIT_L, value, false);
}

ResponsePacket SetSpeedAX(byte id, uint16_t value) {
	return Write16AX(id, AX_GOAL_SPEED_L, value, false);
}

ResponsePacket SetPositionAX(byte id, uint16_t value) {
	return Write16AX(id, AX_GOAL_POSITION_L, value, false);
}

ResponsePacket TorqueEnableAX(byte id) {
	return Write8AX(id, AX_TORQUE_ENABLE, 1, false);
}

uint16_t FixMirroring(byte id, uint16_t value) {
	if (id%2) return 1023 - value;
	return value; 
}

void SendCoxa(uint16_t value) {
	SetPositionAX(1, FixMirroring(1, value));
	SetPositionAX(13, FixMirroring(13, value));
	SetPositionAX(7, FixMirroring(7, value));
	SetPositionAX(8, FixMirroring(8, value));
	SetPositionAX(14, FixMirroring(14, value));
	SetPositionAX(2, FixMirroring(2, value));
}

void SendFemur(uint16_t value) {
	SetPositionAX(3, FixMirroring(3, value));
	SetPositionAX(15, FixMirroring(15, value));
	SetPositionAX(9, FixMirroring(9, value));
	SetPositionAX(10, FixMirroring(10, value));
	SetPositionAX(16, FixMirroring(16, value));
	SetPositionAX(4, FixMirroring(4, value));
}

void SendTibia(uint16_t value) {
	SetPositionAX(5, FixMirroring(5, value));
	SetPositionAX(17, FixMirroring(17, value));
	SetPositionAX(11, FixMirroring(11, value));
	SetPositionAX(12, FixMirroring(12, value));
	SetPositionAX(18, FixMirroring(18, value));
	SetPositionAX(6, FixMirroring(6, value));
}

void SetLayPosition() {
	SendCoxa(511);
	SendFemur(200);
	SendTibia(777);
}

void SetStartPosition() {	
	SendFemur(???);
}