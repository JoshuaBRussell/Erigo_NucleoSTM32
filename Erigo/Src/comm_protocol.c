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

bool is_comm_success(){
	return comm_success;
}

void comm_get_control_params(uint16_t* val1, uint16_t* val2, uint16_t* val3){

    while(!comm_success)
    {
	uint8_t rec_buffer[SERIAL_MESSAGE_SIZE];
	memset(&rec_buffer[0], 0, sizeof(rec_buffer));

	HAL_UART_Receive(&huart2, rec_buffer, SERIAL_MESSAGE_SIZE, HAL_MAX_DELAY);

    min_poll(&min_ctx, rec_buffer, SERIAL_MESSAGE_SIZE);

    }

}

void parse_message(uint8_t msg_id){
     if(msg_id == WAV_GEN_MSG_IDENTIFIER){
    	 comm_success = true;
     }
}
