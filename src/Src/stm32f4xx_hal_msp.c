/* USER CODE BEGIN Header */
/**
	******************************************************************************
	* File Name					: stm32f4xx_hal_msp.c
	* Description				: This file provides code for the MSP Initialization 
	*											and de-Initialization codes.
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
	*	 1. Redistributions of source code must retain the above copyright notice,
	*			this list of conditions and the following disclaimer.
	*	 2. Redistributions in binary form must reproduce the above copyright notice,
	*			this list of conditions and the following disclaimer in the documentation
	*			and/or other materials provided with the distribution.
	*	 3. Neither the name of STMicroelectronics nor the names of its contributors
	*			may be used to endorse or promote products derived from this software
	*			without specific prior written permission.
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
#include "BSP_globalfunctions.h"

										/**
	* Initializes the Global MSP.
	*/
void HAL_MspInit(void) {
	/* USER CODE BEGIN MspInit 0 */

	/* USER CODE END MspInit 0 */

	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

	/* System interrupt init*/

	/* USER CODE BEGIN MspInit 1 */

	/* USER CODE END MspInit 1 */
}

/* USER CODE BEGIN 1 */
void SystemClock_Config(void);

void HAL_HardwareInit() {
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
	
	GlobalFunctions_Init();
	UART_init();
	GLCD_init(128, 64);
	
	ADC_Init();
	DAC_Init();
	
	UIIO_init();
	
	
	/*
	int8_t x, y;
	int8_t n = 0xAA;
	int8_t btnVal = 0;
	while (1) {
		if (n != (TIM3->CNT & 0xFF)) {
			/ *
			for (y = 0; y < 8; y++) {
				GLCD_Write(GLCD_CS_12, GLCD_DI_COMMAND, GLCD_REG_COLUMN_ADDR);
				GLCD_Write(GLCD_CS_12, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | y));
				for (x = 0; x < 64; x ++) {
					GLCD_Write(GLCD_CS_12, GLCD_DI_DATA, x);
				}
			}
			* /
			for (y = 0; y < 8; y ++) {
				GLCD_Write(GLCD_CS_1, GLCD_DI_COMMAND, GLCD_REG_COLUMN_ADDR);
				GLCD_Write(GLCD_CS_1, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | y));
				for (x = 0; x < 64; x ++) {
					GLCD_Write(GLCD_CS_1, GLCD_DI_DATA, n); // ((x & 1) ? (~n) : n)
				}
				GLCD_Write(GLCD_CS_2, GLCD_DI_COMMAND, GLCD_REG_COLUMN_ADDR);
				GLCD_Write(GLCD_CS_2, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | y));
				for (x = 0; x < 64; x ++) {
					GLCD_Write(GLCD_CS_2, GLCD_DI_DATA, (n + x)); // ((x & 1) ? (~n) : n)
				}
			}
			n = (TIM3->CNT & 0xFF);
		}
		// HAL_GPIO_WritePin(GPIOB, LED_R_Pin, (TIM3->CNT & 0x01));
		btnVal = HAL_GPIO_ReadPin(BTN_MAIN_GPIO_Port, BTN_MAIN_Pin);
		btnVal |= HAL_GPIO_ReadPin(BTN_1_GPIO_Port, BTN_1_Pin);
		btnVal |= HAL_GPIO_ReadPin(BTN_2_GPIO_Port, BTN_2_Pin);
		btnVal |= HAL_GPIO_ReadPin(BTN_3_GPIO_Port, BTN_3_Pin);
		btnVal |= HAL_GPIO_ReadPin(BTN_4_GPIO_Port, BTN_4_Pin);
		HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, btnVal);
		
		HAL_GPIO_WritePin(IN2_AC_GPIO_Port, IN2_AC_Pin, HAL_GPIO_ReadPin(BTN_1_GPIO_Port, BTN_1_Pin));
		HAL_GPIO_WritePin(IN1_AC_GPIO_Port, IN1_AC_Pin, HAL_GPIO_ReadPin(BTN_2_GPIO_Port, BTN_2_Pin));
		HAL_GPIO_WritePin(IN1_AA_GPIO_Port, IN1_AA_Pin, HAL_GPIO_ReadPin(BTN_3_GPIO_Port, BTN_3_Pin));
		HAL_GPIO_WritePin(IN2_AA_GPIO_Port, IN2_AA_Pin, HAL_GPIO_ReadPin(BTN_4_GPIO_Port, BTN_4_Pin));
		
		HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
		//HAL_Delay(500);
	}
	*/
	/* USER CODE BEGIN 2 */
	
	/* USER CODE END 2 */
}



/**
	* @brief System Clock Configuration
	* @retval None
	*/
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/**Configure the main internal regulator output voltage  */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/**Initializes the CPU, AHB and APB busses clocks  */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/**Initializes the CPU, AHB and APB busses clocks  */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
	* @brief	This function is executed in case of error occurrence.
	* @retval None
	*/
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__asm__ __volatile__ ("bkpt #0");
	/* USER CODE END Error_Handler_Debug */
}

#ifdef	USE_FULL_ASSERT
/**
	* @brief	Reports the name of the source file and the source line number
	*				 where the assert_param error has occurred.
	* @param	file: pointer to the source file name
	* @param	line: assert_param error line source number
	* @retval None
	*/
void assert_failed(uint8_t *file, uint32_t line) { 
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
		 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */



/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
