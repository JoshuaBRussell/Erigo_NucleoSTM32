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
#include "cmd_msg_struct.h"

#define SERIAL_MESSAGE_SIZE 255
#define CMD_MESSAGE_SIZE 16
#define NO_MSG_IDENTIFIER       0x00;
#define WAV_GEN_MSG_IDENTIFIER  0x01
#define DATA_LOG_MSG_IDENTIFIER 0x02
#define ADC_OUTPUT_END_OF_DATA  0x03
#define STOP_PROC_ID            0x04


//typedef struct WAV_CMD_DATA{
//    uint16_t test_amp_ma;
//	uint16_t nm_amp_ma;
//	uint16_t freq_sel;
//}WAV_CMD_DATA;

//extern WAV_CMD_DATA CMD_DATA;

bool is_comm_success();

void comm_reset_success();

//Allows for new msgs to come in.
void comm_allow();

//A ptr to that typedef struct is returned
WAV_CMD_DATA* comm_init();

/* Wait for CMD message from whatever comm protocol is desired.
 *
 * A ptr to that typedef struct is returned
 */
void comm_get_control_params_blocking();

/* Parses buffer for data.
 *
 * msg_id      - msg id indicating message type
 * min_payload - buffer where incoming MIN payload data was stored
 * len_payload - size of the buffer
 */
//"valx" so it is generic.
void parse_message(uint8_t msg_id, uint8_t *min_payload, uint8_t len_payload);

void comm_send_data(uint32_t* data_buffer, uint32_t buff_len);

//Sends an ACK msg with the specified msg_id
void comm_send_ACK(uint8_t msg_id);

#endif /* COMM_PROTOCOL_H_ */
