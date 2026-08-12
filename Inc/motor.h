/*
 * motor.h
 *
 *  Created on: 9 lip 2022
 *      Author: rober
 */

#include "global.h"
#include "timer.h"

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

typedef enum {
	MOTOR_GPIO,
	MOTOR_PWM,
} Motor_type;

typedef enum {
	MOTOR_MODE_NORMAL   = 0,
	MOTOR_MODE_SOFT_START = 1,
	MOTOR_MODE_SOFT_STOP  = 2,
	MOTOR_MODE_RAMP       = 3,
} Motor_mode;

typedef enum {
	MOTOR_DIR_NOT_SET,
	MOTOR_DIR_LEFT,
	MOTOR_DIR_RIGHT,
} Motor_direction;

typedef enum {
	MOTOR_STATE_STOP,
	MOTOR_STATE_MOVING_LEFT,
	MOTOR_STATE_MOVING_RIGHT,
} Motor_state;

typedef enum {
	MOTOR_RAMP_STOP    = 0,
	MOTOR_RAMP_ACCEL   = 1,
	MOTOR_RAMP_CONST   = 2,
	MOTOR_RAMP_DECEL   = 3,
} Ramp_state;

typedef struct {
	Ramp_state state;
	Timer_t timer;
	int32_t initSpeed;
	int32_t targetSpeed;
	int32_t accelTime;
	int32_t decelTime;
} Motor_ramp;

typedef struct {
	IOPin A;
	IOPin B;
} Motor_GPIO_t;

typedef struct {
	PWMOutput A;
	PWMOutput B;
} Motor_PWM_t;

typedef struct {

	char* name;

	union {
		Motor_GPIO_t outGpio;
		Motor_PWM_t  outPwm;
	};

	Motor_type type;
	Motor_mode mode;
	Motor_state state;
	Motor_direction direction;

	uint16_t minSpeed;
	uint16_t maxSpeed;
	int32_t speed;

	Timer_t timer;
//	int32_t stopTime;
	int32_t timeout;

	Motor_ramp ramp;

////// To DO  /////////
//	void (*Motor_Start_Callback)(void);
//	void (*Motor_Stop_Callback)(void);
//	void (*Motor_ChangedSpeed_Callback)(void);
//	void (*Motor_ChangedDirection_Callback)(void);
//	void (*Motor_HalfTime_Callback)(void);

} Motor_t;

void Motor_GPIO_Init(Motor_t* motor, GPIO_TypeDef *port_A, uint16_t pin_A, GPIO_TypeDef *port_B, uint16_t pin_B, const char* name);
void Motor_PWM_Init(Motor_t* motor, Motor_mode mode, TIM_HandleTypeDef* htim_A, uint32_t channel_A, TIM_HandleTypeDef* htim_B, uint32_t channel_B, const char* name, int32_t minSpeed, int32_t maxSpeed);
void Motor_RampInit(Motor_t* motor, uint32_t accel_time, uint32_t decel_time);

void Motor_Service(Motor_t* motor);

void Motor_SetSpeed(Motor_t* motor, int32_t speed);
void Motor_SetSpeed_F(Motor_t* motor, float speed);
int32_t Motor_GetSpeed(Motor_t* motor);

void Motor_SetTime(Motor_t* motor, uint32_t time);

void Motor_SetDirection(Motor_t* motor, Motor_direction direction);
Motor_direction Motor_GetDirection(Motor_t* motor);
void Motor_ChangeDirection(Motor_t* motor);

Motor_state Motor_GetState(Motor_t* motor);

void Motor_Move(Motor_t* motor, int32_t speed);
void Motor_Start(Motor_t* motor);
void Motor_Stop(Motor_t* motor);

void Motor_MoveInTime_BlockingMode(Motor_t* motor, int32_t speed, uint32_t time); // only in STANDARD mode
void Motor_MoveInTime_NonBlockingMode(Motor_t* motor, int32_t speed, uint32_t time); // only in STANDARD mode

void updatePWMSpeed(Motor_t* motor);

#endif /* INC_MOTOR_H_ */
