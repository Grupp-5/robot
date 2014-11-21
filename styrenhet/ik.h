/*
 * ik.h
 *
 * Created: 11/21/2014 5:40:38 AM
 *  Author: geoza435
 */ 


#ifndef IK_H_
#define IK_H_

#include "datatypes.h"
#include "vector_math.h"

// Längder på ben i cm
#define C 5.0
#define F 6.6
#define T 13.3

Rotation ik(Vector pos);
Vector fix_leg_vector(Vector pos);
Vector world_to_local(byte legid, Vector pos);
Vector* get_rotation_at(double angle);

#endif /* IK_H_ */