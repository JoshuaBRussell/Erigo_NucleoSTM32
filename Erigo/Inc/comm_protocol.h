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

#define SERIAL_MESSAGE_SIZE 255
#define CMD_MESSAGE_SIZE 15
#define NO_MSG_IDENTIFIER       0x00;
#define WAV_GEN_MSG_IDENTIFIER  0x01
#define DATA_LOG_MSG_IDENTIFIER 0x02
#define ADC_OUTPUT_END_OF_DATA  0x03


typedef struct WAv_CMD_DATA{
    uint16_t test_amp_ma;
	uint16_t nm_amp_ma;
	uint16_t freq_sel;
}WAV_CMD_DATA;

extern WAV_CMD_DATA CMD_DATA;

bool is_comm_success();

void comm_reset_seccess();

/* Gets CMD message from whatever comm protocol is desired.
 * Will get cmd message and then transfer data into the appropriate struct.
 *
 */
uint8_t comm_get_control_params();

/* Parses buffer for data.
 *
 * msg_id      - msg id indicating message type
 * min_payload - buffer where incoming MIN payload data was stored
 * len_payload - size of the buffer
 */
//"valx" so it is generic.
void parse_message(uint8_t msg_id, uint8_t *min_payload, uint8_t len_payload);

void comm_send_data(uint32_t* data_buffer, uint32_t buff_len);

#endif /* COMM_PROTOCOL_H_ */
