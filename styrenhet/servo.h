#ifndef _SERVO_H_
#define _SERVO_H_

#include "datatypes.h"

#define ID_BROADCAST 0xFE

ResponsePacket SetTorqueAX(byte id, uint16_t value);
ResponsePacket SetSpeedAX(byte id, uint16_t value);
ResponsePacket SetPositionAX(byte id, uint16_t value);
ResponsePacket TorqueEnableAX(byte id);

// Led-nummer i LEGS-arrayen
#define COXA  0
#define FEMUR 1
#define TIBIA 2

// Vackert fulhack
// Användning: uint8_t LEGS[6][3] = LEGS_ARRAY
#define LEGS_ARRAY \
{                  \
    {1 ,  3,  5},  \
    {2 ,  4,  6},  \
    {13, 15, 17},  \
    {14, 16, 18},  \
    {7 ,  9, 11},  \
    {8 , 10, 12}   \
}                  \

#endif
