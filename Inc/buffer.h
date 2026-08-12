/*
 * buffer.h
 *
 *  Created on: 1 gru 2022
 *      Author: rober
 */

#ifndef SVLIB_INC_BUFFER_H_
#define SVLIB_INC_BUFFER_H_

#include "global.h"

typedef enum{
	BUFFER_TYPE_STANDARD,
	BUFFER_TYPE_CIRCLE,
} bufferType;

typedef struct {
	uint8_t   *data;
	bufferType type;
	size_t    size;
	int32_t   head;
	int32_t   tail;
	int32_t   elements;
} Buffer_t;

void Buffer_init(Buffer_t *buffer, uint8_t* data, size_t size, bufferType type);
void Buffer_setLocation(Buffer_t *buffer, uint32_t location); // index from 1

void Buffer_write_u8(Buffer_t *buffer, uint8_t data);
void Buffer_write_u16(Buffer_t *buffer, uint16_t data);
void Buffer_write_u32(Buffer_t *buffer, uint32_t data);

void Buffer_write_data_u8(Buffer_t *buffer, uint8_t *data, size_t size);
void Buffer_write_data_u16(Buffer_t *buffer, uint16_t *data, size_t size);
void Buffer_write_data_u32(Buffer_t *buffer, uint32_t *data, size_t size);

void Buffer_copy_data_u8(Buffer_t *buffer, uint8_t *data, size_t size);
void Buffer_copy_data_u16(Buffer_t *buffer, uint16_t *data, size_t size);
void Buffer_copy_data_u32(Buffer_t *buffer, uint32_t *data, size_t size);

uint8_t Buffer_read_u8(Buffer_t *buffer);
uint16_t Buffer_read_u16(Buffer_t *buffer);
uint32_t Buffer_read_u32(Buffer_t *buffer);

void Buffer_read_data_u8(Buffer_t *buffer, uint8_t *data, size_t size);
void Buffer_read_data_u16(Buffer_t *buffer, uint16_t *data, size_t size);
void Buffer_read_data_u32(Buffer_t *buffer, uint32_t *data, size_t size);

void Buffer_erase(Buffer_t *buffer);
void Buffer_clear(Buffer_t *buffer);
void Buffer_fill(Buffer_t *buffer, uint8_t value);

int8_t Buffer_isEmpty(Buffer_t *buffer);
int8_t Buffer_isFull(Buffer_t *buffer);

//=====================================
void pack_8b(uint8_t* dest, uint8_t* src, size_t size);
void pack_16b(uint8_t* dest, uint16_t* src, size_t size);
void pack_32b(uint8_t* dest, uint32_t* src, size_t size);

void unpack_8b(uint8_t* dest, uint8_t* src, size_t size);
void unpack_16b(uint8_t* dest, uint16_t* src, size_t size);
void unpack_32b(uint8_t* dest, uint32_t* src, size_t size);

void swap_8b(uint16_t* dest, uint8_t* src, size_t size);
void swap_16b(uint16_t* dest, uint16_t* src, size_t size);
void swap_32b(uint32_t* dest, uint32_t* src, size_t size);

//=====================================

void pack_u1(uint8_t *buffer, uint32_t *packet_count, const uint8_t data_u8, uint8_t *byte_index);
void pack_u8(uint8_t *buffer, uint32_t *packet_count, const uint8_t data_u8);
void pack_u16(uint8_t *buffer, uint32_t *packet_count, const uint16_t data_u16);
void pack_u32(uint8_t *buffer, uint32_t *packet_count, const uint32_t data_u32);
void pack_float( uint8_t *buffer, uint32_t *packet_count, const float data_float);
void unpack_u16( const uint8_t *buffer, uint32_t *packet_count,  uint16_t *data_u16);
void unpack_u32(const uint8_t *buffer, uint32_t *packet_count, uint32_t *data_u32);
void unpack_float( const uint8_t *buffer, uint32_t *packet_count, float *data_float);

#endif /* SVLIB_INC_BUFFER_H_ */
