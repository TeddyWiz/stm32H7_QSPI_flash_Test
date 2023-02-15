/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w25q_mem.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

OSPI_HandleTypeDef hospi1;
MDMA_HandleTypeDef hmdma_octospi1_fifo_th;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
OSPI_RegularCmdTypeDef com;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_MDMA_Init(void);
static void MX_OCTOSPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
uint8_t rxData[2];

int _write(int fd, char *str, int len)
{
	for(int i=0; i<len; i++)
	{
		HAL_UART_Transmit(&huart3, (uint8_t *)&str[i], 1, 0xFFFF);
	}
	return len;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

    /*
        This will be called once data is received successfully,
        via interrupts.
    */

     /*
       loop back received data
     */
     HAL_UART_Receive_IT(&huart3, rxData, 1);
     HAL_UART_Transmit(&huart3, rxData, 1, 1000);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t rxData;
  W25Q_STATE res_W25Q;
  uint8_t buf[20] = {0x01, 0x02 ,0x03, 0x04, 0x00 };
  uint8_t temp_buf[2] = {0x00, 0x00};
  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_MDMA_Init();
  MX_OCTOSPI1_Init();
  MX_USART3_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  //SCB_CleanDCache();
  //SCB_CleanInvalidateDCache();
  //SCB_InvalidateICache
  HAL_UART_Receive_IT(&huart3, rxData, 1);
  printf("hello world !! \r\n");
  // instruction test
#if 0
  com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
  com.Instruction = 0xAB;	 // Command
  com.AddressSize = HAL_OSPI_ADDRESS_16_BITS;//HAL_OSPI_ADDRESS_24_BITS;
  com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;//HAL_OSPI_ADDRESS_4_LINES;//QSPI_ADDRESS_1_LINE;
  com.Address = 0x00000002;

  com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  com.AlternateBytes = HAL_OSPI_ALTERNATE_BYTES_NONE;
  com.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_NONE;

  com.DummyCycles = 0;
  com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
  com.NbData = 4;

  com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
  //com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
  if (HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
		!= HAL_OK)
	return W25Q_SPI_ERR;
  printf("qspi command !! \r\n");
  if (HAL_OSPI_Transmit(&hospi1, buf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
		!= HAL_OK)
	return W25Q_SPI_ERR;
  printf("test complete !! \r\n");
#endif
#if 1
    com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
    com.Instruction = 0xAB;    // Command
    com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
    com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;//HAL_OSPI_ADDRESS_4_LINES;//QSPI_ADDRESS_1_LINE;
    com.Address = 0x00000000;
  
    com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    com.AlternateBytes = HAL_OSPI_ALTERNATE_BYTES_NONE;
    com.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
    com.DummyCycles = 0;
    com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
    com.NbData = 1;
  
    com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
    //com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    if (HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
          != HAL_OK)
      return W25Q_SPI_ERR;
    printf("qspi command !! \r\n");
    if (HAL_OSPI_Receive(&hospi1, temp_buf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		printf("[%s > %s : %d]Recv Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
		return W25Q_SPI_ERR;
	}
    printf("ID: %x \r\n", temp_buf[0]);
    temp_buf[0] = 0;
    printf("test complete !! \r\n");
    if (HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
          != HAL_OK)
      return W25Q_SPI_ERR;
    printf("qspi command !! \r\n");
    if (HAL_OSPI_Receive(&hospi1, temp_buf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		printf("[%s > %s : %d]Recv Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
		return W25Q_SPI_ERR;
	}
    printf("ID 2: %x \r\n", temp_buf[0]);
    W25Q_ReadID(temp_buf);
    printf("ID 3: %x \r\n", temp_buf[0]);
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
#if 1
  res_W25Q = W25Q_Init();		 // init the chip
  printf("Init RES = %d \r\n", (uint8_t)res_W25Q);
	res_W25Q = W25Q_EraseSector(0); // erase 4K sector - required before recording
  printf("W25Q_EraseSector RES = %d \r\n", (uint8_t)res_W25Q);

	// make test data
	uint8_t byte = 0x65;
	uint8_t byte_read = 0;
	uint8_t in_page_shift = 0;
	uint8_t page_number = 0;
	// write data
	printf("input byte : %02X\r\n", byte);
	res_W25Q = W25Q_ProgramByte(byte, in_page_shift, page_number);
  printf("W25Q_ProgramByte RES = %d \r\n", (uint8_t)res_W25Q);
	// read data
	res_W25Q = W25Q_ReadByte(&byte_read, in_page_shift, page_number);
  printf("W25Q_ReadByte RES = %d \r\n", (uint8_t)res_W25Q);
	printf("output byte : %02X\r\n", byte_read);
#if 1
	// make example structure
	struct STR {
		uint8_t abc;
		uint32_t bca;
		char str[4];
		float gg;
	}_str,_str2;

	// fill instance
	_str.abc = 0x20;
	_str.bca = 0x3F3F4A;
	_str.str[0] = 'a';
	_str.str[1] = 'b';
	_str.str[2] = 'c';
	_str.str[3] = '\0';
	_str.gg = 12.658;

	uint16_t len = sizeof(_str);	// length of structure in bytes
	printf("input data \r\n abc : 0x%02X\r\n dca : 0x%06X\r\n str : %s\r\n gg : %f|%04X\r\n", _str.abc, _str.bca, _str.str, _str.gg, _str.gg);
	// program structure
	W25Q_ProgramData((uint8_t*) &_str, len, ++in_page_shift, page_number);
	// read structure to another instance
	//HAL_Delay(20);
	W25Q_ReadData((uint8_t*) &_str2, len, in_page_shift, page_number);
	printf("output data \r\n abc : 0x%02X\r\n dca : 0x%06X\r\n str : %s\r\n gg : %f|%04X\r\n", _str2.abc, _str2.bca, _str2.str, _str2.gg, _str2.gg);
  #endif
#endif
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 275;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 15;
  RCC_OscInitStruct.PLL.PLLR = 4;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief OCTOSPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OCTOSPI1_Init(void)
{

  /* USER CODE BEGIN OCTOSPI1_Init 0 */

  /* USER CODE END OCTOSPI1_Init 0 */

  OSPIM_CfgTypeDef sOspiManagerCfg = {0};

  /* USER CODE BEGIN OCTOSPI1_Init 1 */

  /* USER CODE END OCTOSPI1_Init 1 */
  /* OCTOSPI1 parameter configuration*/
  hospi1.Instance = OCTOSPI1;
  hospi1.Init.FifoThreshold = 1;
  hospi1.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
  hospi1.Init.MemoryType = HAL_OSPI_MEMTYPE_MICRON;
  hospi1.Init.DeviceSize = 17;
  hospi1.Init.ChipSelectHighTime = 2;
  hospi1.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi1.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
  hospi1.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi1.Init.ClockPrescaler = 1;
  hospi1.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  hospi1.Init.ChipSelectBoundary = 0;
  hospi1.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED;
  hospi1.Init.MaxTran = 0;
  hospi1.Init.Refresh = 0;
  if (HAL_OSPI_Init(&hospi1) != HAL_OK)
  {
    Error_Handler();
  }
  sOspiManagerCfg.ClkPort = 1;
  sOspiManagerCfg.NCSPort = 1;
  sOspiManagerCfg.IOLowPort = HAL_OSPIM_IOPORT_1_LOW;
  if (HAL_OSPIM_Config(&hospi1, &sOspiManagerCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OCTOSPI1_Init 2 */

  /* USER CODE END OCTOSPI1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable MDMA controller clock
  */
static void MX_MDMA_Init(void)
{

  /* MDMA controller clock enable */
  __HAL_RCC_MDMA_CLK_ENABLE();
  /* Local variables */

  /* MDMA interrupt initialization */
  /* MDMA_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(MDMA_IRQn);

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin|LED_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_FS_PWR_EN_GPIO_Port, USB_FS_PWR_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_GREEN_Pin LED_RED_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin|LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_FS_PWR_EN_Pin */
  GPIO_InitStruct.Pin = USB_FS_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_FS_PWR_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_FS_OVCR_Pin */
  GPIO_InitStruct.Pin = USB_FS_OVCR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_FS_OVCR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_YELLOW_Pin */
  GPIO_InitStruct.Pin = LED_YELLOW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_YELLOW_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
