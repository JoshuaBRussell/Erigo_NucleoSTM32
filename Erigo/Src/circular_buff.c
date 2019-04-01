/*Cicular Buffer Implementaiton File
  Joshua B. Russell  
*/
#include "circular_buff.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

struct circ_buff{
    uint32_t* buffer;
    size_t head;
    size_t tail;
    size_t max_buffer_size;
    bool is_full;

    uint64_t buffer_sum;
};

//Creates a user defined number of buffs of size NUMBER_OF_BUFFS. 
static struct circ_buff CIRC_BUFFS[NUMBER_OF_BUFFS];
static size_t num_of_buffs_issued = 0;

//----static helper functions----//

//Advances the head/tail indices of the circular buffer
static void advance_indices(circ_buff_handle circ_buff){
    assert(circ_buff);

    if (circ_buff->is_full){
        circ_buff->tail = (circ_buff->tail + 1) % circ_buff->max_buffer_size;
    }

    circ_buff->head = (circ_buff->head + 1) % circ_buff->max_buffer_size;
    circ_buff->is_full = (circ_buff->head == circ_buff->tail);
}

static void retreat_indices(circ_buff_handle circ_buff){
    assert(circ_buff);

    circ_buff->is_full = false;
    circ_buff->tail = (circ_buff->tail + 1) % circ_buff->max_buffer_size;
}


//----Circular Buffer Interface Functions----//
circ_buff_handle circ_buff_init(uint32_t* buffer, uint32_t length){
    assert(buffer && length);
    
    circ_buff_handle circ_buff = NULL;
    if(num_of_buffs_issued != NUMBER_OF_BUFFS){
        circ_buff = CIRC_BUFFS+num_of_buffs_issued; 
        num_of_buffs_issued++;
        circ_buff->buffer = buffer;
        circ_buff->max_buffer_size = length;
        circ_buff_reset(circ_buff);
    }

    return circ_buff;
}

void circ_buff_reset(circ_buff_handle circ_buff){
    assert(circ_buff);
    
    circ_buff->head = 0;
    circ_buff->tail = 0;
    circ_buff->buffer_sum = 0;

    circ_buff->is_full = false;
}

void circ_buff_put(circ_buff_handle circ_buff, uint32_t data){
    assert(circ_buff);

    if(circ_buff->is_full){
        circ_buff->buffer_sum-=circ_buff->buffer[circ_buff->head];
    }
    circ_buff->buffer[circ_buff->head] = data;
    circ_buff->buffer_sum+=data;


    advance_indices(circ_buff);
}

int circ_buff_get(circ_buff_handle circ_buff, uint32_t* data){
    assert(circ_buff);

    int return_flag = -1;

    if(!circ_buff_is_empty(circ_buff)){
        *data = circ_buff->buffer[circ_buff->tail];
        circ_buff->buffer_sum-=circ_buff->buffer[circ_buff->tail];
        retreat_indices(circ_buff);
        return_flag = 0;
    }
    return return_flag;
}

bool circ_buff_is_full(circ_buff_handle circ_buff){
    assert(circ_buff);

    return circ_buff->is_full;
}

bool circ_buff_is_empty(circ_buff_handle circ_buff){
    assert(circ_buff);

    return (!circ_buff->is_full && (circ_buff->head == circ_buff->tail));
}

size_t circ_buff_max_capacity(circ_buff_handle circ_buff){
    assert(circ_buff);

    return circ_buff->max_buffer_size;
}

size_t circ_buff_size(circ_buff_handle circ_buff){
    assert(circ_buff);

    size_t size = circ_buff->max_buffer_size;

    if(!circ_buff->is_full){
        if(circ_buff->head >= circ_buff->tail){
            size = (circ_buff->head - circ_buff->tail);
        }
        else{
            size = (circ_buff->max_buffer_size + circ_buff->head - circ_buff->tail);
        }
    }

    return size;
}

uint32_t circ_buff_get_avg(circ_buff_handle circ_buff){
    assert(circ_buff);

    return (uint32_t)(circ_buff->buffer_sum/(circ_buff_size(circ_buff)));
}
