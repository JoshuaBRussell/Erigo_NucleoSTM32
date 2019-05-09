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
#include "cmd_msg_struct.h"

/* Defines -------------------------------------------------------------------*/
#define BYTES_PER_ADC_SAMPLE 4
#define DATA_OUT_PAYLOAD_SIZE 200 //Chosen so that it is a multiple of BYTES_PER_ADC_SAMPLE. Must be lower than MIN MAX_PAYLOAD
#define SAMPLES_PER_FRAME (DATA_OUT_PAYLOAD_SIZE/BYTES_PER_ADC_SAMPLE)

extern UART_HandleTypeDef huart2;
extern struct min_context min_ctx;

static bool comm_success = false;

WAV_CMD_DATA CMD_DATA;

bool is_comm_success(){
	return comm_success;
}

void comm_reset_seccess(){
	comm_success = false;
}

WAV_CMD_DATA* comm_get_control_params(WAV_CMD_DATA* cmd_data){

    while(!comm_success)
    {
	uint8_t rec_buffer[CMD_MESSAGE_SIZE];
	memset(&rec_buffer[0], 0, sizeof(rec_buffer));

	HAL_UART_Receive(&huart2, rec_buffer, CMD_MESSAGE_SIZE, HAL_MAX_DELAY);

    min_poll(&min_ctx, rec_buffer, CMD_MESSAGE_SIZE);

    }
    return &CMD_DATA;

}

void parse_message(uint8_t msg_id, uint8_t *min_payload, uint8_t len_payload){
     if(msg_id == WAV_GEN_MSG_IDENTIFIER){

         //Since parse_message gets called from min_user(at least not easily,nor without serious modification),
         //it can't have the CMD_DATA ptr from
         //get_control_params passes in. BUT since, the definition of the CMD_DATA container
         //is in this file, we can reference it here.
    	 CMD_DATA.cmd_id = msg_id;
    	 CMD_DATA.test_amp_ma = ((uint16_t)min_payload[2] << 8) | ((uint16_t)min_payload[1]);
    	 CMD_DATA.nm_amp_ma =   ((uint16_t)min_payload[4] << 8) | ((uint16_t)min_payload[3]);
    	 CMD_DATA.freq_sel =    ((uint16_t)min_payload[6] << 8) | ((uint16_t)min_payload[5]);


    	 comm_success = true;
     }else if (msg_id == DATA_LOG_MSG_IDENTIFIER){
    	 CMD_DATA.cmd_id = msg_id;

    	 comm_success = true;
     }

     //if a valid msg was received, send the appropriate ACK
     if(comm_success==true){
         comm_send_ACK(msg_id);
     //if not, send an ERROR msg (not yet implemented)
     }else{

     }
}


void comm_send_data(uint32_t* data_buffer, uint32_t buff_len){
    //volatile b/c they kept getting optimized out
	volatile uint32_t num_of_MIN_frames = 0;
    volatile uint8_t excess_samples = 0;

    //Find # of MIN frames needed to send data
    if((buff_len*BYTES_PER_ADC_SAMPLE)%(DATA_OUT_PAYLOAD_SIZE)==0){
	    num_of_MIN_frames = (buff_len*BYTES_PER_ADC_SAMPLE)/(DATA_OUT_PAYLOAD_SIZE);
    }else{
	    num_of_MIN_frames = (buff_len*BYTES_PER_ADC_SAMPLE)/(DATA_OUT_PAYLOAD_SIZE) + 1;
	    excess_samples = ((buff_len*BYTES_PER_ADC_SAMPLE)%(DATA_OUT_PAYLOAD_SIZE))/BYTES_PER_ADC_SAMPLE;
    }

    //Fills frames with data the will completely fit.
    //First loop acts as a window(window size is the number of samples that can fit in a frame)
    //Second loop looks over each sample in that window, and places individual bytes into
    //the tx/payload buffer
    uint8_t temp_tx_buff[DATA_OUT_PAYLOAD_SIZE];
    for(int i = 0; i < (buff_len - excess_samples); i+=SAMPLES_PER_FRAME){
	    for(int j = 0; j < SAMPLES_PER_FRAME; j ++){
	    	//The *4 multiplication steps through each sample, but in terms of bytes
		    temp_tx_buff[4*j]   = (data_buffer[i + j] >> 24) & 0xFF;
		    temp_tx_buff[4*j+1] = (data_buffer[i + j] >> 16) & 0xFF;
		    temp_tx_buff[4*j+2] = (data_buffer[i + j] >>  8) & 0xFF;
		    temp_tx_buff[4*j+3] = (data_buffer[i + j])       & 0xFF;
	    }
	    min_send_frame(&min_ctx, DATA_LOG_MSG_IDENTIFIER, temp_tx_buff, DATA_OUT_PAYLOAD_SIZE);
    }


    //Send any "excess bytes" in one last MIN frame if needed
    if(excess_samples > 0){
	    for(int j = 0; j < excess_samples; j++){
		    temp_tx_buff[4*j]   = (data_buffer[(buff_len - excess_samples) + j] >> 24) & 0xFF;
		    temp_tx_buff[4*j+1] = (data_buffer[(buff_len - excess_samples) + j] >> 16) & 0xFF;
		    temp_tx_buff[4*j+2] = (data_buffer[(buff_len - excess_samples) + j] >>  8) & 0xFF;
		    temp_tx_buff[4*j+3] = (data_buffer[(buff_len - excess_samples) + j])       & 0xFF;
	    }
	    min_send_frame(&min_ctx, DATA_LOG_MSG_IDENTIFIER, temp_tx_buff, excess_samples*BYTES_PER_ADC_SAMPLE);
    }

    //Send an empty MIN msg with a ID that signifies the end of data transfer
    min_send_frame(&min_ctx, ADC_OUTPUT_END_OF_DATA, temp_tx_buff, 0);

}

void comm_send_ACK(uint8_t msg_id){
	uint8_t null_buff = 0;  //For the ACK, no data needs to be sent,
	                        //but a pointer must be supplied to min_send_frame as a buff

	min_send_frame(&min_ctx, msg_id, &null_buff, 0);

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    __NOP();
}
