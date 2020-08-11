/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdbool.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

static uint8_t char_buf[512];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

extern int cic_main(void);

static void SPI_Transmit(UART_HandleTypeDef *handle, uint8_t *data_out, size_t size, size_t delay)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
  //HAL_Delay(100);
  HAL_SPI_Transmit(handle, data_out, size, delay);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

static uint8_t *SPI_TransmitReceive(UART_HandleTypeDef *handle, uint8_t *data_out, uint8_t * data_in, size_t size, size_t delay)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
  //HAL_Delay(100);
  HAL_SPI_TransmitReceive(handle, data_out, data_in, size, delay);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

static uint8_t *SPI_Receive(UART_HandleTypeDef *handle, uint8_t * data_in, size_t size, size_t delay)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
  //HAL_Delay(100);
  HAL_SPI_Receive(handle, data_in, size, delay);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void process_input(uint8_t do_print)
{
  uint8_t *data_ptr = &char_buf[0];
  while(true) 
  {
    if(HAL_OK == HAL_UART_Receive(&huart1, data_ptr, sizeof(uint8_t), HAL_MAX_DELAY))
    {
      char c = *data_ptr;
      if(*data_ptr == 0xc)
      {
        c = '\b';
        if(do_print)
          HAL_UART_Transmit(&huart1, &c, sizeof(char), HAL_MAX_DELAY);
        continue;  
      }
      else if(*data_ptr == '\n' || *data_ptr == '\r')
      {
        c = '\n';
        if(do_print)
          HAL_UART_Transmit(&huart1, "\r\n", sizeof(char)*2, HAL_MAX_DELAY);
        return;
      }
      if(do_print)
        HAL_UART_Transmit(&huart1, &c, sizeof(char), HAL_MAX_DELAY);
      data_ptr += 1;
    }
  }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define STR_BUFFER "Failed to sprintf\r\n"

char ReadBit(void)
{
  unsigned char res;

  // wait for DCLK to go low
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET)
  { }

  // Read the data bit
  res = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

  // wait for DCLK to go high
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET)
  { }

  return res;
}

void WriteBit(unsigned char b)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // wait for DCLK to go low
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET)
  { }

  if (b == 0)
  {
    // drive the output low
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
  }

  // wait for DCLK to go high
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET)
  { }

  // tristate the output
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  uint8_t data_out = 0;
  uint8_t data_in = 0;
  uint32_t data_size = 0;
  uint8_t do_print = 1;

  // Turn on the LED
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);

  HAL_Delay(1000);


#if 1
  data_out = 1;
  SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
  data_out = 0;
  SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
  data_out = 0;
  SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
  data_out = 0;
  SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
  data_out = 0x81;
  SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
  data_out = 0x82;
  SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint16_t result = 0;
    uint8_t status = 0xFF;
    data_in = 0;

    if(do_print)
      HAL_UART_Transmit(&huart1, "Enter command: ", sizeof("Enter command: "), HAL_MAX_DELAY);
    else
      HAL_UART_Transmit(&huart1, "D", 1, HAL_MAX_DELAY);
    process_input(do_print);
    int cmd = 0;
    int addr = 0;
    int data = 0;
    sscanf(char_buf, "%d %d %d", &cmd, &addr, &data);

    if(cmd == 1)
    {
        data_out = 1;
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
        data_out = (addr & 0xFF0000) >> 16;
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
        data_out = (addr & 0x00FF00) >> 8;
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
        data_out = (addr & 0x0000FF);
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
        data_out = (data & 0xFF00) >> 8;
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
        data_out = (data & 0x00FF);
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
    } else if(cmd == 2)
    {
        data_out = 2;
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
        data_out = (addr & 0xFF0000) >> 16;
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
        data_out = (addr & 0x00FF00) >> 8;
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
        data_out = (addr & 0x0000FF);
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);

        while(true)
        {
          data_out = 4;
          SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
          data_in = 0;
          SPI_Receive(&hspi1, &data_in, sizeof(uint8_t), HAL_MAX_DELAY);
          if(data_in == 0x01)
          {
            break;
          }
          else 
          {
              data_size = sprintf(&char_buf, "Got delay 0x%X\r\n", (int)data_in);
              if(data_size > 0)
                HAL_UART_Transmit(&huart1, char_buf, data_size, HAL_MAX_DELAY);
              else
                HAL_UART_Transmit(&huart1, STR_BUFFER, sizeof(STR_BUFFER), HAL_MAX_DELAY);
          }
        }
        result = 0;
        data_out = 3;
        SPI_Transmit(&hspi1, &data_out, sizeof(uint8_t), HAL_MAX_DELAY);
        data_in = 0;
        SPI_Receive(&hspi1, &data_in, sizeof(uint8_t), HAL_MAX_DELAY);
        result = data_in << 8;
        data_in = 0;
        SPI_Receive(&hspi1, &data_in, sizeof(uint8_t), HAL_MAX_DELAY);
        result |= data_in;

        data_size = sprintf(&char_buf, "Read 0x%X\r\n", (uint32_t)result);
        if(data_size > 0)
          HAL_UART_Transmit(&huart1, char_buf, data_size, HAL_MAX_DELAY);
        else
          HAL_UART_Transmit(&huart1, STR_BUFFER, sizeof(STR_BUFFER), HAL_MAX_DELAY);
    }
    else if(cmd == 3)
    {
      cic_main();
    }
    else if(cmd == 4)
    {
      do_print = !do_print;
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

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_1);
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
