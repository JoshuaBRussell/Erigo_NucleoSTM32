/** @file signal_proc_util.h
 *
 * @brief Simple/ Custom signal processing functions.
 *
 * @par
 *
 */

#ifndef SIG_PROC_UTIL_H
#define SIG_PROC_UTIL_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"

/* Defines -------------------------------------------------------------------*/


/* Function Prototypes -------------------------------------------------------*/
bool all_above_threshold(const uint32_t* arr, const uint32_t arr_size, const uint16_t threshold);
bool all_below_threshold(const uint32_t* arr, const uint32_t arr_size, const uint16_t threshold);

#endif
