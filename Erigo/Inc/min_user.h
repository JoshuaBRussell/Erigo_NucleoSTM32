/*Header file for user supplied functions for MIN Protocol found at: https://github.com/min-protocol/min.
* Joshua B. Russell.
*
*
*/

#ifndef __MIN_USER_HEADER__
#define __MIN_USER_HEADER__

#include <stdint.h>


// There are several callbacks: these must be provided by the programmer and are called by the library:
//
// -  min_tx_space()
//    The programmer's serial drivers must return the number of bytes of space available in the sending buffer.
//    This helps cut down on the number of lost frames (and hence improve throughput) if a doomed attempt to transmit a
//    frame can be avoided.
uint16_t min_tx_space(uint8_t port);

// -  min_tx_byte()
//    The programmer's drivers must send a byte on the given port. The implementation of the serial port drivers
//    is in the domain of the programmer: they might be interrupt-based, polled, etc.
void min_tx_byte(uint8_t port, uint8_t byte);

// -  min_application_handler()
//    This is the callback that provides a MIN frame received on a given port to the application. The programmer
//    should then deal with the frame as part of the application.
void min_application_handler(uint8_t min_id, uint8_t *min_payload, uint8_t len_payload, uint8_t port);

// -  min_time_ms()
//    This is called to obtain current time in milliseconds. This is used by the MIN transport protocol to drive
//    timeouts and retransmits.
uint32_t min_time_ms(void);

#endif