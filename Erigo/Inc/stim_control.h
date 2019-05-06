#ifndef _STIM_CONTROL_H_
#define _STIM_CONTROL_H_
/* Stim Control Header
 *
 *
 */
/* Includes ------------------------------------------------------------------*/
#include "cmd_msg_struct.h"

/* Defines -------------------------------------------------------------------*/


enum FREQ_OPTIONS{
	FREQ_12_5Hz = 0,
	FREQ_25Hz,
	FREQ_50Hz,
	FREQ_100Hz

};

enum WF_STATE{
    STIM_FREQ_TRIGGER_LOW = 0,
	STIM_FREQ_TRIGGER_HIGH,
	STIM_TEST_TRIGGER_LOW_PRETEST,
	STIM_TEST_TRIGGER_HIGH,
	STIM_TEST_TRIGGER_LOW_POSTTEST

};

void stim_control_setup(WAV_CMD_DATA* cmd_data);

void stim_control_start();

void set_test_flag();
void unset_test_flag();


#endif
