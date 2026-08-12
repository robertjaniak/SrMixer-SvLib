/*
 * potentiometer.c
 *
 *  Created on: Mar 14, 2021
 *      Author: Dom
 */


#include <motopot.h>

#define MOTOPOT_TOLERANCE 0

#define KNEE_VALUE 74 //64
#define KNEE_POSITION  185 //166

#define MOTOPOT_DETECT_SPEED_TIME 65


static void PotSpeedDetectElapsedCallback(void* handle){

	Motopot_t* motopot = (Motopot_t*)handle;

	int32_t dist = Pot_getCurrentPosition(motopot->pot) - motopot->temp_position;
	uint32_t time = Timer_GetTime(&motopot->speed_timer);

	if (time > 0){
		motopot->speed = (motopot->speed + ((dist * 1000) / time)) / 2;
		motopot->temp_position = Pot_getCurrentPosition(motopot->pot);
	}
}



static void Motopot_Start(Motopot_t* motopot){

	motopot->state = MOTOPOT_STATE_START;
	Motor_Start(motopot->motor);
}

void Motopot_Stop(Motopot_t* motopot){

	motopot->state = MOTOPOT_STATE_STOP;
	Motor_Stop(motopot->motor);
}

static void  MotopotStateDetect(Motopot_t* motopot){

	Pot_state pot_state = Pot_getState(motopot->pot);
	Motor_state motor_state = Motor_GetState(motopot->motor);

	if (pot_state == POT_STATE_IDLE && motor_state == MOTOR_STATE_STOP){
		motopot->state = MOTOPOT_STATE_IDLE;
	}

	if (pot_state == POT_STATE_IDLE && motor_state != MOTOR_STATE_STOP){

		if (motopot->state == MOTOPOT_STATE_IDLE){
			motopot->state = MOTOPOT_STATE_START;
		}

		if (motopot->state == MOTOPOT_MOVING_BY_MOTOR){
			motopot->state = MOTOPOT_FORCE_STOP;
		}
	}

	if (pot_state != POT_STATE_IDLE && motor_state != MOTOR_STATE_STOP ){
		motopot->state = MOTOPOT_MOVING_BY_MOTOR;
	}

	if (pot_state != POT_STATE_IDLE && motor_state == MOTOR_STATE_STOP ){
		motopot->state = MOTOPOT_MOVING_BY_HAND;
	}
}

void MotopotEventDetect(Motopot_t* motopot){

		switch (motopot->state){

		case MOTOPOT_STATE_IDLE:

			if (motopot->Event_Idle_Callback_Flag == 1){
				if (motopot->Event_Idle_Callback != NULL){
					(*motopot->Event_Idle_Callback)(motopot);
				}
			}

			break;

		case MOTOPOT_STATE_START:

			if (motopot->Event_Start_Callback_Flag == 1){
				if (motopot->Event_Start_Callback != NULL){
					(*motopot->Event_Start_Callback)(motopot);
				}
				motopot->Event_Start_Callback_Flag = 0;
			}

			break;

		case MOTOPOT_STATE_STOP:

			if (motopot->Event_Stop_Callback_Flag == 1){
				if (motopot->Event_Stop_Callback != NULL){
					(*motopot->Event_Stop_Callback)(motopot);
				}
				motopot->Event_Stop_Callback_Flag = 0;
			}
			break;

		case MOTOPOT_MOVING_BY_HAND:

			if (motopot->Event_MovingByMotor_Callback_Flag == 1){
				if (motopot->Event_MovingByMotor_Callback != NULL){
					(*motopot->Event_MovingByMotor_Callback)(motopot);
				}
			}
			break;

		case MOTOPOT_MOVING_BY_MOTOR:

			if (motopot->Event_MovingByHand_Callback_Flag == 1){
				if (motopot->Event_MovingByHand_Callback != NULL){
					(*motopot->Event_MovingByHand_Callback)(motopot);
				}
				motopot->Event_MovingByHand_Callback_Flag = 0;
			}
			break;

		case MOTOPOT_FORCE_STOP:

			if (motopot->Event_ForceStop_Callback_Flag == 1){
				if (motopot->Event_ForceStop_Callback != NULL){
					(*motopot->Event_ForceStop_Callback)(motopot);
				}
				motopot->Event_ForceStop_Callback_Flag = 0;
			}
			break;

		default:
			break;
	}
}

static void MotopotStandardConstService(Motopot_t* motopot){

	int32_t distance = motopot->pot->desiredPosition - motopot->pot->currentPosition;

	switch (Motor_GetState(motopot->motor)){
		case MOTOR_STATE_STOP:
			break;

		case MOTOR_STATE_MOVING_RIGHT:

			if (distance <= motopot->tolerance / (motopot->motor->maxSpeed / motopot->motor->speed)){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_FINISH;
				Motor_SetSpeed(motopot->motor, motopot->motor->minSpeed);
			}
			break;

		case MOTOR_STATE_MOVING_LEFT:

			if (distance >= -(motopot->tolerance / (motopot->motor->maxSpeed / motopot->motor->speed))){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_FINISH;
				Motor_SetSpeed(motopot->motor, motopot->motor->minSpeed);
			}
			break;

		default:
			break;
	}
}

static void MotopotRampConstService(Motopot_t* motopot){

	int32_t distance = motopot->pot->desiredPosition - motopot->pot->currentPosition;

	switch (Motor_GetState(motopot->motor)){
		case MOTOR_STATE_STOP:
			break;

		case MOTOR_STATE_MOVING_RIGHT:

			if (distance <= 4 * (motopot->tolerance / (motopot->motor->maxSpeed / motopot->motor->speed))){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_DECEL;
				//Motor_SoftStop(motopot->motor);
			}
			break;

		case MOTOR_STATE_MOVING_LEFT:

			if (distance >= -4 * (motopot->tolerance / (motopot->motor->maxSpeed / motopot->motor->speed))){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_DECEL;
				//Motor_SoftStop(motopot->motor);
			}
			break;

		default:
			break;
	}
}

static void MotopotFinishService(Motopot_t* motopot){


	switch (Motor_GetState(motopot->motor)){
		case MOTOR_STATE_STOP:
			break;

		case MOTOR_STATE_MOVING_RIGHT:
			if (motopot->pot->currentPosition >= motopot->pot->desiredPosition){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_IDLE;
				motopot->motor->ramp.state = MOTOR_RAMP_STOP;
				Motopot_Stop(motopot);
			}
			break;

		case MOTOR_STATE_MOVING_LEFT:
			if (motopot->pot->currentPosition <= motopot->pot->desiredPosition){
				motopot->ramp.state = MOTOPOT_RAMP_STATE_IDLE;
				motopot->motor->ramp.state = MOTOR_RAMP_STOP;
				Motopot_Stop(motopot);
			}
			break;

		default:
			break;
	}

}

static void MotopotMovingByMotorService(Motopot_t* motopot){

	TimerService(&motopot->ramp.timer);
//	int32_t distance = motopot->pot.desired_position - motopot->pot.current_position;
//	uint32_t time_to_stop = distance / motopot->pot.speed;


	switch(motopot->ramp.state){

		case MOTOPOT_RAMP_STATE_IDLE:
			break;

		case MOTOPOT_RAMP_STATE_ACCEL:

//			if (Motor_GetRampState(motopot->motor) == MOTOR_RAMP_CONST){
//				motopot->ramp.state = MOTOPOT_RAMP_STATE_CONST;
//			}

//			if (Motor_GetRampState(motopot->motor) == MOTOR_RAMP_DECEL){
//				motopot->ramp.state = MOTOPOT_RAMP_STATE_DECEL;
//			}

//			if(time_to_stop <= motopot->ramp.decel_time){
//				Motopot_SoftStopInit(motopot);
//			}


			break;

		case MOTOPOT_RAMP_STATE_CONST:

//			if(time_to_stop <= motopot->ramp.decel_time){
//				motopot->ramp.state = MOTOPOT_RAMP_STATE_DECEL;
//				Motopot_SoftStopInit(motopot);
//			}

			if (motopot->motor->mode == MOTOR_MODE_RAMP){
				MotopotRampConstService(motopot);
			}

			if (motopot->motor->mode == MOTOR_MODE_NORMAL){
				MotopotStandardConstService(motopot);
			}


			break;

		case MOTOPOT_RAMP_STATE_DECEL:

//			if (Motor_GetRampState(motopot->motor) == MOTOR_RAMP_FINISH){
//				//motopot->ramp.state = MOTOPOT_RAMP_STATE_FINISH;
//			}

			break;

		case MOTOPOT_RAMP_STATE_FINISH:
			MotopotFinishService(motopot);
			break;

		default:
			break;
	}
}





static void startInit(Motopot_t* motopot){

	int32_t distance;

	distance = Pot_getTotalDistance(motopot->pot);

	if (motopot->motor->mode == MOTOR_MODE_NORMAL){
		motopot->ramp.state = MOTOPOT_RAMP_STATE_CONST;

		if (distance > MOTOPOT_TOLERANCE){
			Motor_Move(motopot->motor, motopot->motor->maxSpeed);
		}
		else if (distance < - MOTOPOT_TOLERANCE){
			Motor_Move(motopot->motor, -motopot->motor->maxSpeed);
		}
	}

	if (motopot->motor->mode == MOTOR_MODE_RAMP){
		motopot->ramp.state = MOTOPOT_RAMP_STATE_ACCEL;

		if (distance > MOTOPOT_TOLERANCE){
			//(motopot->motor, motopot->motor->maxSpeed, MOTOR_DIR_RIGHT);
		}
		else if (distance < -MOTOPOT_TOLERANCE){
			//Motor_SoftStartInit(motopot->motor, motopot->motor->maxSpeed, MOTOR_DIR_LEFT);
		}
	}
}

void Motopot_MoveTo(Motopot_t* motopot, uint16_t position){

	Pot_setDesiredPosition(motopot->pot, position);

	motopot->moveType = MOTOPOT_MOVE_ONE_SHOT;

	if(Pot_getCurrentPosition(motopot->pot) < position) {
		Motor_Move(motopot->motor, motopot->motor->minSpeed);
	}
	else if (Pot_getCurrentPosition(motopot->pot) > position){
		Motor_Move(motopot->motor, -motopot->motor->minSpeed);
	}
	else{
		Motor_Stop(motopot->motor);
	}
}


void Motopot_setPosition(Motopot_t* motopot, uint16_t position){

	Pot_setDesiredPosition(motopot->pot, position);

	startInit(motopot);

//	if (motopot->motor.mode == MOTOR_MODE_STANDARD){
//		Motopot_StandardInit(motopot);
//	}
//
//	if (motopot->motor.mode == MOTOR_MODE_RAMP){
//		Motopot_SoftStartInit(motopot);
//	}

}

void Motopot_setPositionToCenter(Motopot_t* motopot){

	Motopot_setPosition(motopot, motopot->pot->maxValue/2);

}

void Motopot_serviceEnable(Motopot_t* motopot, uint8_t enable){

	if (enable){
		motopot->serviceEnable = 1;
	}
	else {
		motopot->serviceEnable = 0;
	}

}

static void MotopotTimerElapsedCallback(void* handle){

	Motopot_t* motopot = (Motopot_t*)handle;

	TimerStop(&motopot->timer);
	Motor_Stop(motopot->motor);

	motopot->state = MOTOPOT_STATE_STOP;

}

void Motopot_Init(Motopot_t* motopot, Pot_t* pot, Motor_t* motor){

	motopot->pot = pot;
	motopot->motor = motor;

	motopot->serviceEnable = 1;

	motopot->tolerance = MOTOPOT_TOLERANCE;

	//PID_Init(pid, _kp, _ki, _kd, _Ts, _fc, _maxOutput);

	motopot->moveType = MOTOPOT_MOVE_CONTINOUS;

	PID_Init(&motopot->pid, 24, 0, 0, 0.001, 0, motor->maxSpeed);

//	    {
//	        6,     // Kp: proporional gain
//	        2,     // Ki: integral gain
//	        0.035, // Kd: derivative gain
//	        Ts,    // Ts: sampling time
//	        60, // fc: cutoff frequency of derivative filter (Hz), zero to disable
//
//	    // This one has more overshoot, but less ramp tracking error
//	    {
//	        4,     // Kp: proportional gain
//	        11,    // Ki: integral gain
//	        0.028, // Kd: derivative gain
//	        Ts,    // Ts: sampling time
//	        40, // fc: cutoff frequency of derivative filter (Hz), zero to disable
//	    },
//	    // This is a very aggressive controller
//	    {
//	        8.55,  // Kp: proportional gain
//	        440,   // Ki: integral gain
//	        0.043, // Kd: derivative gain
//	        Ts,    // Ts: sampling time
//	        70, // fc: cutoff frequency of derivative filter (Hz), zero to disable
//	    },
//	    // Fourth controller
//	    {
//	        6,     // Kp: proportional gain
//	        2,     // Ki: integral gain
//	        0.035, // Kd: derivative gain
//	        Ts,    // Ts: sampling time
//	        60, // fc: cutoff frequency of derivative filter (Hz), zero to disable
//	    },

	Timer_Init(motopot, &motopot->timer, 1000, TIMER_MODE_ONE_SHOT);
	TimerRegisterCallback(&motopot->timer, MotopotTimerElapsedCallback, TIMER_EVENT_ELAPSED);

	Timer_Init(motopot, &motopot->speed_timer, MOTOPOT_DETECT_SPEED_TIME, TIMER_MODE_CONTINOUS);
	TimerRegisterCallback(&motopot->speed_timer, PotSpeedDetectElapsedCallback, TIMER_EVENT_ELAPSED);
	TimerStart(&motopot->speed_timer);

	motopot->Event_Idle_Callback = NULL;
	motopot->Event_Idle_Callback_Flag = 1;

	motopot->Event_Start_Callback = NULL;
	motopot->Event_Start_Callback_Flag = 1;

	motopot->Event_Stop_Callback = NULL;
	motopot->Event_Stop_Callback_Flag = 1;

	motopot->Event_MovingByMotor_Callback = NULL;
	motopot->Event_MovingByMotor_Callback_Flag = 1;

	motopot->Event_MovingByHand_Callback = NULL;
	motopot->Event_MovingByHand_Callback_Flag = 1;

	motopot->Event_ForceStop_Callback = NULL;
	motopot->Event_ForceStop_Callback_Flag = 1;

};

static void simpleSpeedController(Motopot_t* motopot){

	if (motopot->pot->currentPosition < motopot->pot->desiredPosition - 1){
		Motor_SetSpeed(motopot->motor, motopot->motor->minSpeed);
	}

	else if (motopot->pot->currentPosition > motopot->pot->desiredPosition + 1){
		Motor_SetSpeed(motopot->motor, -motopot->motor->minSpeed);
	}
	else {
		Motor_SetSpeed(motopot->motor, 0);
	}

	updatePWMSpeed(motopot->motor);

}

static void PID_Controller(Motopot_t* motopot){

	int32_t speed;

	PID_setSetpoint(&motopot->pid, motopot->pot->desiredPosition);

	speed = PID_update(&motopot->pid, motopot->pot->currentPosition);

	if (speed > 0){

		//speed += 1000;
		if (speed < motopot->motor->minSpeed) speed = motopot->motor->minSpeed;

	}
	else if (speed < 0){

		//speed -= 1000;

		if (speed > - motopot->motor->minSpeed) speed = - motopot->motor->minSpeed;
	}

	Motor_SetSpeed(motopot->motor, speed);

	updatePWMSpeed(motopot->motor);
}

void Motopot_Service(Motopot_t* motopot){

	if (motopot->serviceEnable){
		//simpleSpeedController(motopot);
		PID_Controller(motopot);
	}

	if(motopot->pot->currentPosition == motopot->pot->desiredPosition) {
		if (motopot->moveType == MOTOPOT_MOVE_ONE_SHOT){
			Motor_Stop(motopot->motor);
			motopot->moveType = MOTOPOT_MOVE_CONTINOUS;
		}
	}

}


