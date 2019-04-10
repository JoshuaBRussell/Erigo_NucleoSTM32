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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STIM_TRIGGER_THRESHOLD 2000
#define STIM_TRIGGER_TOLERANCE 50
#define STIM_TRIGGER_CYCLE_LIMIT 5 //Number of times the threshold must be reached before Test Pulse is produced.

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

#define SERIAL_MESSAGE_SIZE 10//Includes start/end/delimiter bytes also

#define MILLIAMP_TO_DAC_CONV_FACTOR 12.4

#define ADC_BUFFER_SIZE 5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DAC_HandleTypeDef hdac;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
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
enum WF_STATE STIM_STATE = STIM_FREQ_TRIGGER_LOW;

struct min_context min_ctx;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
bool TEST_FLAG = false;
uint32_t Num_of_Threshold_Counts = 0;

uint16_t T_LOW = 0; //Initially based on freq.
uint16_t T_PERIOD = 0;

uint16_t Test_Amplitude_in_Counts = 0;
uint16_t NM_Amplitude_in_Counts = 0;
uint16_t Freq_Sel_in_Counts = 0;

uint32_t gADC_reading = 0;

//ADC Buffer
uint32_t adc_buffer_array[ADC_BUFFER_SIZE] = {0};
//Declare circular buffer for position ADC values
circ_buff_handle position_adc_meas;

bool POS_BELOW_THRESHOLD = false;

bool should_test_pulse_be_produced(){
	//STIM_TRIGGER_THRESHOLD +/- STIM_TRIGGER_TOLERANCE implements hysteresis
	return (POS_BELOW_THRESHOLD&&all_above_threshold(adc_buffer_array, ADC_BUFFER_SIZE, STIM_TRIGGER_THRESHOLD + STIM_TRIGGER_TOLERANCE)) || (!POS_BELOW_THRESHOLD&&all_below_threshold(adc_buffer_array, ADC_BUFFER_SIZE, STIM_TRIGGER_THRESHOLD - STIM_TRIGGER_TOLERANCE));
}

//This is only called at the onset of the program a couple of times. Otherwise the
//cast and float multiplicatin would be considered "not pretty."
void milliamps_to_DAC_counts(const uint16_t in_milliamp, uint16_t* dac_counts){
    *dac_counts =  (uint16_t)(MILLIAMP_TO_DAC_CONV_FACTOR * (float)in_milliamp);
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
  /* USER CODE BEGIN 2 */
//  while(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)){
//
//  }

  //Init of MIN CTX//
  min_init_context(&min_ctx, 0);


  uint16_t test_amp_ma, nm_amp_ma, freq_sel;
  while(!is_comm_success())
  {
	  comm_get_control_params();
  }

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


  //Define the circular buffer
  position_adc_meas = circ_buff_init(adc_buffer_array, ADC_BUFFER_SIZE);

  //Set initial value of stim amplitude.
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, NM_Amplitude_in_Counts);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_ADC_Start_IT(&hadc1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

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
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SCOPE_Pin_GPIO_Port, SCOPE_Pin_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SCOPE_Pin_Pin */
  GPIO_InitStruct.Pin = SCOPE_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SCOPE_Pin_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//----ISR Callbacks----//
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

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	gADC_reading = HAL_ADC_GetValue(&hadc1);
	//HAL_GPIO_TogglePin(SCOPE_Pin_GPIO_Port, SCOPE_Pin_Pin);

	circ_buff_put(position_adc_meas , gADC_reading);
	if(should_test_pulse_be_produced())
	{
		if(((Num_of_Threshold_Counts+1)%STIM_TRIGGER_CYCLE_LIMIT==0))
		{
			TEST_FLAG = true;
		}
		Num_of_Threshold_Counts++;
		POS_BELOW_THRESHOLD = !POS_BELOW_THRESHOLD;
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
