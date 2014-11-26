/*
 * motion.h
 *
 * Created: 11/21/2014 4:31:00 AM
 *  Author: geoza435
 */ 


#ifndef MOTION_H_
#define MOTION_H_

#include "datatypes.h"
#include "vector_math.h"

void SetLayPosition();
void SetStartPosition();

void moveLegTo(byte legid, double x, double y, double z);

void setStartPosition();
void setLayPosition();
void moveLegSet(Vector left, Vector right);
void takeStep(double speed, double speed_forward, double speed_sideway, double rotation, double height_offset, double xrot, double yrot);

#define POS_LEG_UP 823
#define POS_LEG_DOWN 773
#define STEP_LENGTH 60

#endif /* MOTION_H_ */
