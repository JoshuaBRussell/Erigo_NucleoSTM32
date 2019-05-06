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

//Sets up the parameters for stimulation control based values
//Values are in a typedef struct. The pointer to which should be passed into stim control
void stim_control_setup(WAV_CMD_DATA* cmd_data);

//Sets the initial value for the DAC.
//Starts the timer which triggers the stimulation control state machine
void stim_control_start();

/*Once it is determined that a diagnostic pulse should be produced (by what ever means the user wishes),
 *this should be called, and when appropriate the stimulation control state machine will
 *cause a diagnostic pulse.
 *
 *Once the diagnostic pulse is produced, the flag will automatically be cleared
 */


void set_diagnostic_pulse_flag();


#endif
