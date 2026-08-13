/*
 * pid.h
 *
 *  Created on: 4 wrz 2023
 *      Author: rober
 */

#ifndef SVLIB_INC_PID_CUSTOM_H_
#define SVLIB_INC_PID_CUSTOM_H_

#include "global.h"

typedef struct {

    float Ts;               ///< Sampling time (seconds)
    float maxOutput;        ///< Maximum control output magnitude
    float kp;               ///< Proportional gain
    float ki_Ts;            ///< Integral gain times Ts
    float kd_Ts;            ///< Derivative gain divided by Ts
    float emaAlpha;         ///< Weight factor of derivative EMA filter.
    float prevInput;        ///< (Filtered) previous input for derivative.
    uint16_t activityCount; ///< How many ticks since last setpoint change.
    uint16_t activityThres; ///< Threshold for turning off the output.
    uint8_t errThres;       ///< Threshold with hysteresis.
    int32_t integral;       ///< Sum of previous errors for integral.
    uint16_t setpoint;      ///< Position reference.

}PID_t;

void PID_Init(PID_t* pid, float _kp, float _ki, float _kd, float _Ts, float _fc, float _maxOutput);

/// Update the controller: given the current position, compute the control
/// action.
float PID_update(PID_t* pid, uint16_t input);

void setKp(PID_t* pid, float kp); ///< Proportional gain
void setKi(PID_t* pid, float ki); ///< Integral gain
void setKd(PID_t* pid, float kd); ///< Derivative gain

float getKp(PID_t* pid); ///< Proportional gain
float getKi(PID_t* pid); ///< Integral gain
float getKd(PID_t* pid); ///< Derivative gain

/// Set the cutoff frequency (-3 dB point) of the exponential moving average
/// filter that is applied to the input before taking the difference for
/// computing the derivative term.
void setEMACutoff(PID_t* pid, float f_c);

/// Set the reference/target/setpoint of the controller.
void PID_setSetpoint(PID_t* pid, uint16_t setpoint);

/// @see @ref setSetpoint(int16_t)
uint16_t getSetpoint(PID_t* pid);

/// Set the maximum control output magnitude. Default is 255, which clamps
/// the control output in [-255, +255].
void setMaxOutput(PID_t* pid, float maxOutput);
/// @see @ref setMaxOutput(float)
float getMaxOutput(PID_t* pid);

/// Reset the activity counter to prevent the motor from turning off.
void resetActivityCounter(PID_t* pid);

/// Set the number of seconds after which the motor is turned off, zero to
/// keep it on indefinitely.
void setActivityTimeout(PID_t* pid, float s);

/// Reset the sum of the previous errors to zero.
void resetIntegral(PID_t* pid);

#endif /* SVLIB_INC_PID_CUSTOM_H_ */
