/*
 * reg.c
 *
 *  Created on: 10 mar 2023
 *      Author: rober
 */

#include <reg.h>

void Reg_Init(Reg_t *reg){
	reg->to_update = 1;
}


void sendSPIdata(Reg_t *reg){

	HAL_GPIO_WritePin(reg->latch.port, reg->latch.pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(reg->hspi, reg->data, reg->size, 10);
	HAL_GPIO_WritePin(reg->latch.port, reg->latch.pin, GPIO_PIN_SET);

	reg->to_update = 0;
}


void clearAllRegisters(Reg_t *reg){

	for (int i = 0; i < reg->size; i++){
		*(reg->data + i) = 0x00;
	}
	sendSPIdata(reg);
}

void fullAllRegisters(Reg_t *reg){

	for (int i = 0; i < reg->size; i++){
		*(reg->data + i) = 0xFF;
	}

	sendSPIdata(reg);
}

uint8_t needsUpdate(Reg_t *reg){
	return reg->to_update;
}

void setRegBit(Reg_t *reg, uint8_t bit){

	reg->data[bit / 8] |= 0x01 << (bit % 8);

	reg->to_update = 1;

}

void resetRegBit(Reg_t *reg, uint8_t bit){

	reg->data[bit / 8] &= ~(0x01 << (bit % 8));

	reg->to_update = 1;

}
