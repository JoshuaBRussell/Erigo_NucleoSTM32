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
#include <stdint.h>
#include "cmd_msg_struct.h"
#include "global_state.h"

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

//Needed HW Handlers
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

static enum WF_STATE STIM_STATE = STIM_FREQ_TRIGGER_LOW;

static uint16_t TestAmp_DAC = 0;
static uint16_t CondAmp_DAC = 0;

static uint16_t T_LOW = 0; //Initially based on freq.
static uint16_t T_PERIOD = 0;

static bool send_diagnostic_pulse = false;
static uint32_t time_of_diagnostic_pulse = 0;

//This is only called at the onset of the program a couple of times. Otherwise the
//cast and float multiplication would be considered "not pretty."
static void milliamps_to_DAC_counts(const uint16_t in_milliamp, uint16_t* dac_counts){
    *dac_counts =  (uint16_t)(MILLIAMP_TO_DAC_CONV_FACTOR * (float)in_milliamp);
}

//Thi
static void unset_diagnostic_pulse_flag(){
	send_diagnostic_pulse = false;
}

void set_diagnostic_pulse_flag(){
	send_diagnostic_pulse = true;
}

uint32_t get_time_of_last_diagnostic_pulse(){
    return time_of_diagnostic_pulse;
}

void stim_control_setup(WAV_CMD_DATA* cmd_data){
	uint32_t TestAmp_mA, CondAmp_mA, CondFreq_Hz;

	//Update local variables from WAV_GEN_CMD data. (comm_get_control_params MUST be called and successful.)
	TestAmp_mA = cmd_data->TestAmp_mA;
	CondAmp_mA = cmd_data->CondAmp_mA;
	CondFreq_Hz = cmd_data->CondFreq_Hz;

	//Convert
	milliamps_to_DAC_counts(TestAmp_mA, &TestAmp_DAC);
	milliamps_to_DAC_counts(CondAmp_mA, &CondAmp_DAC);

	switch(CondFreq_Hz){
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
  	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, CondAmp_DAC);
  	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

  	HAL_TIM_Base_Start_IT(&htim3);
}

void stim_control_reset(){

	//( Order is important here. Stim ISR turns off first
	//so no other stimulation pulses are sent. Then the amplitude. Then the ADC sampling.)
	HAL_TIM_Base_Stop_IT(&htim3); //Turn off Stim Waveform Timer ISR
	HAL_TIM_Base_Stop_IT(&htim4); //Turn off predictive stim ISR so timer doesn't
	                              //set_diagnostic_pulse_flag after stim state should end

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET); //Reset LD2/STim pin

	unset_diagnostic_pulse_flag(); //Ensures the flag is reset. This can still be set sometimes,
                                   //if the time expires before the diagnostic pulse can be produced

    STIM_STATE = STIM_FREQ_TRIGGER_LOW;

	//Set stim amplitude to zero
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

	HAL_TIM_Base_Stop_IT(&htim2); //Turn of ADC sampling
	HAL_ADC_Stop_IT(&hadc1);
	HAL_ADC_Stop_IT(&hadc2);


}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim4){
		set_diagnostic_pulse_flag();
		HAL_TIM_Base_Stop_IT(&htim4);
	}


	if (htim==&htim3) {  //This should only run for TIM3's Period Elapse
		if (getGlobalState() == STIM_CONTROL) {  //This should only run if we are in the correct state(STIM_CONTROL).
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

			//HAL_GPIO_TogglePin(SCOPE_Pin_GPIO_Port, SCOPE_Pin_Pin);

			switch(STIM_STATE)
			{
			case STIM_FREQ_TRIGGER_LOW:
				__HAL_TIM_SET_AUTORELOAD(&htim3, TPULSE_IN_COUNTS);
				STIM_STATE = STIM_FREQ_TRIGGER_HIGH;

				HAL_GPIO_WritePin(Stim_NM_GPIO_Port, Stim_NM_Pin, GPIO_PIN_SET);
				break;

			case STIM_FREQ_TRIGGER_HIGH:
				if(send_diagnostic_pulse){
					__HAL_TIM_SET_AUTORELOAD(&htim3, STIM_LOW_PRETEST_IN_COUNTS);
					HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, TestAmp_DAC);
					HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
					STIM_STATE = STIM_TEST_TRIGGER_LOW_PRETEST;
				}else{
					__HAL_TIM_SET_AUTORELOAD(&htim3, T_LOW);
					STIM_STATE = STIM_FREQ_TRIGGER_LOW;
				}

				HAL_GPIO_WritePin(Stim_NM_GPIO_Port, Stim_NM_Pin, GPIO_PIN_RESET);
				break;

			case STIM_TEST_TRIGGER_LOW_PRETEST:
				__HAL_TIM_SET_AUTORELOAD(&htim3, TPULSE_IN_COUNTS);
				time_of_diagnostic_pulse = HAL_GetTick();
				STIM_STATE = STIM_TEST_TRIGGER_HIGH;

				HAL_GPIO_WritePin(Stim_Test_GPIO_Port, Stim_Test_Pin, GPIO_PIN_SET);
				break;

			case STIM_TEST_TRIGGER_HIGH:
				unset_diagnostic_pulse_flag();
				__HAL_TIM_SET_AUTORELOAD(&htim3, STIM_LOW_POSTTEST_IN_COUNTS);
				HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, CondAmp_DAC);
				HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
				STIM_STATE = STIM_TEST_TRIGGER_LOW_POSTTEST;

				HAL_GPIO_WritePin(Stim_Test_GPIO_Port, Stim_Test_Pin, GPIO_PIN_RESET);
				break;

			case STIM_TEST_TRIGGER_LOW_POSTTEST:
				__HAL_TIM_SET_AUTORELOAD(&htim3, TPULSE_IN_COUNTS);
				STIM_STATE = STIM_FREQ_TRIGGER_HIGH;

				HAL_GPIO_WritePin(Stim_NM_GPIO_Port, Stim_NM_Pin, GPIO_PIN_SET);
				break;
			}
		}else{
			//Send some kind of error message?
		}
	}

}
