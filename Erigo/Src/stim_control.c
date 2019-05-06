/*
 * stim_controller.c
 *
 *  Created on: May 03, 2019
 *      Author: Joshua B. Russell
 */
/* Includes -------------------------------------------------------------------*/
#include "stim_control.h"

#include "main.h"
#include <stdbool.h>

/* Defines -------------------------------------------------------------------*/
#define STIM_FREQ_INTENSITY 1000
#define STIM_TEST_INTENSITY 2500
//These are based on a PSC: 200 and Internal Clock: 84E6
#define TPULSE_IN_COUNTS 2090
#define TPERIOD_100HZ_IN_COUNTS 4178
#define TPERIOD_050HZ_IN_COUNTS 8357
#define TPERIOD_025HZ_IN_COUNTS 16715
#define TPERIOD_12_5HZ_IN_COUNTS 33432

#define STIM_LOW_PRETEST_IN_COUNTS 16715
#define STIM_LOW_POSTTEST_IN_COUNTS 16715

#define MILLIAMP_TO_DAC_CONV_FACTOR 12.4

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim3;

static enum WF_STATE STIM_STATE = STIM_FREQ_TRIGGER_LOW;

static uint16_t Test_Amplitude_in_Counts = 0;
static uint16_t NM_Amplitude_in_Counts = 0;
static uint16_t Freq_Sel_in_Counts = 0;

static uint16_t T_LOW = 0; //Initially based on freq.
static uint16_t T_PERIOD = 0;

static bool TEST_FLAG = false;

//This is only called at the onset of the program a couple of times. Otherwise the
//cast and float multiplication would be considered "not pretty."
static void milliamps_to_DAC_counts(const uint16_t in_milliamp, uint16_t* dac_counts){
    *dac_counts =  (uint16_t)(MILLIAMP_TO_DAC_CONV_FACTOR * (float)in_milliamp);
}

void set_test_flag(){
	TEST_FLAG = true;
}

void unset_test_flag(){
	TEST_FLAG = false;
}




void stim_control_setup(){
	uint16_t test_amp_ma, nm_amp_ma, freq_sel;

	//Update local variables from WAV_GEN_CMD data. (comm_get_control_params MUST be called and successful.)
	test_amp_ma = CMD_DATA.test_amp_ma;
	nm_amp_ma = CMD_DATA.nm_amp_ma;
	freq_sel = CMD_DATA.freq_sel;

	//Convert
	milliamps_to_DAC_counts(test_amp_ma, &Test_Amplitude_in_Counts);
	milliamps_to_DAC_counts(nm_amp_ma, &NM_Amplitude_in_Counts);

	switch(freq_sel){
	case FREQ_12_5Hz :
		T_PERIOD = TPERIOD_12_5HZ_IN_COUNTS;
		break;

	case FREQ_25Hz :
		T_PERIOD = TPERIOD_025HZ_IN_COUNTS;
		break;

	case FREQ_50Hz :
		T_PERIOD = TPERIOD_050HZ_IN_COUNTS;
		break;

	case FREQ_100Hz :
		T_PERIOD = TPERIOD_100HZ_IN_COUNTS;
		break;
	}
	T_LOW = (T_PERIOD-TPULSE_IN_COUNTS);
}

void stim_control_start(){
  	//Set initial value of stim amplitude.
  	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, NM_Amplitude_in_Counts);
  	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

  	HAL_TIM_Base_Start_IT(&htim3);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{


	if (htim==&htim3) {  //This should only run for TIM3's Period Elapse
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

		HAL_GPIO_TogglePin(SCOPE_Pin_GPIO_Port, SCOPE_Pin_Pin);

		switch(STIM_STATE)
		{
		case STIM_FREQ_TRIGGER_LOW:
			__HAL_TIM_SET_AUTORELOAD(&htim3, TPULSE_IN_COUNTS);
			STIM_STATE = STIM_FREQ_TRIGGER_HIGH;
			break;

		case STIM_FREQ_TRIGGER_HIGH:
			if(TEST_FLAG){
				__HAL_TIM_SET_AUTORELOAD(&htim3, STIM_LOW_PRETEST_IN_COUNTS);
				HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Test_Amplitude_in_Counts);
				HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
				STIM_STATE = STIM_TEST_TRIGGER_LOW_PRETEST;
			}else{
				__HAL_TIM_SET_AUTORELOAD(&htim3, T_LOW);
				STIM_STATE = STIM_FREQ_TRIGGER_LOW;
			}
			break;

		case STIM_TEST_TRIGGER_LOW_PRETEST:
			__HAL_TIM_SET_AUTORELOAD(&htim3, TPULSE_IN_COUNTS);
			STIM_STATE = STIM_TEST_TRIGGER_HIGH;
			break;

		case STIM_TEST_TRIGGER_HIGH:
			TEST_FLAG = false;
			__HAL_TIM_SET_AUTORELOAD(&htim3, STIM_LOW_POSTTEST_IN_COUNTS);
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, NM_Amplitude_in_Counts);
			HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
			STIM_STATE = STIM_TEST_TRIGGER_LOW_POSTTEST;
			break;

		case STIM_TEST_TRIGGER_LOW_POSTTEST:
			__HAL_TIM_SET_AUTORELOAD(&htim3, TPULSE_IN_COUNTS);
			STIM_STATE = STIM_FREQ_TRIGGER_HIGH;
			break;
		}
	}

}
