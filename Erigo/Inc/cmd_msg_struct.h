#ifndef _CMD_MSG_STRUCT_H_
#define _CMD_MSG_STRUCT_H_

#include <stdint.h>

typedef struct WAV_CMD_DATA{
	uint8_t cmd_id;

	uint32_t stimmode;
	uint32_t test_amp_ma;
	uint32_t testpretime_ms;
	uint32_t testposttime_ms;
	uint32_t testtotalpulses;
	uint32_t testintercycles;
	uint32_t testcyclephase;
	uint32_t nm_amp_ma;
	uint32_t freq_sel;
	uint32_t condnumofpulses;
	uint32_t thresholdzerocrossing;
}WAV_CMD_DATA;

#endif
