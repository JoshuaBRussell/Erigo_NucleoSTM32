#ifndef _GLOBAL_STATE_H_
#define _GLOBAL_STATE_H_


//Provides access to global state of Erigo Stimulator uC.


enum GLOBAL_STATE{
    IDLE_STATE = 0,
	STIM_CONTROL,
	ADC_OUTPUT
};

enum GLOBAL_STATE getGlobalState();

//The states are explicitly written into the function names so it is clearer what the state is being set to,
//and that way there can be no accidental conversion from magic ints to the wrong state.
void setGlobalState_IDLE_STATE();
void setGlobalState_STIM_CONTROL();
void setGlobalState_ADC_OUTPUT();


#endif //_GLOBAL_STATE_H_
