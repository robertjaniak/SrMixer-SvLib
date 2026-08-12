/*
 * motor.c
 *
 *  Created on: 9 lip 2022
 *      Author: rober
 */

#include <stdlib.h>
#include "motor.h"

#define MAX_PWM_DUTY 3000

/////////////////////////////////////////////////////////////////////////////////

static void motorMoveCW_PWM(Motor_PWM_t* motor, uint32_t speed){

	__HAL_TIM_SET_COMPARE(motor->A.htim, motor->A.channel, speed);
	__HAL_TIM_SET_COMPARE(motor->B.htim, motor->B.channel, 0);
	HAL_TIM_PWM_Start(motor->A.htim, motor->A.channel);
	HAL_TIM_PWM_Start(motor->B.htim, motor->B.channel);
}

static void motorMoveCCW_PWM(Motor_PWM_t* motor, uint32_t speed){

	__HAL_TIM_SET_COMPARE(motor->A.htim, motor->A.channel, 0);
	__HAL_TIM_SET_COMPARE(motor->B.htim, motor->B.channel, speed);
	HAL_TIM_PWM_Start(motor->A.htim, motor->A.channel);
	HAL_TIM_PWM_Start(motor->B.htim, motor->B.channel);
}

static void motorStop_PWM(Motor_PWM_t* motor){

	__HAL_TIM_SET_COMPARE(motor->A.htim, motor->A.channel, 0);
	__HAL_TIM_SET_COMPARE(motor->B.htim, motor->B.channel, 0);
	HAL_TIM_PWM_Stop(motor->A.htim, motor->A.channel);
	HAL_TIM_PWM_Stop(motor->B.htim, motor->B.channel);
}

void updatePWMSpeed(Motor_t* motor){

	switch (motor->direction) {
		case MOTOR_DIR_RIGHT:
			__HAL_TIM_SET_COMPARE(motor->outPwm.A.htim, motor->outPwm.A.channel, abs(motor->speed));
			__HAL_TIM_SET_COMPARE(motor->outPwm.B.htim, motor->outPwm.B.channel, 0);
			break;
		case MOTOR_DIR_LEFT:
			__HAL_TIM_SET_COMPARE(motor->outPwm.A.htim, motor->outPwm.A.channel, 0);
			__HAL_TIM_SET_COMPARE(motor->outPwm.B.htim, motor->outPwm.B.channel, abs(motor->speed));
			break;
		case MOTOR_DIR_NOT_SET:
			__HAL_TIM_SET_COMPARE(motor->outPwm.A.htim, motor->outPwm.A.channel, 0);
			__HAL_TIM_SET_COMPARE(motor->outPwm.B.htim, motor->outPwm.B.channel, 0);
			break;
		default:
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

static void motorMoveCW_GPIO(Motor_GPIO_t* motor){

	HAL_GPIO_WritePin(motor->A.port, motor->A.pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(motor->B.port, motor->B.pin, GPIO_PIN_RESET);
}

static void motorMoveCCW_GPIO(Motor_GPIO_t* motor){

	HAL_GPIO_WritePin(motor->A.port, motor->A.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(motor->B.port, motor->B.pin, GPIO_PIN_SET);
}

static void motorStop_GPIO(Motor_GPIO_t* motor){

	HAL_GPIO_WritePin(motor->A.port, motor->A.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(motor->B.port, motor->B.pin, GPIO_PIN_RESET);
}

//////////////////////////////////////////////////////////////////////////////////////////////

static void startMotorGPIO(Motor_t* motor){

	switch (motor->direction){

		case MOTOR_DIR_RIGHT:
			motor->state = MOTOR_STATE_MOVING_RIGHT;
			motorMoveCW_GPIO(&motor->outGpio);
			break;

		case MOTOR_DIR_LEFT:
			motor->state = MOTOR_STATE_MOVING_LEFT;
			motorMoveCCW_GPIO(&motor->outGpio);
			break;

		default:
			break;
	}
}

static void startMotorPWM(Motor_t* motor){

	int32_t speed;

	speed = ((int32_t)__HAL_TIM_GET_AUTORELOAD(motor->outPwm.A.htim) * motor->speed) / (int32_t)MAX_PWM_DUTY;

	switch (motor->direction){

		case MOTOR_DIR_RIGHT:
			motor->state = MOTOR_STATE_MOVING_RIGHT;
			motorMoveCW_PWM(&motor->outPwm, abs(speed));
			break;

		case MOTOR_DIR_LEFT:
			motor->state = MOTOR_STATE_MOVING_LEFT;
			motorMoveCCW_PWM(&motor->outPwm, abs(speed));
			break;

		case MOTOR_DIR_NOT_SET:
			motor->state = MOTOR_STATE_STOP;
			motorStop_PWM(&motor->outPwm);
			break;

		default:
			break;
	}
}

static void start(Motor_t* motor){

	TimerStart(&motor->timer);

	switch(motor->type){

		case MOTOR_GPIO:
			startMotorGPIO(motor);
			break;

		case MOTOR_PWM:
			startMotorPWM(motor);
			break;

		default:
			break;
	}
}

static void stop(Motor_t* motor){

	TimerStop(&motor->timer);
	TimerReset(&motor->timer);

	motor->state = MOTOR_STATE_STOP;
	motor->direction = MOTOR_DIR_NOT_SET;

	switch(motor->type){

		case MOTOR_GPIO:
			motorStop_GPIO(&motor->outGpio);
			break;

		case MOTOR_PWM:
			motorStop_PWM(&motor->outPwm);
			break;

		default:
			break;
	}
}

static void restart(Motor_t* motor){

	if (motor->state != MOTOR_STATE_STOP){
		stop(motor);
		start(motor);
	}
}


static void setDirection(Motor_t* motor, Motor_direction dir){

	motor->direction = dir;

	if(motor->direction == MOTOR_DIR_RIGHT){
		motor->speed = abs(motor->speed);
	}
	else if (motor->direction == MOTOR_DIR_LEFT){
		motor->speed = -abs(motor->speed);
	} else {
		motor->speed = 0;
	}
}

static void standardStart(Motor_t* motor){
	start(motor);
}

static void softStart(Motor_t* motor){

	Timer_Init(motor, &motor->ramp.timer, motor->ramp.accelTime, TIMER_MODE_ONE_SHOT);
	TimerStart(&motor->ramp.timer);

	motor->ramp.state = MOTOR_RAMP_ACCEL;

	motor->ramp.targetSpeed = motor->speed;
	motor->ramp.initSpeed = motor->speed;

	start(motor);
}

static void standardStop(Motor_t* motor){
	stop(motor);
}

static void softStop(Motor_t* motor){

	Timer_Init(motor, &motor->ramp.timer, motor->ramp.decelTime, TIMER_MODE_ONE_SHOT);
	TimerStart(&motor->ramp.timer);

	motor->ramp.state = MOTOR_RAMP_DECEL;

	motor->ramp.targetSpeed = motor->minSpeed;
	motor->ramp.initSpeed = motor->speed;

}

static void startMotor(Motor_t* motor){

	switch (motor->mode) {

		case MOTOR_MODE_NORMAL:
		case MOTOR_MODE_SOFT_STOP:
			standardStart(motor);
			break;

		case MOTOR_MODE_RAMP:
		case MOTOR_MODE_SOFT_START:
			softStart(motor);
			break;

		default:
			break;
	}
}

static void stopMotor(Motor_t* motor) {

	switch (motor->mode) {

		case MOTOR_MODE_NORMAL:
		case MOTOR_MODE_SOFT_START:
			standardStop(motor);
			break;

		case MOTOR_MODE_RAMP:
		case MOTOR_MODE_SOFT_STOP:
			softStop(motor);
			break;

		default:
			break;
	}
}

static void setSpeed(Motor_t* motor, int32_t speed){

	if (speed > 0){
		motor->direction = MOTOR_DIR_RIGHT;
		if (speed > MAX_PWM_DUTY) speed = MAX_PWM_DUTY;
	}

	else if (speed < 0){
		motor->direction = MOTOR_DIR_LEFT;
		if (speed < -MAX_PWM_DUTY) speed = -MAX_PWM_DUTY;
	}

	else {
		motor->direction = MOTOR_DIR_NOT_SET;
	}

	motor->speed = speed;
}

static void accelerateRamp(Motor_t* motor){

	int32_t speed;

	speed = ((motor->ramp.targetSpeed - motor->ramp.initSpeed) / motor->ramp.accelTime) * Timer_GetTime(&motor->ramp.timer);

	motor->speed =  speed + motor->ramp.initSpeed;


	if (motor->speed >= motor->maxSpeed){
		motor->speed = motor->maxSpeed;
	}

	if (TimerElapsed(&motor->ramp.timer)){
		motor->ramp.state = MOTOR_RAMP_CONST;
	}

	if (motor->timeout > 0){
		if (motor->timeout - Timer_GetTime(&motor->timer) <= motor->ramp.decelTime) {
			softStop(motor);
		}
	}
}

static void decelerateRamp(Motor_t* motor){

	int32_t speed;

	speed = ((motor->ramp.targetSpeed - motor->ramp.initSpeed) / motor->ramp.accelTime) * Timer_GetTime(&motor->ramp.timer);

	motor->speed =  speed + motor->ramp.initSpeed;

	if (motor->speed <= motor->minSpeed){
		motor->speed = motor->minSpeed;
	}

	if (TimerElapsed(&motor->ramp.timer)){
		motor->ramp.state = MOTOR_RAMP_STOP;
		stop(motor);
	}
}

static void constansRamp(Motor_t* motor){

	TimerStop(&motor->ramp.timer);
	TimerReset(&motor->ramp.timer);

	if (motor->timeout > 0){
		if (motor->timeout - Timer_GetTime(&motor->timer) <= motor->ramp.decelTime) {
			softStop(motor);
		}
	}
}

static void rampService(Motor_t* motor){

	TimerService(&motor->ramp.timer);

	switch (motor->ramp.state){

		case MOTOR_RAMP_STOP:
			TimerStop(&motor->ramp.timer);
			TimerReset(&motor->ramp.timer);
			break;

		case MOTOR_RAMP_ACCEL:
			accelerateRamp(motor);
			break;

		case MOTOR_RAMP_CONST:
			constansRamp(motor);
			break;

		case MOTOR_RAMP_DECEL:
			decelerateRamp(motor);
			break;

		default:
			break;
	}
}


////////////////////////////////////////////////////

void Motor_SetSpeed(Motor_t* motor, int32_t speed){
	setSpeed(motor,speed);
}

void Motor_SetSpeed_F(Motor_t* motor, float speed){
	setSpeed(motor,speed*3000);
}

int32_t Motor_GetSpeed(Motor_t* motor){
		return motor->speed;
}

void Motor_SetDirection(Motor_t* motor, Motor_direction dir){
	setDirection(motor, dir);
	restart(motor);
}

void Motor_SetTime(Motor_t* motor, uint32_t time){
	motor->timeout = time;
}

Motor_direction Motor_GetDirection(Motor_t* motor){
	return motor->direction;
}

void Motor_ChangeDirection(Motor_t* motor){

	switch (motor->direction){

		case MOTOR_DIR_RIGHT:
			setDirection(motor, MOTOR_DIR_LEFT);
			break;

		case MOTOR_DIR_LEFT:
			setDirection(motor, MOTOR_DIR_RIGHT);
			break;

		default:
			break;
	}
	restart(motor);
}

Motor_state Motor_GetState(Motor_t* motor){
	return motor->state;
}

void Motor_Move(Motor_t* motor, int32_t speed){

	setSpeed(motor, speed);
	startMotor(motor);
}

void Motor_Start(Motor_t* motor) {
	startMotor(motor);
}

void Motor_Stop(Motor_t* motor){
	stopMotor(motor);
}

void Motor_MoveInTime_BlockingMode(Motor_t* motor, int32_t speed, uint32_t time){

	uint32_t startTime, timeout;

	setSpeed(motor, speed);

	motor->timeout = time;

	startTime = HAL_GetTick();
	timeout = 0;

	startMotor(motor);

	while (timeout < time){
		timeout = HAL_GetTick() - startTime;
	}

	stopMotor(motor);
}

void Motor_MoveInTime_NonBlockingMode(Motor_t* motor, int32_t speed, uint32_t time){

	motor->timeout = time;

	setSpeed(motor, speed);
	startMotor(motor);
}

void Motor_Service(Motor_t* motor){

	TimerService(&motor->timer);

	if (motor->timeout > 0){
		if ((Timer_GetTime(&motor->timer) >= motor->timeout)){
			stop(motor);
		}
	}

	if (motor->mode != MOTOR_MODE_NORMAL){
		rampService(motor);
	}

}

void Motor_RampInit(Motor_t* motor, uint32_t accelTime, uint32_t decelTime){

	motor->ramp.accelTime = accelTime;
	motor->ramp.decelTime = decelTime;

}

//////////////////////////////////////////////////////////////////////////////////////////////

void Motor_GPIO_Init(Motor_t* motor, GPIO_TypeDef *port_A, uint16_t pin_A, GPIO_TypeDef *port_B, uint16_t pin_B, const char* name){

	motor->name = name;

	motor->outGpio.A.port = port_A;
	motor->outGpio.A.pin  =  pin_A;
	motor->outGpio.B.port = port_B;
	motor->outGpio.B.pin  =  pin_B;
	motor->type = MOTOR_GPIO;
	motor->mode = MOTOR_MODE_NORMAL;

	motor->direction = MOTOR_DIR_RIGHT;
	motor->state = MOTOR_STATE_STOP;
	motor->timeout = 0;

	Timer_Init(motor, &motor->timer, 0, TIMER_MODE_CONTINOUS);
}

void Motor_PWM_Init(Motor_t* motor, Motor_mode mode,
		TIM_HandleTypeDef* htim_A, uint32_t channel_A, TIM_HandleTypeDef*
		htim_B, uint32_t channel_B,
		const char* name, int32_t minSpeed, int32_t maxSpeed)
{

	motor->name = name;

	motor->outPwm.A.htim     = htim_A;
	motor->outPwm.A.channel  = channel_A;
	motor->outPwm.B.htim     = htim_B;
	motor->outPwm.B.channel  = channel_B;
	motor->type = MOTOR_PWM;
	motor->mode = mode;

	motor->direction = MOTOR_DIR_NOT_SET;
	motor->state = MOTOR_STATE_STOP;


	motor->minSpeed = minSpeed;
	motor->maxSpeed = maxSpeed;
	motor->speed = 0;

	motor->ramp.targetSpeed = 0;
	motor->ramp.initSpeed = minSpeed;
	motor->ramp.accelTime = 1000;
	motor->ramp.decelTime = 1000;

	motor->timeout = 0;
	Timer_Init(motor, &motor->timer, 0, TIMER_MODE_CONTINOUS);
}

//////////////////////////////////////////////////////////////////////////////////////////////


