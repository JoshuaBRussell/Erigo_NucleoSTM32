/*
 * signal_proc_util.c
 *
 *  Created on: Mar 11, 2019
 *      Author: Joshua B. Russell
 */

#include "signal_proc_util.h"


/* Checks if all values in an array are ABOVE a threshold value
 * arr - array in which values are stored
 * arr_size - size of the array
 * threshold - threshold value
 * ret: true if all are above the threshold, false if not
 */
bool all_above_threshold(const uint32_t* arr,const uint32_t arr_size, const uint16_t threshold){
	for(int i = 0; i < arr_size; i++){
		if(*(arr+i) < (threshold))
		{
            return false;
		}
	}
	return true;
}

/* Checks if all values in an array are BELOW a threshold value
 * arr - array in which values are stored
 * arr_size - size of the array
 * threshold - threshold value
 * ret: true if all are above the threshold, false if not
 */
bool all_below_threshold(const uint32_t* arr,const uint32_t arr_size, const uint16_t threshold){
	for(int i = 0; i < arr_size; i++){
		if(*(arr+i) > (threshold)){
            return false;
		}
	}
	return true;
}
