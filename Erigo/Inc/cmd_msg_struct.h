#ifndef _CMD_MSG_STRUCT_H_
#define _CMD_MSG_STRUCT_H_

#include <stdint.h>

typedef struct WAV_CMD_DATA{
	uint8_t cmd_id;
    uint16_t test_amp_ma;
	uint16_t nm_amp_ma;
	uint16_t freq_sel;
}WAV_CMD_DATA;

#endif
