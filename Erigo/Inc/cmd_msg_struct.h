#ifndef _CMD_MSG_STRUCT_H_
#define _CMD_MSG_STRUCT_H_

#include <stdint.h>

typedef struct WAV_CMD_DATA{
	uint8_t cmd_id;

	uint32_t StimMode;
	uint32_t TestAmp_mA;
	uint32_t TestPreTime_ms;
	uint32_t TestPostTime_ms;
	uint32_t TestTotalPulses;
	uint32_t TestInterCycles;
	uint32_t TestCyclePhase;
	uint32_t CondAmp_mA;
	uint32_t CondFreq_Hz;
	uint32_t CondNumOfPulses;
	uint32_t ThresholdZeroCrossing;
}WAV_CMD_DATA;

#endif
