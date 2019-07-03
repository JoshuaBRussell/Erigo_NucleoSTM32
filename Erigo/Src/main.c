/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "string.h"

#include "circular_buff.h"
#include "signal_proc_util.h"
#include "comm_protocol.h"
#include "min.h"
#include "min_user.h"
#include "cmd_msg_struct.h"
#include "stim_control.h"
#include "global_state.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STIM_TRIGGER_THRESHOLD 2000
#define STIM_TRIGGER_TOLERANCE 50
#define STIM_TRIGGER_CYCLE_LIMIT 5 //Number of times the threshold must be reached before Test Pulse is produced.

#define ADC_BUFFER_SIZE 5
#define ADC_DATA_AMOUNT 1000 //Very temp name

#define TEST_AMP_MA_MIN 0
#define TEST_AMP_MA_MAX 500
#define   NM_AMP_MA_MIN 0
#define   NM_AMP_MA_MAX 300
#define    FREQ_SEL_MIN 0
#define    FREQ_SEL_MAX 3

#define MAX_NM_TIME_MS 60000
#define DIAGNOSTIC_PULSE_TIME 500
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

DAC_HandleTypeDef hdac;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t num_of_threshold_crossings = 0;

uint32_t gADC_reading = 0;
uint32_t gADC_reading_Two = 0;

//Two buffers are used since it is easier to use separate buffer buffer arrays/circ buffers
//since they are different sizes.
//Buffer for ADC samples while stimulating. This is used as the underlying array for the associated circ buff
uint32_t stim_adc_buffer_array[ADC_BUFFER_SIZE] = {0};

//Buffer for ADC samples while only measuring. This is used as the underlying array for the associated circ buff
uint32_t meas_adc_buffer_array[ADC_DATA_AMOUNT] = {0};

//Declare circular buffer for ADC values while stimulating
circ_buff_handle stim_adc_circ_buff;

//Declare circular buffer for ADC values while only measuring
circ_buff_handle meas_adc_circ_buff;

bool POS_BELOW_THRESHOLD = false;

bool has_threshold_been_crossed(){
	//STIM_TRIGGER_THRESHOLD +/- STIM_TRIGGER_TOLERANCE implements hysteresis
	return (POS_BELOW_THRESHOLD&&all_above_threshold(stim_adc_buffer_array, ADC_BUFFER_SIZE, STIM_TRIGGER_THRESHOLD + STIM_TRIGGER_TOLERANCE)) || (!POS_BELOW_THRESHOLD&&all_below_threshold(stim_adc_buffer_array, ADC_BUFFER_SIZE, STIM_TRIGGER_THRESHOLD - STIM_TRIGGER_TOLERANCE));
}

WAV_CMD_DATA* CMD_DATA_Handle;

//Checks to make sure it is within the range (low, high) inclusive
bool in_range(uint16_t x, uint16_t low, uint16_t high){
    return ((x>=low)&&(x<=high));
}

//Before the STIM_CONTROL state is entered, the values in the cmd msg are checked to make sure they
//are not out of range
bool check_stim_params(WAV_CMD_DATA* CMD_DATA_Handle){

	bool vals_in_bounds = false;
	//Check test_amp_ma
    if(in_range(CMD_DATA_Handle->test_amp_ma, TEST_AMP_MA_MIN, TEST_AMP_MA_MAX) &&
       in_range(CMD_DATA_Handle->nm_amp_ma, NM_AMP_MA_MIN,NM_AMP_MA_MAX)   &&
	   in_range(CMD_DATA_Handle->freq_sel, FREQ_SEL_MIN, FREQ_SEL_MAX)){
        return true;
    }

    return vals_in_bounds;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_DAC_Init();
  MX_TIM2_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */

  setGlobalState_IDLE_STATE();

  CMD_DATA_Handle = comm_init();  //Init comms and return a msg handler

  //Define the circular buffer used during stim control mode
  stim_adc_circ_buff = circ_buff_init(stim_adc_buffer_array, ADC_BUFFER_SIZE);
  while(!stim_adc_circ_buff){}; //Infinite loop in case it returns NULL

  //Define the circular buffer used during ADC output mode
  meas_adc_circ_buff = circ_buff_init(meas_adc_buffer_array, ADC_DATA_AMOUNT);
  while(!meas_adc_circ_buff){}; //Infinite loop in case it returns NULL
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	    comm_get_control_params_blocking();

	    if (CMD_DATA_Handle->cmd_id == WAV_GEN_MSG_IDENTIFIER && check_stim_params(CMD_DATA_Handle)) {

	    	//This state is not entered until bounds checking is done.
	    	//This is so that the stim output will not change,
	    	//since it makes sure it is in the STIM_CONTROL state first.
	    	setGlobalState_STIM_CONTROL();

			//Start stimulation
			stim_control_setup(CMD_DATA_Handle);
			stim_control_start();

			//Start collecting ADC position samples
			HAL_TIM_Base_Start_IT(&htim2);
			HAL_ADC_Start_IT(&hadc1);
			HAL_ADC_Start_IT(&hadc2);

			//Loops while Reset Msg isn't received AND TIMEOUT hasn't expired
			uint32_t expiration_time = HAL_GetTick() + MAX_NM_TIME_MS;
			while(CMD_DATA_Handle->cmd_id != STOP_PROC_ID && ((int32_t)(expiration_time - HAL_GetTick()) > 0)){};

			num_of_threshold_crossings = 0; //Reset this to zero once stimulation has ended
			stim_control_reset();
			circ_buff_reset(stim_adc_circ_buff);

			//Reset to Idle State only once stim control has been reset
		    setGlobalState_IDLE_STATE();



	    }else if (CMD_DATA_Handle->cmd_id == DATA_LOG_MSG_IDENTIFIER){

	    	setGlobalState_ADC_OUTPUT();

	  	    //----Collect ADC Data----//

	        //Turn on HW Timer/ADC to collect samples
	  	    HAL_TIM_Base_Start_IT(&htim2);
	  	    HAL_ADC_Start_IT(&hadc1);
	  	    HAL_ADC_Start_IT(&hadc2);

	  	    while(!circ_buff_is_full(meas_adc_circ_buff));

	  	    HAL_TIM_Base_Stop_IT(&htim2);
	  	    HAL_ADC_Stop_IT(&hadc1);
	  	    HAL_ADC_Stop_IT(&hadc2);

	  	    //----Send Data to Host----//
	  	    comm_send_data(meas_adc_buffer_array, ADC_DATA_AMOUNT);

	  	    circ_buff_reset(meas_adc_circ_buff);

	  	    //Reset to Idle State
	  	    setGlobalState_IDLE_STATE();
	     }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /**Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure the ADC multi-mode 
  */
  multimode.Mode = ADC_DUALMODE_REGSIMULT;
  multimode.DMAAccessMode = ADC_DMAACCESSMODE_DISABLED;
  multimode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_5CYCLES;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */
  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */
  /**DAC Initialization 
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }
  /**DAC channel OUT1 config 
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 100;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 8316;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 200;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2090;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Stim_Test_Pin|Stim_NM_Pin|SCOPE_Pin_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|Stim_State_Pin|ADC_Output_State_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(IDLE_State_GPIO_Port, IDLE_State_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Stim_Test_Pin Stim_NM_Pin */
  GPIO_InitStruct.Pin = Stim_Test_Pin|Stim_NM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin Stim_State_Pin ADC_Output_State_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|Stim_State_Pin|ADC_Output_State_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : IDLE_State_Pin */
  GPIO_InitStruct.Pin = IDLE_State_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IDLE_State_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SCOPE_Pin_Pin */
  GPIO_InitStruct.Pin = SCOPE_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SCOPE_Pin_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	//HAL_GPIO_TogglePin(SCOPE_Pin_GPIO_Port, SCOPE_Pin_Pin);
	gADC_reading = HAL_ADC_GetValue(&hadc1);
	gADC_reading_Two = HAL_ADC_GetValue(&hadc2);
	//HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

	if(getGlobalState() == ADC_OUTPUT){
        uint32_t adc_logged = gADC_reading | (gADC_reading_Two << 16);
		circ_buff_put(meas_adc_circ_buff, adc_logged);
	}
	else if(getGlobalState() == STIM_CONTROL){
	    circ_buff_put(stim_adc_circ_buff , gADC_reading);

	    //These series of if's should be inside a "should_diagnostic_pulse_be_produced()" function.
	    if(has_threshold_been_crossed())
	    {
		    if(((num_of_threshold_crossings+1)%STIM_TRIGGER_CYCLE_LIMIT==0))
		    {
		    	//if diagnostic pulse timer has not expired we don't send another pulse for safety
		    	uint32_t expiration_time = get_time_of_last_diagnostic_pulse() + DIAGNOSTIC_PULSE_TIME;
		    	if(!((int32_t)(expiration_time - HAL_GetTick()) > 0)){
		    		set_diagnostic_pulse_flag();
		    	}
		    }
		    num_of_threshold_crossings++;
		    POS_BELOW_THRESHOLD = !POS_BELOW_THRESHOLD;
	    }
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
