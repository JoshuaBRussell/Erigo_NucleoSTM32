

#include "global_state.h"
#include "main.h"


//Hidden Global State variable.
static enum GLOBAL_STATE ERIGO_GLOBAL_STATE = IDLE_STATE;

static void clearLEDPins(){
	HAL_GPIO_WritePin(IDLE_State_GPIO_Port, IDLE_State_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Stim_State_GPIO_Port, Stim_State_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ADC_Output_State_GPIO_Port, ADC_Output_State_Pin, GPIO_PIN_RESET);

}


enum GLOBAL_STATE getGlobalState(){
	return ERIGO_GLOBAL_STATE;
}


void setGlobalState_IDLE_STATE(){
	ERIGO_GLOBAL_STATE = IDLE_STATE;

	clearLEDPins();
	HAL_GPIO_WritePin(IDLE_State_GPIO_Port, IDLE_State_Pin, GPIO_PIN_SET);
}

void setGlobalState_STIM_CONTROL(){
	ERIGO_GLOBAL_STATE = STIM_CONTROL;

	clearLEDPins();

	HAL_GPIO_WritePin(Stim_State_GPIO_Port, Stim_State_Pin, GPIO_PIN_SET);
}

void setGlobalState_ADC_OUTPUT(){
	ERIGO_GLOBAL_STATE = ADC_OUTPUT;

	clearLEDPins();
	HAL_GPIO_WritePin(ADC_Output_State_GPIO_Port, ADC_Output_State_Pin, GPIO_PIN_SET);
}
