#ifndef SIG_PROC_UTIL_H
#define SIG_PROC_UTIL_H

#include "stdint.h"
#include "stdbool.h"

#define STIM_TRIGGER_THRESHOLD 2000
#define STIM_TRIGGER_TOLERANCE 50
#define STIM_TRIGGER_CYCLE_LIMIT 5 //Number of times the threshold must be reached before Test Pulse is produced.


bool all_above_threshold(const uint16_t* arr,const uint16_t arr_size);
bool all_below_threshold(const uint16_t* arr,const uint16_t arr_size);

#endif
