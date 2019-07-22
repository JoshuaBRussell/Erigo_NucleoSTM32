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
static struct min_context min_ctx;

static bool comm_success = false;
static uint8_t rec_buffer[CMD_MESSAGE_SIZE];

WAV_CMD_DATA CMD_DATA;

static void comm_allow(){
	memset(&rec_buffer[0], 0, sizeof(rec_buffer));

	HAL_UART_Receive_IT(&huart2, rec_buffer, CMD_MESSAGE_SIZE);

}

bool is_comm_success(){
	return comm_success;
}

void comm_reset_success(){
	comm_success = false;
}

WAV_CMD_DATA* comm_init(){

    //Init of MIN CTX//
    min_init_context(&min_ctx, 0);

    comm_allow();

    return &CMD_DATA;
}

void comm_get_control_params_blocking(){

	comm_reset_success(); //Reset success so only a new message will work
	while(!is_comm_success()){}
    comm_reset_success();
}

void parse_message(uint8_t msg_id, uint8_t *min_payload, uint8_t len_payload){
     if(msg_id == WAV_GEN_MSG_IDENTIFIER){

         //Since parse_message gets called from min_user(at least not easily,nor without serious modification),
         //it can't have the CMD_DATA ptr from
         //get_control_params passes in. BUT since, the definition of the CMD_DATA container
         //is in this file, we can reference it here.
    	 CMD_DATA.cmd_id = msg_id;

    	 CMD_DATA.StimMode =              ((uint32_t)min_payload[0] << 24 | (uint32_t)min_payload[1] << 16 | (uint32_t)min_payload[2] << 8 | (uint32_t)min_payload[3]);
    	 CMD_DATA.TestAmp_mA =            ((uint32_t)min_payload[4] << 24 | (uint32_t)min_payload[5] << 16 | (uint32_t)min_payload[6] << 8 | (uint32_t)min_payload[7]);
    	 CMD_DATA.TestPreTime_ms =        ((uint32_t)min_payload[8] << 24 | (uint32_t)min_payload[9] << 16 | (uint32_t)min_payload[10] << 8 | (uint32_t)min_payload[11]);
    	 CMD_DATA.TestPostTime_ms =       ((uint32_t)min_payload[12] << 24 | (uint32_t)min_payload[13] << 16 | (uint32_t)min_payload[14] << 8 | (uint32_t)min_payload[15]);
    	 CMD_DATA.TestTotalPulses =       ((uint32_t)min_payload[16] << 24 | (uint32_t)min_payload[17] << 16 | (uint32_t)min_payload[18] << 8 | (uint32_t)min_payload[19]);
    	 CMD_DATA.TestInterCycles =       ((uint32_t)min_payload[20] << 24 | (uint32_t)min_payload[21] << 16 | (uint32_t)min_payload[22] << 8 | (uint32_t)min_payload[23]);
    	 CMD_DATA.TestCyclePhase =        ((uint32_t)min_payload[24] << 24 | (uint32_t)min_payload[25] << 16 | (uint32_t)min_payload[26] << 8 | (uint32_t)min_payload[27]);
    	 CMD_DATA.CondAmp_mA =            ((uint32_t)min_payload[28] << 24 | (uint32_t)min_payload[29] << 16 | (uint32_t)min_payload[30] << 8 | (uint32_t)min_payload[31]);
    	 CMD_DATA.CondFreq_Hz =           ((uint32_t)min_payload[32] << 24 | (uint32_t)min_payload[33] << 16 | (uint32_t)min_payload[34] << 8 | (uint32_t)min_payload[35]);
    	 CMD_DATA.CondNumOfPulses =       ((uint32_t)min_payload[36] << 24 | (uint32_t)min_payload[37] << 16 | (uint32_t)min_payload[38] << 8 | (uint32_t)min_payload[39]);
    	 CMD_DATA.ThresholdZeroCrossing = ((uint32_t)min_payload[40] << 24 | (uint32_t)min_payload[41] << 16 | (uint32_t)min_payload[42] << 8 | (uint32_t)min_payload[43]);



    	 comm_success = true;

     }else if (msg_id == DATA_LOG_MSG_IDENTIFIER){
    	 CMD_DATA.cmd_id = msg_id;

    	 comm_success = true;

     }else if(msg_id == STOP_PROC_ID){
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
	min_poll(&min_ctx, rec_buffer, CMD_MESSAGE_SIZE);

	comm_allow();  //Allows for another msg to be received using ISR
}
