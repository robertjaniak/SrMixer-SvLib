/*
 * potentiometr.h
 *
 *  Created on: Mar 14, 2021
 *      Author: Dom
 */

#ifndef INC_MOTOPOT_H_
#define INC_MOTOPOT_H_

#include "global.h"
#include "motor.h"
#include "pot.h"

typedef enum {
	MOTOPOT_STATUS_IDLE,
	MOTOPOT_STATUS_START,
	MOTOPOT_STATUS_MOTOR_STOP,
	MOTOPOT_STATUS_MOTOR_LEFT,
	MOTOPOT_STATUS_MOTOR_RIGHT,
	MOTOPOT_STATUS_HAND_LEFT,
	MOTOPOT_STATUS_HAND_RIGHT,
}Motopot_status;

struct ramp_s
{
	uint16_t target_speed;
	uint16_t accel;
	uint16_t decel;
	int16_t full_position;
	int16_t stop_position;
};

typedef enum {
	LINEAR,
	LOGARYTMIC,
}Motopot_type;

typedef enum {
	UP,
	DOWN,
}Motopot_direction;

typedef enum {
	MOTOPOT_MOTOR_GPIO,
	MOTOPOT_MOTOR_PWM,
} Motopot_motor_type;

typedef struct{
	Pot_t* pot;
	Motor_t* motor;
} Motopot_t;

typedef struct{
	char* name;
	//Analog_t  analog;

	union {
		Motor_GPIO_t motor_gpio;
		Motor_PWM_t  motor_pwm;
	};
	Motopot_motor_type motor_type;
    Motopot_type type;
	Motopot_status status;
	Motopot_direction direction;
	volatile uint16_t current_position;
	volatile uint16_t desired_position;
	volatile uint16_t last_position;
	volatile uint16_t temp_position;
	volatile int16_t current_distance;
	uint16_t total_distance;
	uint16_t min;
	uint16_t max;
	uint16_t tolerance;
	uint16_t temp; // for store last temporary position
	uint16_t minSpeed;
	uint16_t maxSpeed;
	int32_t  current_speed;
	struct ramp_s ramp;
	_Bool move;
	volatile int32_t active_timer;
	volatile int32_t idle_timer;
	volatile int32_t ramp_timer;
	uint8_t saved;
	uint8_t to_send;
	int16_t last_sent_value;

} MotopotOld_t;


typedef enum {

	MOTOPOT_EVENT_IDLE       = 0,
	MOTOPOT_EVENT_START      = 1,
	MOTOPOT_EVENT_STOP       = 2,
	MOTOPOT_EVENT_MOVING_BY_MOTOR  = 3,
	MOTOPOT_EVENT_MOVING_BY_HAND   = 4,
	MOTOPOT_EVENT_FORCE_STOP       = 5,

}Motopot2_event;;

typedef enum {
	MOTOPOT_STATE_IDLE       = 0,
	MOTOPOT_STATE_START      = 1,
	MOTOPOT_STATE_STOP       = 2,
	MOTOPOT_MOVING_BY_MOTOR  = 3,
	MOTOPOT_MOVING_BY_HAND   = 4,
	MOTOPOT_FORCE_STOP       = 5,
}Motopot2_state;

typedef enum {
	MOTOPOT_RAMP_STATE_IDLE   = 0,
	MOTOPOT_RAMP_STATE_ACCEL  = 1,
	MOTOPOT_RAMP_STATE_CONST  = 2,
	MOTOPOT_RAMP_STATE_DECEL  = 3,
	MOTOPOT_RAMP_STATE_FINISH = 4
}Motopot2_ramp_state;

typedef struct {
	Motopot2_ramp_state state;
	Timer_t timer;
	uint16_t target_speed;
	uint16_t accel_time;
	uint16_t decel_time;
	uint32_t init_speed;
	int16_t full_position;
	int16_t stop_position;
}Motopot2_ramp;

typedef struct{

	Pot_t pot;
	Motor_t motor;


	uint32_t id;
	char*    name;

	Motopot2_state state;

	uint32_t tolerance;

	uint8_t calibrated_flag;

	Timer_t timer;

	uint32_t speed;
	uint32_t temp_position;

	Timer_t  speed_timer;


	Timer_t  speed_timer_2;
	uint32_t speed_2;
	uint32_t speed_array_2[5];
	uint32_t temp_position_2;
	Motopot2_ramp ramp;

	 void (*Event_Idle_Callback)(void*);
	 uint8_t Event_Idle_Callback_Flag;

	 void (*Event_Start_Callback)(void*);
	 uint8_t Event_Start_Callback_Flag;

	 void (*Event_Stop_Callback)(void*);
	 uint8_t Event_Stop_Callback_Flag;

	 void (*Event_MovingByMotor_Callback)(void*);
	 uint8_t Event_MovingByMotor_Callback_Flag;

	 void (*Event_MovingByHand_Callback)(void*);
	 uint8_t Event_MovingByHand_Callback_Flag;

	 void (*Event_ForceStop_Callback)(void*);
	 uint8_t Event_ForceStop_Callback_Flag;

} Motopot2_t;

uint32_t readAnalog(ADC_HandleTypeDef* hadc, uint32_t channel);

//uint32_t Analog_BufferRead(uint32_t index);

//void Motopot2_Init(Motopot2_t* motopot, char* name, Pot_type type, ADC_HandleTypeDef* hadc, uint32_t channel, TIM_HandleTypeDef* htim_A, uint32_t channel_A, TIM_HandleTypeDef* htim_B, uint32_t channel_B);
void Motopot2_Init(Pot_t* pot, Motor_t* motor);

uint32_t Motopot2_Service(Motopot2_t* pot);
void Motopot2_MoveTo(Motopot2_t* motopot, uint32_t position);
void Motopot2_MoveTime(Motopot2_t* motopot, int32_t speed, uint32_t time);
void Motopot2_Stop(Motopot2_t* motopot);
void Motopot2_Calibration(Motopot2_t* motopot[], uint32_t count);
void Motopot2_CallbackRegister(Motopot2_t* motopot, void (*CallbackPtr)(void*), Motopot2_event event);

Motopot2_state Motopot2_GetState(Motopot2_t* motopot);

//void setPotPosition(MotopotOld_t*, int);
//void updatePotPosition(MotopotOld_t*);
//
//void createLinearRamp(MotopotOld_t*);
//void createLogRamp(MotopotOld_t*);
//
//void potMoveCW(MotopotOld_t* pot);
//void potMoveCCW(MotopotOld_t* pot);
//void potStopp(MotopotOld_t* pot);
//
//void pot_ramp_init(MotopotOld_t* p);
//void pot_ramp_process(MotopotOld_t* p);
//void linearRampProcess(MotopotOld_t* p);
//void logarytmicRampProcess(MotopotOld_t* p);
//void potGoingUp(MotopotOld_t* p);
//void potGoingDown(MotopotOld_t* p);
//void testPot(MotopotOld_t* p);


uint16_t valToPos(uint16_t);
uint16_t posToVal(uint16_t);
uint32_t getRealDistance(uint32_t currenValue, uint32_t desiredValue);

//bool isMoveByHand(Pot *pot);
//bool pot_update_position(Pot *pot);
//bool pot_update_position2(Pot *pot, uint16_t *cur_pos);
//void potReset(Pot *pot);
//void potStop(Pot *pot);
//void potCalibrate(Pot *pot);
//
//bool isEqual(uint16_t current_value, uint16_t expected_value, uint16_t tolerance);


#endif /* INC_MOTOPOT_H_ */
