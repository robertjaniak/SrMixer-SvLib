/*
 * pid.c
 *
 *  Created on: 4 wrz 2023
 *      Author: rober
 */

#include "pid_simple.h"
#include "math.h"

static float calcAlphaEMA(float fn);

/// Very basic, mostly educational PID controller with derivative filter.

void PID_Simple_Init(PID_t* pid, float _kp, float _ki, float _kd, float _fc, float _Ts){

/// @param  kp  Proportional gain   @f$ K_p @f$
/// @param  ki  Integral gain       @f$ K_i @f$
/// @param  kd  Derivative gain     @f$ K_d @f$
/// @param  fc  Cutoff frequency    @f$ f_c @f$ of derivative filter in Hz
/// @param  Ts  Controller sampling time    @f$ T_s @f$ in seconds
/// The derivative filter can be disabled by setting `fc` to zero.

	pid->kp = _kp;
	pid->ki = _ki;
	pid->kd = _kd;
	pid->alpha = calcAlphaEMA(_fc * _Ts);
	pid->Ts = _Ts;
	pid->max_output = 255;
	pid->integral = 0;
	pid->old_ef = 0;

}


/// Update the controller with the given position measurement `meas_y` and
/// return the new control signal.

float PID_Simple_Update(PID_t* pid, float reference, float meas_y) {
    // e[k] = r[k] - y[k], error between setpoint and true position
    float error = reference - meas_y;
    // e_f[k] = α e[k] + (1-α) e_f[k-1], filtered error
    float ef = pid->alpha * error + (1 - pid->alpha) * pid->old_ef;
    // e_d[k] = (e_f[k] - e_f[k-1]) / Tₛ, filtered derivative
    float derivative = (ef - pid->old_ef) / pid->Ts;
    // e_i[k+1] = e_i[k] + Tₛ e[k], integral
    float new_integral = pid->integral + error * pid->Ts;

    // PID formula:
    // u[k] = Kp e[k] + Ki e_i[k] + Kd e_d[k], control signal
    float control_u = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;

    // Clamp the output
	if (control_u > pid->max_output)
		control_u = pid->max_output;
	else if (control_u < -pid->max_output)
		control_u = -pid->max_output;
	else // Anti-windup
		pid->integral = new_integral;

    // store the state for the next iteration
    pid->old_ef = ef;
    // return the control signal
    return control_u;
}


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
