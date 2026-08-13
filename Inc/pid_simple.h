/*
 * pid.h
 *
 *  Created on: 4 wrz 2023
 *      Author: rober
 */

#ifndef SVLIB_INC_PID_SIMPLE_H_
#define SVLIB_INC_PID_SIMPLE_H_

#include "global.h"

typedef struct {

	float kp;
	float ki;
	float kd;
	float alpha;
	float Ts;
	float max_output;
	float integral;
	float old_ef;

}PID_t;

void PID_Simple_Init(PID_t* pid, float _kp, float _ki, float _kd, float _fc, float _Ts);
float PID_Simple_Update(PID_t* pid, float reference, float meas_y);

#endif /* SVLIB_INC_PID_SIMPLE_H_ */
