#ifndef _SERVO_H_
#define _SERVO_H_

#include "ax12.h"

ResponsePacket SetTorqueAX(byte id, uint16_t value);
ResponsePacket SetSpeedAX(byte id, uint16_t value);
ResponsePacket SetPositionAX(byte id, uint16_t value);
ResponsePacket TorqueEnableAX(byte id);

void SetLayPosition();
void SetStartPosition();
void Walk();

void moveLegTo(byte legid, double x, double y, double z);

#define POS_LEG_UP 823
#define POS_LEG_DOWN 773
#define STEP_LENGTH 60

#endif
