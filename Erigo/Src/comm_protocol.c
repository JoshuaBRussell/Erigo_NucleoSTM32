/*
 * comm_protocol.c
 *
 *  Created on: Mar 11, 2019
 *      Author: Joshua B. Russell
 */
/* Includes -------------------------------------------------------------------*/
#include "comm_protocol.h"

/* Defines -------------------------------------------------------------------*/
#define SERIAL_START_CHAR 0x02 //Start Of Text in ASCII
#define SERIAL_END_CHAR 0x03   //End Of Text in ASCII
#define SERIAL_DELIMITER 0xF1

#define SERIAL_MESSAGE_SIZE 10//Includes start/end/delimiter bytes also


bool check_message_indicators(uint8_t* buffer, uint16_t buffer_size){
	//Start message is received?
	if(buffer[0] == SERIAL_START_CHAR){
		//End Message is received?
		if(buffer[SERIAL_MESSAGE_SIZE-1] == SERIAL_END_CHAR){
			//Delimiters in correct location?
			if(buffer[3]==SERIAL_DELIMITER && buffer[6]==SERIAL_DELIMITER){
                return true;
			}
		}
	}
	return false;
}


void parse_message(uint8_t* buffer, uint16_t buffer_size, uint16_t* const val1, uint16_t* const val2, uint16_t* const val3){
	//get Test Amplitude[1,2]
    *val1 = ((uint16_t)buffer[1] << 8) | ((uint16_t)buffer[2]);
	//Get NM_Amplitude[4,5]
    *val2 = ((uint16_t)buffer[4] << 8) | ((uint16_t)buffer[5]);
	//Get Freq. Selection[7,8]
    *val3 = ((uint16_t)buffer[7] << 8) | ((uint16_t)buffer[8]);
}
