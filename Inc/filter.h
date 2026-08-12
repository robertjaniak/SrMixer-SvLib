/*
 * filter.h
 *
 *  Created on: Aug 27, 2023
 *      Author: rober
 */

#ifndef SVLIB_INC_FILTER_H_
#define SVLIB_INC_FILTER_H_

#include "global.h"

typedef enum {
	FILTER_STATUS_COLLECT_DATA,
	FILTER_STATUS_NORMAL,
	FILTER_STATUS_PRESENT_VALUE,
}Filter_status;;

typedef struct{
	uint32_t* data;
	uint32_t size;
	uint32_t count;
	uint32_t sum;
	Filter_status status;

} Filter_t;

void Filter_Init(Filter_t* sma, uint32_t* data, uint32_t size);
int32_t Filter_SMA(Filter_t* sma, int32_t value);

void Filter_Average_Init(Filter_t* filter, uint32_t* data, uint32_t size);
int32_t Filter_Average(Filter_t* filter, int32_t value);

#endif /* SVLIB_INC_FILTER_H_ */
