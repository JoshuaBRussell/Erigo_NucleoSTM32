/*
 * comm_protocol.c
 *
 *  Created on: Mar 11, 2019
 *      Author: Joshua B. Russell
 */
/* Includes -------------------------------------------------------------------*/
#include "comm_protocol.h"

#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdbool.h"
#include "min.h"

/* Defines -------------------------------------------------------------------*/


extern UART_HandleTypeDef huart2;
extern struct min_context min_ctx;

static bool comm_success = false;
static uint8_t CMD_ID = 0;

WAV_CMD_DATA CMD_DATA;

bool is_comm_success(){
	return comm_success;
}

uint8_t comm_get_control_params(){

    while(!comm_success)
    {
	uint8_t rec_buffer[CMD_MESSAGE_SIZE];
	memset(&rec_buffer[0], 0, sizeof(rec_buffer));

	HAL_UART_Receive(&huart2, rec_buffer, CMD_MESSAGE_SIZE, HAL_MAX_DELAY);

    min_poll(&min_ctx, rec_buffer, CMD_MESSAGE_SIZE);

    }

    return CMD_ID;

}

void parse_message(uint8_t msg_id, uint8_t *min_payload, uint8_t len_payload){
     if(msg_id == WAV_GEN_MSG_IDENTIFIER){
         CMD_ID = msg_id;

    	 CMD_DATA.test_amp_ma = ((uint16_t)min_payload[2] << 8) | ((uint16_t)min_payload[1]);
    	 CMD_DATA.nm_amp_ma =   ((uint16_t)min_payload[4] << 8) | ((uint16_t)min_payload[3]);
    	 CMD_DATA.freq_sel =    ((uint16_t)min_payload[6] << 8) | ((uint16_t)min_payload[5]);

    	 comm_success = true;
     }else if (msg_id == DATA_LOG_MSG_IDENTIFIER){
    	 CMD_ID = msg_id;

    	 comm_success = true;
     }
}
