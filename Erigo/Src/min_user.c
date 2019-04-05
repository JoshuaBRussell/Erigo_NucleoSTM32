/*Implementation of user supplied functions for MIN Protocol found at: https://github.com/min-protocol/min.
* Joshua B. Russell.
*
*
*/
#include <stdint.h>
#include <stdio.h>


uint16_t min_tx_space(uint8_t port){
    return 255;
}


void min_tx_byte(uint8_t port, uint8_t byte){
    printf("TX: %u\n", byte);
}


void min_application_handler(uint8_t min_id, uint8_t *min_payload, uint8_t len_payload, uint8_t port){
    printf("FRAME DONE GOT GOT!\n");
    printf("ID: %u\n", min_id);
    for (int i = 0; i < len_payload; i++){
        printf("Data [%u]: %u\n", i, min_payload[i]);
    }
}


uint32_t min_time_ms(void){
    return 0x12345678;
}