/** @file comm_protocol.h
 *
 * @brief Functions for implementing comm protocol. Uses serial to get raw bytes.
 *
 * @par
 *
 *  Created on: Mar 11, 2019
 *      Author: Joshua B. Russell
 */

#ifndef COMM_PROTOCOL_H_
#define COMM_PROTOCOL_H_

#include "stdbool.h"
#include "stdint.h"

bool is_comm_success();

void comm_get_control_params(uint16_t* val1, uint16_t* val2, uint16_t* val3);

/* Checks if specific 'markers' are in the correct location of the incoming message.
 *
 * buffer - buffer where incoming serial message was stored
 * buffer_size - size of the array
 * ret: true if message has correct 'markers'; false otherwise
 */
bool check_message_indicators(uint8_t* buffer, uint16_t buffer_size);

/* Parses buffer for data.
 *
 * buffer - buffer where incoming serial message was stored
 * buffer_size - size of the array
 * val1-3: Generic var names. Data can be anything.
 */
//"valx" so it is generic.
void parse_message(uint8_t* buffer, uint16_t buffer_size, uint16_t* const val1, uint16_t* const val2, uint16_t* const val3);

#endif /* COMM_PROTOCOL_H_ */
