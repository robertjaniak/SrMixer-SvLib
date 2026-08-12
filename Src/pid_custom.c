/*
 * pid.c
 *
 *  Created on: 4 wrz 2023
 *      Author: rober
 */

#include "pid_custom.h"
#include "math.h"

static float calcAlphaEMA(float fn);

/// Standard PID (proportional, integral, derivative) controller. Derivative
/// component is filtered using an exponential moving average filter.

void PID_Init(PID_t* pid, float _kp, float _ki, float _kd, float _Ts, float _fc, float _maxOutput) {

    /// @param  kp
    ///         Proportional gain
    /// @param  ki
    ///         Integral gain
    /// @param  kd
    ///         Derivative gain
    /// @param  Ts
    ///         Sampling time (seconds)
    /// @param  fc
    ///         Cutoff frequency of derivative EMA filter (Hertz),
    ///         zero to disable the filter entirely


	pid->Ts = 1;               ///< Sampling time (seconds)
    pid->maxOutput = 255;      ///< Maximum control output magnitude
    pid->kp = 1;               ///< Proportional gain
    pid->ki_Ts = 0;            ///< Integral gain times Ts
    pid->kd_Ts = 0;            ///< Derivative gain divided by Ts
    pid->emaAlpha = 1;         ///< Weight factor of derivative EMA filter.
    pid->prevInput = 0;        ///< (Filtered) previous input for derivative.
    pid->activityCount = 0; ///< How many ticks since last setpoint change.
    pid->activityThres = 0; ///< Threshold for turning off the output.
    pid->errThres = 1;       ///< Threshold with hysteresis.
    pid->integral = 0;       ///< Sum of previous errors for integral.
    pid->setpoint = 0;      ///< Position reference.

    setKp(pid, _kp);
    setKi(pid, _ki);
    setKd(pid, _kd);
    setEMACutoff(pid, _fc);

    pid->Ts = _Ts;
    pid->maxOutput = _maxOutput;

}


/// Update the controller: given the current position, compute the control
/// action.
float PID_update(PID_t* pid, uint16_t input) {
    // The error is the difference between the reference (setpoint) and the
    // actual position (input)
    int16_t error = pid->setpoint - input;
    // The integral or sum of current and previous errors
    int32_t newIntegral = pid->integral + error;
    // Compute the difference between the current and the previous input,
    // but compute a weighted average using a factor α ∊ (0,1]
    float diff = pid->emaAlpha * (pid->prevInput - input);
    // Update the average
    pid->prevInput -= diff;

    // Check if we can turn off the motor
    if (pid->activityCount >= pid->activityThres && pid->activityThres) {
        float filtError = pid->setpoint - pid->prevInput;
        if (filtError >= -pid->errThres && filtError <= pid->errThres) {
        	pid->errThres = 2; // hysteresis
            return 0;
        } else {
        	pid->errThres = 1;
        }
    } else {
        ++pid->activityCount;
        pid->errThres = 1;
    }

    uint8_t backward = 0;
    int32_t calcIntegral = backward ? newIntegral : pid->integral;

    // Standard PID rule
    float output = pid->kp * error + pid->ki_Ts * calcIntegral + pid->kd_Ts * diff;

    // Clamp and anti-windup
    if (output > pid->maxOutput)
        output = pid->maxOutput;
    else if (output < -pid->maxOutput)
        output = -pid->maxOutput;
    else
    	pid->integral = newIntegral;

    return output;
}

void setKp(PID_t* pid, float kp) { pid->kp = kp; }               ///< Proportional gain
void setKi(PID_t* pid, float ki) { pid->ki_Ts = ki * pid->Ts; }  ///< Integral gain
void setKd(PID_t* pid, float kd) { pid->kd_Ts = kd / pid->Ts; }  ///< Derivative gain

float getKp(PID_t* pid) { return pid->kp; }         ///< Proportional gain
float getKi(PID_t* pid) { return pid->ki_Ts / pid->Ts; } ///< Integral gain
float getKd(PID_t* pid) { return pid->kd_Ts * pid->Ts; } ///< Derivative gain

/// Set the cutoff frequency (-3 dB point) of the exponential moving average
/// filter that is applied to the input before taking the difference for
/// computing the derivative term.
void setEMACutoff(PID_t* pid, float f_c) {
	float f_n = f_c * pid->Ts; // normalized sampling frequency
	pid->emaAlpha = f_c == 0 ? 1 : calcAlphaEMA(f_n);
}

/// Set the reference/target/setpoint of the controller.
void PID_setSetpoint(PID_t* pid, uint16_t setpoint) {
	if (pid->setpoint != setpoint) pid->activityCount = 0;
	pid->setpoint = setpoint;
}

/// @see @ref setSetpoint(int16_t)
uint16_t getSetpoint(PID_t* pid) { return pid->setpoint; }

/// Set the maximum control output magnitude. Default is 255, which clamps
/// the control output in [-255, +255].
void setMaxOutput(PID_t* pid, float maxOutput) { pid->maxOutput = maxOutput; }
/// @see @ref setMaxOutput(float)
float getMaxOutput(PID_t* pid){ return pid->maxOutput; }

/// Reset the activity counter to prevent the motor from turning off.
void resetActivityCounter(PID_t* pid) { pid->activityCount = 0; }
/// Set the number of seconds after which the motor is turned off, zero to
/// keep it on indefinitely.
void setActivityTimeout(PID_t* pid, float s) {
	if (s == 0)
		pid->activityThres = 0;
	else
		pid->activityThres = (uint16_t)(s / pid->Ts) == 0 ? 1 : s / pid->Ts;
}

/// Reset the sum of the previous errors to zero.
void resetIntegral(PID_t* pid) { pid->integral = 0; }



/// Compute the weight factor α for an exponential moving average filter
/// with a given normalized cutoff frequency `fn`.
static float calcAlphaEMA(float _fn) {

    if (_fn <= 0) {
        return 1;
    }

    // α(fₙ) = cos(2πfₙ) - 1 + √( cos(2πfₙ)² - 4 cos(2πfₙ) + 3 )

    const float c = cos(2 * (float)M_PI * _fn);
    return c - 1 + sqrt(c * c - 4 * c + 3);
}
