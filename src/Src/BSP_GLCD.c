
#include "BSP_GLCD.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>


volatile int32_t sys_delay = 0; // decremented by 1 every us if larger than zero

void Sys_DelayUs(int32_t us) { // blocking delay
	sys_delay = us;
	while(sys_delay);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance == TIM4) {
		if (sys_delay > 0) {
			sys_delay--;
		}
	}
}

void TIM4_IRQHandler(void) {
	HAL_TIM_IRQHandler(&htim4);
}

// TODO: fix & write tests for graphics functions
// TODO: separate graphics functions, STM32 and SBN6400G-D + SBN0064G-D code

void GLCD_init(uint32_t w, uint32_t h) {
	MonoGFXDisplay.width = w;
	MonoGFXDisplay.height = h;
	MonoGFXDisplay.mode = MonoGFX_DISPLAY_MODE_VERTICAL;
	MonoGFX_init(&MonoGFXDisplay);
	
	MX_GLCD_GPIO_Init();
	MX_TIM8_Init();
	MX_TIM4_Init();
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
	
	GLCDEN(1);
	
	// Init GLCD functions
	HAL_GPIO_WritePin(GLCD_E_Port, GLCD_E_Pin, GPIO_PIN_RESET);			// GLCD_E = 0
	HAL_GPIO_WritePin(GLCD_RST_Port, GLCD_RST_Pin, GPIO_PIN_SET);	// GLCD_RESET = 1
	
	GLCD_Write(GLCD_CS_12, GLCD_DI_COMMAND, GLCD_REG_DISPLAY_STATE | GLCD_DISPLAY_STATE_ON);	// GLCD on
	GLCD_Write(GLCD_CS_12, GLCD_DI_COMMAND, GLCD_REG_START_LINE | (0 & GLCD_START_LINE_MASK));	// GLCD Start Line
}

void GLCD_update() {
	int8_t x, y;
	uint32_t pos = 0;
	for (y = 0; y < 8; y ++) {
		GLCD_Write(GLCD_CS_1, GLCD_DI_COMMAND, GLCD_REG_COLUMN_ADDR); // set column to 0
		GLCD_Write(GLCD_CS_1, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | y)); // send row number
		for (x = 0; x < 64; x ++) {
			GLCD_Write(GLCD_CS_1, GLCD_DI_DATA, MonoGFXDisplay.buffer[pos]);
			pos++;
		}
		GLCD_Write(GLCD_CS_2, GLCD_DI_COMMAND, GLCD_REG_COLUMN_ADDR);
		GLCD_Write(GLCD_CS_2, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | y));
		for (x = 0; x < 64; x ++) {
			GLCD_Write(GLCD_CS_2, GLCD_DI_DATA, MonoGFXDisplay.buffer[pos]);
			pos++;
		}
	}
}



void GLCD_setBacklight(float d) {
	__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint32_t)(d * (float)0xFFFF));
}
void GLCDEN(int newState) {
	if (newState) {
		HAL_GPIO_WritePin(GLCD_EN_Port, GLCD_EN_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(GLCD_EN_Port, GLCD_EN_Pin, GPIO_PIN_SET);
	}
}
/**
* 	cs_s, 1 = CS1, 2 = CS2, 3 = CS1&CS2
* 	d_i, 0 = instruction, 1 = data
*/
void GLCD_Write(int8_t cs_s, int8_t d_i, int8_t g_data) {
	uint16_t data = 0x0000;
	switch (cs_s) {
		case GLCD_CS_1:
			HAL_GPIO_WritePin(GLCD_CS1_Port, GLCD_CS1_Pin, GPIO_PIN_SET);
			break;
		case GLCD_CS_2:
			HAL_GPIO_WritePin(GLCD_CS2_Port, GLCD_CS2_Pin, GPIO_PIN_SET);
			break;
		case GLCD_CS_12:
			HAL_GPIO_WritePin(GLCD_CS1_Port, GLCD_CS1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GLCD_CS2_Port, GLCD_CS2_Pin, GPIO_PIN_SET);
			break;
	}
	switch (d_i) {
		case GLCD_DI_COMMAND:
			HAL_GPIO_WritePin(GLCD_DI_Port, GLCD_DI_Pin, GPIO_PIN_RESET);
			break;
		case GLCD_DI_DATA:
			HAL_GPIO_WritePin(GLCD_DI_Port, GLCD_DI_Pin, GPIO_PIN_SET);
			break;
	}

	data = GLCD_DATA_Port->IDR;
	data &= 0xFF00;
	data |= g_data & 0x00FF;
	GLCD_DATA_Port->ODR = data;
	
	Sys_DelayUs(1);
	HAL_GPIO_WritePin(GLCD_E_Port, GLCD_E_Pin, GPIO_PIN_SET);
	Sys_DelayUs(2);
	HAL_GPIO_WritePin(GLCD_E_Port, GLCD_E_Pin, GPIO_PIN_RESET);
	Sys_DelayUs(4);
	HAL_GPIO_WritePin(GLCD_CS1_Port, GLCD_CS1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GLCD_CS2_Port, GLCD_CS2_Pin, GPIO_PIN_RESET);
}

/**
 * 		m_data: adott metszet rajzolata in hex format
 * 		cX: sor (0-7)
 * 		cY: oszlop (0-127) */
void GLCD_Write_Block(int8_t m_data, int8_t cY, int8_t cX) {
	int8_t chip = GLCD_CS_1;
	if (cX >= 64) {
		chip = GLCD_CS_2;
		cX -= 64;
	}
	GLCD_Write(chip, GLCD_DI_COMMAND, (GLCD_REG_COLUMN_ADDR | cX));
	GLCD_Write(chip, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | cY));
	GLCD_Write(chip, GLCD_DI_DATA, m_data);
}


/**
* @brief TIM_PWM MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_pwm: TIM_PWM handle pointer
* @retval None
*/
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	if(htim_pwm->Instance==TIM8) {
		/* Peripheral clock enable */
		__HAL_RCC_TIM8_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**TIM8 GPIO Configuration
		PC8		 ------> TIM8_CH3 
		*/
		GPIO_InitStruct.Pin = GLCD_BACKLIGHT_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
		HAL_GPIO_Init(GLCD_BACKLIGHT_Port, &GPIO_InitStruct);
	}

}

/**
* @brief TIM_PWM MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_pwm: TIM_PWM handle pointer
* @retval None
*/

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm) {

	if(htim_pwm->Instance==TIM8) {
	/* USER CODE BEGIN TIM8_MspDeInit 0 */

	/* USER CODE END TIM8_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM8_CLK_DISABLE();
	/* USER CODE BEGIN TIM8_MspDeInit 1 */

	/* USER CODE END TIM8_MspDeInit 1 */
	}

}

void MX_TIM4_Init(void) {
	__TIM4_CLK_ENABLE();
	
	htim4.Instance = TIM4;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.CounterMode = TIM_COUNTERMODE_DOWN;
	htim4.Init.Prescaler = 1;
	htim4.Init.Period = 180; // 180 value to measure 1us
	htim4.State = HAL_TIM_STATE_RESET;
	HAL_TIM_Base_Init(&htim4);
	HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
	HAL_TIM_Base_Start_IT(&htim4);
}

/**
	* @brief TIM8 Initialization Function
	* @param None
	* @retval None
	*/
void MX_TIM8_Init(void) {

	/* USER CODE BEGIN TIM8_Init 0 */

	/* USER CODE END TIM8_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

	/* USER CODE BEGIN TIM8_Init 1 */

	/* USER CODE END TIM8_Init 1 */
	htim8.Instance = TIM8;
	htim8.Init.Prescaler = 0;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim8.Init.Period = 0xFFFF;
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim8.Init.RepetitionCounter = 0;
	if (HAL_TIM_PWM_Init(&htim8) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0x7FFFU;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM8_Init 2 */

	/* USER CODE END TIM8_Init 2 */
}

/**
	* @brief GPIO Initialization Function
	* @param None
	* @retval None
	*/
void MX_GLCD_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GLCD_DB0_Pin|GLCD_DB1_Pin|GLCD_DB2_Pin|GLCD_DB3_Pin 
							|GLCD_DB4_Pin|GLCD_DB5_Pin|GLCD_DB6_Pin|GLCD_DB7_Pin 
							|GLCD_E_Pin|GLCD_EN_Pin|GLCD_RST_Pin|GLCD_CS1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GLCD_DI_Pin|GLCD_RW_Pin|GLCD_CS2_Pin, GPIO_PIN_RESET);
	
	/*Configure GPIO pins : GLCD_DB0_Pin GLCD_DB1_Pin GLCD_DB2_Pin GLCD_DB3_Pin 
													 GLCD_DB4_Pin GLCD_DB5_Pin GLCD_DB6_Pin GLCD_DB7_Pin 
													 GLCD_E_Pin GLCD_EN_Pin GLCD_RST_Pin GLCD_CS1_Pin */
	GPIO_InitStruct.Pin = GLCD_DB0_Pin|GLCD_DB1_Pin|GLCD_DB2_Pin|GLCD_DB3_Pin 
						 |GLCD_DB4_Pin|GLCD_DB5_Pin|GLCD_DB6_Pin|GLCD_DB7_Pin 
						 |GLCD_E_Pin|GLCD_EN_Pin|GLCD_RST_Pin|GLCD_CS1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : GLCD_DI_Pin GLCD_RW_Pin GLCD_CS2_Pin */
	GPIO_InitStruct.Pin = GLCD_DI_Pin|GLCD_RW_Pin|GLCD_CS2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
