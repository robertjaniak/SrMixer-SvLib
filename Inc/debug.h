/*
 * debug_defines.h
 *
 *  Created on: Jul 7, 2022
 *      Author: rober
 */

#ifndef INC_DEBUG_DEF_H_
#define INC_DEBUG_DEF_H_

#include "usart.h"
#include "string.h"
#include "stdio.h"

#define DEBUG_MESSAGE_BUFFER_SIZE 64
char debugMessageBuffer[DEBUG_MESSAGE_BUFFER_SIZE];

//#define FIRMWARE_VERSION __DATE__
#define FIRMWARE_VERSION CURRENT_DATE
#define FIRMWARE_CONFIG CURRENT_CONFIG

#define PRINT_MSG(msg) HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
#define PRINT_STR(msg) PRINT_MSG(msg)

#define PRINT_MSG_STR(msg, str) { 			\
  sprintf(debug_msg_buffer, msg, str);		\
  PRINT_MSG(debug_msg_buffer)						\
}

#define PRINT_MSG_INT(msg, val) { 			\
  sprintf(debug_msg_buffer, msg, val);		\
  PRINT_MSG(debug_msg_buffer)						\
}

#define VALUE_MSG "value: %ld \r\n"
#define PRINT_VALUE(value) { 				\
  sprintf(debug_msg_buffer, VALUE_MSG, (uint32_t)value);		\
  PRINT_MSG(debug_msg_buffer)						\
}

#define ERROR_MSG "ERROR !!! : %d \r\n"
#define PRINT_ERROR(error) { 				\
  sprintf(debug_msg_buffer, ERROR_MSG, error);		\
  PRINT_MSG(debug_msg_buffer)						\
}

#ifdef DEBUG
	#define DBG_PRINT(msg)    PRINT_MSG(msg);
	#define DBG_PRINT_STR(msg, str) PRINT_MSG_STR(msg, str)
	#define DBG_PRINT_INT(msg, val) PRINT_MSG_INT(msg, val)
	#define DBG_VALUE(value)  PRINT_VALUE(value);
	#define DBG_ERROR(error)  PRINT_ERROR(error);
#else
	#define DBG_PRINT(msg)
	#define DBG_PRINT_STR(msg, str)
	#define DBG_PRINT_INT(msg, val)
	#define DBG_VALUE(value)
	#define DBG_ERROR(error)
#endif

typedef struct {
	char* name;
	uint32_t last;
	uint32_t now;
	uint32_t period;
} WorkingTime_t;

void workingTime_Init(WorkingTime_t* wt, char* name);
void workingTime_Start(WorkingTime_t* wt);
void workingTime_Print(WorkingTime_t* wt);

#endif /* INC_DEBUG_DEF_H_ */
