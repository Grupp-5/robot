#ifndef _SERVO_H_
#define _SERVO_H_

#include "ax12.h"

ResponsePacket SetTorqueAX(byte id, uint16_t value);
ResponsePacket SetSpeedAX(byte id, uint16_t value);
ResponsePacket SetPositionAX(byte id, uint16_t value);
ResponsePacket TorqueEnableAX(byte id);

#endif
