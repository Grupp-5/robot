/*
 * motion.h
 *
 * Created: 11/21/2014 4:31:00 AM
 *  Author: geoza435
 */ 


#ifndef MOTION_H_
#define MOTION_H_

#include "datatypes.h"

void SetLayPosition();
void SetStartPosition();
void Walk();

void moveLegTo(byte legid, double x, double y, double z);
void tiltTo(double angle);

void setStartPosition();

#define POS_LEG_UP 823
#define POS_LEG_DOWN 773
#define STEP_LENGTH 60

#endif /* MOTION_H_ */