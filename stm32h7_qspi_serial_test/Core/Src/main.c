/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
typedef struct QSPI_Set_Data_t
{
    char mode;
    int dummy;
    int instruction;
    int addr;
}QSPI_Set_Data;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

OSPI_HandleTypeDef hospi1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart9;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
OSPI_RegularCmdTypeDef com;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_OCTOSPI1_Init(void);
static void MX_RTC_Init(void);
static void MX_UART9_Init(void);
/* USER CODE BEGIN PFP */
uint8_t rxData;
uint8_t rx_buffer[2048]= {0,};
int rx_index = 0;
uint8_t rx_flag =0;
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
     HAL_UART_Receive_IT(&huart3, &rxData, 1);
     HAL_UART_Transmit(&huart3, &rxData, 1, 10);
     //rx_buffer[rx_index++] = rxData;
     if(rxData == '\n')
    {
        rx_flag = 1;
        rx_buffer[rx_index] = 0;
    }
     else
     {
    	 rx_buffer[rx_index++] = rxData;
     }
     
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char Hex2Char(char const* szHex, unsigned char *rch)
{
    if(*szHex >= '0' && *szHex <= '9')
            *rch = *szHex - '0';
    else if(*szHex >= 'A' && *szHex <= 'F')
            *rch = *szHex - 55; //-'A' + 10
    else if(*szHex >= 'a' && *szHex <= 'f')
            *rch = *szHex - 87; //-'a' + 10
    else
        //Is not really a Hex string
        return 1;
    szHex++;
    if(*szHex >= '0' && *szHex <= '9')
        *rch = (*rch << 4) | *szHex - '0';
    else if(*szHex >= 'A' && *szHex <= 'F')
        *rch = (*rch << 4) | *szHex - 55; //-'A' + 10;
    else if(*szHex >= 'a' && *szHex <= 'f')
        *rch = (*rch << 4) | *szHex - 87; //-'a' + 10
    else
        //Is not really a Hex string
        return 2;
    return 0;
}

char qspi_set_parse(char *r_data, QSPI_Set_Data *init_data)
{
    int seq = 0;
    int temp = 0;
    char ret = 0;
    unsigned char tempHex = 0;
    char *ptr = strtok(r_data, ",");
    while(ptr != NULL)
    {
        switch(seq)
        {
            case 0: //spi format mode
                seq++;
                printf("spi mode : %c \r\n", *ptr);
                if((*ptr=='s')||(*ptr=='d')||(*ptr=='q'))
                {
                    init_data->mode = *ptr;
                }
                else
                {
                    printf("parse spi mode error \r\n");
                    return 1;
                }
                break;
            case 1: //dummy cycle
                seq++;
                temp = atoi(ptr);
                printf("dumy cycle : %s, %d\r\n", ptr, temp);
                if((temp < 0)||(temp > 31))
                {
                    printf("dummy cycle value error \r\n");
                    return 2;
                }
                init_data->dummy = temp;
                break;
            case 2: //Instruction data
                seq++;
                temp = strlen(ptr);
                //printf("instruction data : %s[%d]\r\n", ptr, temp);
                ret = Hex2Char(ptr, &tempHex);
                if(ret != 0)
                {
                    printf("instruction data value error %02X \r\n", tempHex);
                    return 3;
                }
                printf("instruction data : %02X \r\n", tempHex);
                init_data->instruction = tempHex;
                break;
            case 3: //Address data
                seq++;
                temp = strlen(ptr);
                //printf("Address data : %s[%d]\r\n", ptr, temp);
                ret = Hex2Char(ptr, &tempHex);
                if(ret != 0)
                {
                    printf("address data value error %02X \r\n", tempHex);
                    return 3;
                }
                temp = (int)tempHex;
                ret = Hex2Char(ptr+2, &tempHex);
                if(ret != 0)
                {
                    printf("address data value error %02X \r\n", tempHex);
                    return 3;
                }
                temp = temp << 8 | tempHex;
                printf("address data : %04X \r\n", temp);
                init_data->addr = temp;
                break;
        }
        ptr = strtok(NULL, ",");
    }
    return 0;
}
char qspi_set_parameter(QSPI_Set_Data *init_data)
{
    switch(init_data->mode)
    {
        case 's':
            com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
            com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
            break;
        case 'd':
            com.AddressMode = HAL_OSPI_ADDRESS_2_LINES;
            com.DataMode = HAL_OSPI_DATA_2_LINES;
            break;
        case 'q':
            com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
            com.DataMode = HAL_OSPI_DATA_4_LINES;
            break;
    }
    com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
    com.Instruction = init_data->instruction;//0xAB;    // Command
    com.AddressSize = HAL_OSPI_ADDRESS_16_BITS;
    //com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;//HAL_OSPI_ADDRESS_4_LINES;//QSPI_ADDRESS_1_LINE;
    com.Address = init_data->addr;//0x00000000;
  
    com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    com.AlternateBytes = HAL_OSPI_ALTERNATE_BYTES_NONE;
    com.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
    com.DummyCycles = init_data->dummy;
    //com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
    //com.NbData = 1;
  
    com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
    //com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    return 0;
}
char send_spi_data(int len, char *data)
{
    com.NbData = len;
    printf("send data[%d]:%s\r\n >",len, data);
    if (HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
        != HAL_OK)
    {
        printf("[%s > %s : %d]CMD Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
        return 1;
    }
    if (HAL_OSPI_Transmit(&hospi1, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
        != HAL_OK)
    {
        printf("[%s > %s : %d]Send Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
        return 2;
    }
    return 0;
}
char recv_spi_data(int len, char *data)
{
    com.NbData = len;
    if (HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
          != HAL_OK)
    {
        printf("[%s > %s : %d]Cmd Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
        return 1;
    }
    if (HAL_OSPI_Receive(&hospi1, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
        != HAL_OK) 
    {
        printf("[%s > %s : %d]Recv Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
        return 2;
    }
    return 0;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  //uint8_t rxData
  char *recv_data = NULL;
  char *temp_ptr = NULL;
  char *send_data = NULL;
  uint8_t buf[20] = {0x01, 0x02 ,0x03, 0x04, 0x00 };
  uint8_t temp_buf[2] = {0x00, 0x00};
  QSPI_Set_Data test_data;
  char testHex[2] = "AB";
  char tempHex = 0;
  char returnHex = 0;
  char ret;
  int len = 0;
  int temp_len = 0;
  int i = 0;
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
  MX_USART3_UART_Init();
  MX_OCTOSPI1_Init();
  MX_RTC_Init();
  MX_UART9_Init();
  /* USER CODE BEGIN 2 */
  //SCB_CleanDCache();
  //SCB_CleanInvalidateDCache();
  //SCB_InvalidateICache
  HAL_UART_Receive_IT(&huart3, &rxData, 1);
  HAL_UART_Receive_IT(&huart9, &rxData, 1);
  printf("hello world !! \r\n");
  HAL_UART_Transmit(&huart9, "hello Uart6!! \r\n", 16, 10);
  HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_RESET);
  HAL_Delay(500);
  HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_SET);
  HAL_Delay(500);
   //SPI CS Test

   HAL_GPIO_WritePin(SPI_EN_MOD2_GPIO_Port, SPI_EN_MOD2_Pin, GPIO_PIN_SET);
   HAL_Delay(500);
   HAL_GPIO_WritePin(SPI_EN_MOD2_GPIO_Port, SPI_EN_MOD2_Pin, GPIO_PIN_RESET);
   HAL_Delay(500); HAL_Delay(500);
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
#if 0
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
    	printf("[%s > %s : %d]Cmd Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
      //return W25Q_SPI_ERR;
    printf("qspi command !! \r\n");
    if (HAL_OSPI_Receive(&hospi1, temp_buf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		printf("[%s > %s : %d]Recv Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
		//return W25Q_SPI_ERR;
	}
    printf("ID: %x \r\n", temp_buf[0]);
    temp_buf[0] = 0;
    printf("test complete !! \r\n");
    if (HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
          != HAL_OK)
    	printf("[%s > %s : %d]Cmd Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
      //return W25Q_SPI_ERR;
    printf("qspi command !! \r\n");
    if (HAL_OSPI_Receive(&hospi1, temp_buf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		printf("[%s > %s : %d]Recv Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
		//return W25Q_SPI_ERR;
	}
    printf("ID 2: %x \r\n", temp_buf[0]);
#endif
#if 0
    ret = Hex2Char(testHex, &returnHex);
    printf("ret : %d, hex : %02X \r\n", ret, returnHex);
    #endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
#if 0
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
#if 0
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
  printf("CMD List\r\n");
  printf("help : show CMD list\r\n");
  printf("sett : format,dummy cycle,Instruction Data,Addr Data\r\n");
  printf("example : format=quad, dummy cycle 2, instruction AB, Addr = 1234 \r\n");
  printf(">sett q,2,AB,1234<LF>  \r\n");
  printf("send : send string data \r\n");
  printf("hexs : send hex data \r\n");
  printf("recv : recv count number \r\n");
  //printf(">>");
  HAL_UART_Transmit(&huart3, (uint8_t *)">", 1, 0xFFFF);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if(rx_flag == 1)
    {
        rx_flag = 0;
        printf("serial recv : [%d] %s \r\n>", rx_index ,rx_buffer);
        if(rx_index < 4)
        {
        //error size
        }
        else if(strncmp(rx_buffer, "sett", 4) == 0)
        {//set
            printf("set spi parameter \r\n");
            qspi_set_parse(rx_buffer+5, &test_data);
            printf("mode : %c, dummy : %d, Ins : %02x, Addr : %04x\r\n>", test_data.mode, test_data.dummy, test_data.instruction, test_data.addr);
            qspi_set_parameter(&test_data);
        }
        else if(strncmp(rx_buffer, "hexs", 4) == 0)
        {
        	len = strlen(rx_buffer + 5);
        	temp_ptr = rx_buffer + 5;
        	temp_len = len/2;//+ len%2;
        	send_data = (char*)calloc(temp_len + 1, sizeof(char));
        	//printf("hexs send\r\n");
        	for(i=0; i<temp_len; i++)
        	{
        		ret = Hex2Char(temp_ptr+2*i, &tempHex);
        		//printf("%02x ", tempHex);
        		if(ret != 0)
				{
					printf("HEX data value error %02X \r\n", tempHex);
					break;
				}
        		else
        		{
        			send_data[i] = tempHex;
        		}
        	}
        	//printf("\r\n tet=%d\r\n", ret);
			if(ret == 0)
			{
				printf("Hex send data[%d]\r\n>",temp_len);
				for(i=0; i<temp_len; i++)
				{
					printf("0x%02x ",send_data[i]);
				}
				printf("\r\n");
				send_spi_data(temp_len, send_data);
			}
			free(send_data);
        }
        else if(strncmp(rx_buffer, "send", 4) == 0)
        {//set
        	//rx_buffer[rx_index] = 0;
            len = strlen(rx_buffer + 5);
            printf("send spi data[%d]\r\n>",len);
            send_spi_data(len, rx_buffer + 5);
        }
        else if(strncmp(rx_buffer, "recv", 4) == 0)
        {//set
            len = atoi(rx_buffer + 5);
            recv_data = (char*)calloc(len + 1, sizeof(char));
            recv_spi_data(len, recv_data);
            printf("recv spi data[%d] : \r\n ",len);
            for(i= 0; i< len; i++)
            {
            	printf("%02X ", recv_data[i]);
            }
            printf("\r\n");
            free(recv_data);
            //recv funcion
        }
        else if(strncmp(rx_buffer, "help", 4) == 0)
        {//set
            printf("help : show CMD list\r\n");
            printf("sett : format,dummy cycle,Instruction Data,Addr Data\r\n");
            printf("example : format=quad, dummy cycle 2, instruction AB, Addr = 1234 \r\n");
            printf(">sett q,2,AB,1234<LF>  \r\n");
            printf("send : send string data \r\n");
		  printf("hexs : send hex data \r\n");
		  printf("recv : recv count number \r\n");
        }
        else
        {//not cmd
        	printf("do not find cmd \r\n >");
        }
        rx_index = 0;
        //printf(">>");
        HAL_UART_Transmit(&huart3, (uint8_t *)">", 1, 0xFFFF);
    }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 32;
  RCC_OscInitStruct.PLL.PLLN = 275;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 55;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief UART9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART9_Init(void)
{

  /* USER CODE BEGIN UART9_Init 0 */

  /* USER CODE END UART9_Init 0 */

  /* USER CODE BEGIN UART9_Init 1 */

  /* USER CODE END UART9_Init 1 */
  huart9.Instance = UART9;
  huart9.Init.BaudRate = 115200;
  huart9.Init.WordLength = UART_WORDLENGTH_8B;
  huart9.Init.StopBits = UART_STOPBITS_1;
  huart9.Init.Parity = UART_PARITY_NONE;
  huart9.Init.Mode = UART_MODE_TX_RX;
  huart9.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart9.Init.OverSampling = UART_OVERSAMPLING_16;
  huart9.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart9.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart9.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart9) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart9, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart9, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart9) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART9_Init 2 */

  /* USER CODE END UART9_Init 2 */

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
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_DMADISABLEONERROR_INIT;
  huart3.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin|LED_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_FS_PWR_EN_GPIO_Port, USB_FS_PWR_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_EN_MOD2_GPIO_Port, SPI_EN_MOD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RSTn_Pin */
  GPIO_InitStruct.Pin = RSTn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RSTn_GPIO_Port, &GPIO_InitStruct);

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

  /*Configure GPIO pin : SPI_EN_MOD2_Pin */
  GPIO_InitStruct.Pin = SPI_EN_MOD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI_EN_MOD2_GPIO_Port, &GPIO_InitStruct);

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
