/*
 * signal_proc_util.c
 *
 *  Created on: Mar 11, 2019
 *      Author: Joshua B. Russell
 */

#include "signal_proc_util.h"

bool all_above_threshold(const uint16_t* arr,const uint16_t arr_size){
	for(int i = 0; i < arr_size; i++){
		if(*(arr+i) < (STIM_TRIGGER_THRESHOLD + STIM_TRIGGER_TOLERANCE))
		{
            return false;
		}
	}
	return true;
}

bool all_below_threshold(const uint16_t* arr,const uint16_t arr_size){
	for(int i = 0; i < arr_size; i++){
		if(*(arr+i) > (STIM_TRIGGER_THRESHOLD - STIM_TRIGGER_TOLERANCE)){
            return false;
		}
	}
	return true;
}
