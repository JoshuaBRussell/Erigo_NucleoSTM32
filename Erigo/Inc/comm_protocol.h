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

#define SERIAL_MESSAGE_SIZE 14
#define WAV_GEN_MSG_IDENTIFIER 0x01
#define DATA_LOG_MSG_IDENTIFIER 0x02


typedef struct WAv_CMD_DATA{
    uint16_t test_amp_ma;
	uint16_t nm_amp_ma;
	uint16_t freq_sel;
}WAV_CMD_DATA;

extern WAV_CMD_DATA CMD_DATA;

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
void parse_message(uint8_t msg_id, uint8_t *min_payload, uint8_t len_payload);

#endif /* COMM_PROTOCOL_H_ */
