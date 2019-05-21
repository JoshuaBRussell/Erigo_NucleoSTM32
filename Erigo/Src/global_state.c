

#include "global_state.h"

//Hidden Global State variable.
static enum GLOBAL_STATE ERIGO_GLOBAL_STATE = IDLE_STATE;


enum GLOBAL_STATE getGlobalState(){
	return ERIGO_GLOBAL_STATE;
}


void setGlobalState_IDLE_STATE(){
	ERIGO_GLOBAL_STATE = IDLE_STATE;
}

void setGlobalState_STIM_CONTROL(){
	ERIGO_GLOBAL_STATE = STIM_CONTROL;
}

void setGlobalState_ADC_OUTPUT(){
	ERIGO_GLOBAL_STATE = ADC_OUTPUT;
}
