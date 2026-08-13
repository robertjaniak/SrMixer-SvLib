/*
 * reg.h
 *
 *  Created on: 10 mar 2023
 *      Author: rober
 */

#ifndef SVLIB_INC_REG_H_
#define SVLIB_INC_REG_H_


#include "global.h"
#include "timer.h"



typedef struct {
	uint8_t id;
	uint8_t *data;
	uint16_t size;

#ifdef HAL_SPI_MODULE_ENABLED
	SPI_HandleTypeDef *hspi;
#endif /* HAL_SPI_MODULE_ENABLED */

	IOPin latch;
	uint8_t to_update;

} Reg_t;

void Reg_Init(Reg_t *reg);

void sendSPIdata(Reg_t *reg);

void clearAllRegisters(Reg_t *reg);
void fullAllRegisters(Reg_t *reg);

uint8_t needsUpdate(Reg_t *reg);

void setRegBit(Reg_t *reg, uint8_t bit);
void resetRegBit(Reg_t *reg, uint8_t bit);



#endif /* SVLIB_INC_REG_H_ */
