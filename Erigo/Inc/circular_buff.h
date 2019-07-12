#ifndef _CIRC_BUFF_HEADER_
#define _CIRC_BUFF_HEADER_
/* Circular Buffer Header
*  Joshua B. Russell 3/23/29
*
*  Description: 
*  Circular Buffer Header file written in C. 
*
*  Heavily based on the blog post (https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc)
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_NUMBER_OF_BUFFS 5

typedef struct circ_buff* circ_buff_handle;

//returns NULL if no more buffers available
circ_buff_handle circ_buff_init(uint32_t* buffer, uint32_t length);

//void circ_buff_free(circ_buff_handle circ_buff);

void circ_buff_reset(circ_buff_handle circ_buff);

//Places new data into the buffer. Overwrite oldest data.
void circ_buff_put(circ_buff_handle circ_buff, uint32_t data);

//Gets data from the buffer. Returns 0 if success, -1 if buffer is empty. 
int circ_buff_get(circ_buff_handle circ_buff, uint32_t* data);

bool circ_buff_is_full(circ_buff_handle circ_buff);

bool circ_buff_is_empty(circ_buff_handle circ_buff);

size_t circ_buff_max_capacity(circ_buff_handle circ_buff);

size_t circ_buff_size(circ_buff_handle circ_buff);

uint32_t circ_buff_get_avg(circ_buff_handle circ_buff);

uint32_t circ_buff_get_sum(circ_buff_handle circ_buff);
#endif
