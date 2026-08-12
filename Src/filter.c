/*
 * filter.c
 *
 *  Created on: Aug 27, 2023
 *      Author: rober
 */

#include "filter.h"


void Filter_Init(Filter_t* filter, uint32_t* data, uint32_t size){

	filter->data = data;
	filter->size = size;
	filter->count = 0;
	filter->sum = 0;
	filter->status = FILTER_STATUS_COLLECT_DATA;

	for (int i = 0; i < filter->size; i++){
		filter->data[i] = 0;
	}
}

int32_t Filter_SMA(Filter_t* sma, int32_t value) {

	uint32_t result = 0;

	switch (sma->status){

		case FILTER_STATUS_COLLECT_DATA:

			sma->data[sma->count] = value;
			sma->sum += value;
			result = sma->sum / (sma->count + 1);
			sma->count++;
			if (sma->count >= sma->size) {
				sma->count = 0;
				sma->status = FILTER_STATUS_NORMAL;
			}
			break;

		case FILTER_STATUS_NORMAL:

			sma->sum -= (uint32_t)sma->data[sma->count];
			sma->data[sma->count] = value;
			sma->sum += value;
			result = sma->sum / sma->size;
			sma->count++;
			if (sma->count >= sma->size) {
				sma->count = 0;
			}

			break;

		default:
			break;
	}

	return result;
}

int32_t Filter_Average(Filter_t* filter, int32_t value) {

	uint32_t result = -1;

	filter->data[filter->count] = value;
	filter->count++;

	if (filter->count == filter->size) {
		filter->count = 0;
		for (int i = 0; i < filter->size; i++){
			filter->sum += (uint32_t)filter->data[i];
			result = filter->sum / filter->size;
		}
		filter->sum = 0;
	}

	return result;
}
