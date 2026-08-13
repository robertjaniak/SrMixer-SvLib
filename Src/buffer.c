

#include "buffer.h"
#include "string.h"

void Buffer_init(Buffer_t *buffer, uint8_t* data, size_t size, bufferType type) {

	buffer->data = data;
	buffer->type = type;
	buffer->size = size;

	buffer->elements = 0;
    buffer->head = 0;
    buffer->tail = 0;

	for (size_t i = 0; i < buffer->size; i++){
		buffer->data[i] = 0;
	}

}

//=====================================
// WRITE
//=====================================

int8_t write(Buffer_t *buffer, uint8_t data){

	uint32_t next;

	switch (buffer->type) {
		case BUFFER_TYPE_CIRCLE:
			next = (buffer->head + 1) % buffer->size;
			if( next == buffer->tail){ // No space for new data, buffer full
				return -1;
			}
			else { //write data to buffer
				buffer->data[buffer->head] = data;
				buffer->head = next;
				buffer->elements++;
			}
			break;

		case BUFFER_TYPE_STANDARD:
			next = (buffer->head + 1) % buffer->size;
			buffer->data[buffer->head] = data;
			buffer->head = next;
			buffer->elements++;
			break;

		default:
			break;
	}

	return 0;
}

void Buffer_write_u8(Buffer_t *buffer, uint8_t data){

	write(buffer, data);

}

void Buffer_write_u16(Buffer_t *buffer, uint16_t data){

	write(buffer, ((data >> 0) & 0xFF));
	write(buffer, ((data >> 8) & 0xFF));

}

void Buffer_write_u32(Buffer_t *buffer, uint32_t data){

	write(buffer, ((data >>  0) & 0xFF));
	write(buffer, ((data >>  8) & 0xFF));
	write(buffer, ((data >> 16) & 0xFF));
	write(buffer, ((data >> 24) & 0xFF));

}

void Buffer_write_data_u8(Buffer_t *buffer, uint8_t *data, size_t size){

	for(int i = 0; i < size; i++){
		write(buffer, data[i]);
	}
}

void Buffer_write_data_u16(Buffer_t *buffer, uint16_t *data, size_t size){

	for(int i = 0; i < size; i++){
		write(buffer, ((data[i] >> 0) & 0xFF));
		write(buffer, ((data[i] >> 8) & 0xFF));
	}
}

void Buffer_write_data_u32(Buffer_t *buffer, uint32_t *data, size_t size){

	for(int i = 0; i < size; i++){
		write(buffer, ((data[i] >>  0) & 0xFF));
		write(buffer, ((data[i] >>  8) & 0xFF));
		write(buffer, ((data[i] >> 16) & 0xFF));
		write(buffer, ((data[i] >> 24) & 0xFF));
	}
}

void Buffer_copy_data_u8(Buffer_t *buffer, uint8_t *data, size_t size){

	if (size <= buffer->size){
		memcpy(buffer->data, data, size);
	}

}
void Buffer_copy_data_u16(Buffer_t *buffer, uint16_t *data, size_t size){

	size_t newSize = size * 2;

	if (newSize <= buffer->size){
		pack_16b(buffer->data, data, size);
	}
}
void Buffer_copy_data_u32(Buffer_t *buffer, uint32_t *data, size_t size){

	size_t newSize = size * 4;

	if (newSize <= buffer->size){
		pack_32b(buffer->data, data, size);
	}
}


//=====================================
// READ
//=====================================

uint8_t read(Buffer_t *buffer){

	return buffer->data[buffer->head];

}

uint8_t Buffer_read_u8(Buffer_t *buffer){

	return read(buffer);

}

uint16_t Buffer_read_u16(Buffer_t *buffer){

	return 0;

}

uint32_t Buffer_read_u32(Buffer_t *buffer){

	return 0;

}

int8_t readData(Buffer_t *buffer, uint8_t *data){

	uint32_t next;

	if( buffer->tail == buffer->head){ // No data to read, buffer empty
			return -1;
		}
		else {// read data from buffer
			next = (buffer->tail + 1) % buffer->size;
			*data = buffer->data[buffer->tail];
			buffer->tail = next;
			buffer->elements--;
		}
		return 0;
}

void Buffer_read_data_u8(Buffer_t *buffer, uint8_t *data, size_t size){
	readData(buffer, data);
}

void Buffer_read_data_u16(Buffer_t *buffer, uint16_t *data, size_t size){
	//readData(buffer, data);
}

void Buffer_read_data_u32(Buffer_t *buffer, uint32_t *data, size_t size){
	//readData(buffer, data);
}

//=====================================
// UTILITY
//=====================================

void Buffer_setLocation(Buffer_t *buffer, uint32_t location){

	if (location > buffer->size) location = buffer->size;
	buffer->head = location;

}

void Buffer_erase(Buffer_t *buffer){

	for (int32_t i = 0; i < buffer->size; i++){
		buffer->data[i] = 0;
	}

	buffer->head = 0;
	buffer->tail = 0;
	buffer->elements = 0;
}


void Buffer_fill(Buffer_t *buffer, uint8_t value){

	for (int32_t i = 0; i < buffer->size; i++){
		buffer->data[i] = value;
	}

	buffer->head = 0;
	buffer->tail = 0;
	buffer->elements = 0;
}

void Buffer_clear(Buffer_t *buffer){

	buffer->head = 0;
	buffer->tail = 0;
	buffer->elements = 0;

}

int8_t Buffer_isEmpty(Buffer_t *buffer){

	return buffer->tail == buffer->head;

}
int8_t Buffer_isFull(Buffer_t *buffer){

	return buffer->tail == (buffer->head + 1) % buffer->size;
}

//=====================================
// PACKING/UNPACKING DATA
//=====================================

void pack_u1(uint8_t *buffer, uint32_t *packet_index, const uint8_t data, uint8_t *byte_index){

	uint8_t value = buffer[*packet_index];

	if (data == 0){
		buffer[*packet_index] = clearBit(value, *byte_index);
	}
	else{
		buffer[*packet_index] = setBit(value, *byte_index);
	}

	if (*byte_index == 8){
		*byte_index = 1;
		*packet_index += 1;
	}
	else {
		*byte_index += 1;
	}
}

void pack_u8(uint8_t *buffer, uint32_t *packet_count, const uint8_t data_u8)
{
    buffer[(*packet_count) + 0] = ((uint8_t*)&data_u8)[0];

    *packet_count+= sizeof(data_u8);
}

void pack_u16(uint8_t *buffer, uint32_t *packet_count, const uint16_t data_u16)
{
    buffer[(*packet_count) + 0] = ((uint8_t*)&data_u16)[0];
    buffer[(*packet_count) + 1] = ((uint8_t*)&data_u16)[1];

    *packet_count+= 2;
}

void pack_u32(uint8_t *buffer, uint32_t *packet_count, const uint32_t data_u32)
{
    buffer[(*packet_count) + 0] = ((uint8_t*)&data_u32)[0];
    buffer[(*packet_count) + 1] = ((uint8_t*)&data_u32)[1];
    buffer[(*packet_count) + 2] = ((uint8_t*)&data_u32)[2];
    buffer[(*packet_count) + 3] = ((uint8_t*)&data_u32)[3];

    *packet_count+= 4;
}

void pack_float( uint8_t *buffer, uint32_t *packet_count, const float data_float)
{
    buffer[(*packet_count) + 0] = ((uint8_t*)&data_float)[0];
    buffer[(*packet_count) + 1] = ((uint8_t*)&data_float)[1];
    buffer[(*packet_count) + 2] = ((uint8_t*)&data_float)[2];
    buffer[(*packet_count) + 3] = ((uint8_t*)&data_float)[3];

    *packet_count+= sizeof(data_float);
}

void unpack_u16( const uint8_t *buffer, uint32_t *packet_count,  uint16_t *data_u16)
{
    ((uint8_t*)data_u16)[0] = buffer[(*packet_count) + 0];
    ((uint8_t*)data_u16)[1] = buffer[(*packet_count) + 1];

    *packet_count+= 2;
}

void unpack_u32(const uint8_t *buffer, uint32_t *packet_count, uint32_t *data_u32)
{
    ((uint8_t*)data_u32)[0] = buffer[(*packet_count) + 0];
    ((uint8_t*)data_u32)[1] = buffer[(*packet_count) + 1];
    ((uint8_t*)data_u32)[2] = buffer[(*packet_count) + 2];
    ((uint8_t*)data_u32)[3] = buffer[(*packet_count) + 3];

    *packet_count += 4;
}

void unpack_float( const uint8_t *buffer, uint32_t *packet_count, float *data_float)
{
    ((uint8_t*)data_float)[0] = buffer[(*packet_count) + 0];
    ((uint8_t*)data_float)[1] = buffer[(*packet_count) + 1];
    ((uint8_t*)data_float)[2] = buffer[(*packet_count) + 2];
    ((uint8_t*)data_float)[3] = buffer[(*packet_count) + 3];

    *packet_count+= sizeof(data_float);
}

void pack_1b(uint8_t* dest, uint8_t* src, size_t size){

	uint32_t data_count = 0;
	uint8_t byte_count = 0;

	for (int i = 0; i < size ; i++){
		pack_u1(dest, &data_count, src[i], &byte_count);
	}
}

void pack_8b(uint8_t* dest, uint8_t* src, size_t size){
	memcpy(dest, src, size);
}

void pack_16b(uint8_t* dest, uint16_t* src, size_t size){

	uint32_t count = 0;

	for (int i = 0; i < size ; i++){
		pack_u16(dest, &count, src[i]);
	}

}
void pack_32b(uint8_t* dest, uint32_t* src, size_t size){

	uint32_t count = 0;

	for (int i = 0; i < size ; i++){
		pack_u32(dest, &count, src[i]);
	}

}

void unpack_8b(uint8_t* dest, uint8_t* src, size_t size){

}

void unpack_16b(uint8_t* src, uint16_t* dest, size_t size /* endianness */){

	//uint32_t count = 0;

	for (int i = 0; i < size ; i++){
		((uint8_t*)dest)[i + 0] = src[i + 0];
		((uint8_t*)dest)[i + 1] = src[i + 1];
	}
}


void unpack_32b(uint8_t* dest, uint32_t* src, size_t size){

}

void swap_8b(uint16_t* dest, uint8_t* src, size_t size){

}

void swap_16b(uint16_t* dest, uint16_t* src, size_t size){

}
void swap_32b(uint32_t* dest, uint32_t* src, size_t size){

}



