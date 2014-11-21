#include "ax12.h"
#include "servo.h"
#include <util/delay.h>
#include <math.h>

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
