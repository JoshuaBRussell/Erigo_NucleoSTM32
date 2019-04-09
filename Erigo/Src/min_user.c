/*Implementation of user supplied functions for MIN Protocol found at: https://github.com/min-protocol/min.
* Joshua B. Russell.
*
*
*/
#include <stdint.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"
#include "comm_protocol.h"

extern UART_HandleTypeDef huart2;

uint16_t min_tx_space(uint8_t port){
    return SERIAL_MESSAGE_SIZE;
}


void min_tx_byte(uint8_t port, uint8_t byte){
	 HAL_UART_Transmit(&huart2, &byte, 1, HAL_MAX_DELAY);
}


void min_application_handler(uint8_t min_id, uint8_t *min_payload, uint8_t len_payload, uint8_t port){
	//if valild MIN messege parse it
	parse_message(min_id);

	//if valid command messege, comm_success = true
	HAL_UART_Transmit(&huart2, &min_id, 1, HAL_MAX_DELAY);
}


uint32_t min_time_ms(void){
    return 0x12345678;
}
