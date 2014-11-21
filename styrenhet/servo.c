#include "ax12.h"
#include "servo.h"
#include <util/delay.h>
#include <math.h>

uint8_t sLEGS[6][3] = LEGS_ARRAY;


ResponsePacket SetTorqueAX(byte id, uint16_t value) {
	return Write16AX(id, AX_TORQUE_LIMIT_L, value, false);
}

ResponsePacket SetSpeedAX(byte id, uint16_t value) {
	return Write16AX(id, AX_GOAL_SPEED_L, value, false);
}

uint16_t FixMirroring(byte id, uint16_t value) {
	// Genom att inte spegelvända COXA-leden får alla ben samma
	// referens-axlar
	for (byte legid = 0; legid < 6; legid++) {
		if (id == sLEGS[legid][COXA])
			return value;
	}
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
