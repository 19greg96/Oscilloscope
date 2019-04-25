
#include "BSP_UIIO.h"


static void MX_TIM3_Init(void);
static void MX_GPIO_Init(void);

UIIO_BtnTypedef UIIO_btnTable[] = {
	{.pin = BTN_MAIN_Pin, .port = BTN_MAIN_GPIO_Port, .pressTime = 0, .releaseTime = 0, .state = UIIO_BTN_STATE_RELEASED, .onPress = NULL, .onRelease = NULL, .onLongPress = NULL, .onClick = NULL},
	{.pin = BTN_1_Pin, .port = BTN_1_GPIO_Port, .pressTime = 0, .releaseTime = 0, .state = UIIO_BTN_STATE_RELEASED, .onPress = NULL, .onRelease = NULL, .onLongPress = NULL, .onClick = NULL},
	{.pin = BTN_2_Pin, .port = BTN_2_GPIO_Port, .pressTime = 0, .releaseTime = 0, .state = UIIO_BTN_STATE_RELEASED, .onPress = NULL, .onRelease = NULL, .onLongPress = NULL, .onClick = NULL},
	{.pin = BTN_3_Pin, .port = BTN_3_GPIO_Port, .pressTime = 0, .releaseTime = 0, .state = UIIO_BTN_STATE_RELEASED, .onPress = NULL, .onRelease = NULL, .onLongPress = NULL, .onClick = NULL},
	{.pin = BTN_4_Pin, .port = BTN_4_GPIO_Port, .pressTime = 0, .releaseTime = 0, .state = UIIO_BTN_STATE_RELEASED, .onPress = NULL, .onRelease = NULL, .onLongPress = NULL, .onClick = NULL},
};
uint32_t UIIO_numButtons = 5;
volatile uint32_t UIIO_scrollTime = 0;

void EXTI9_5_IRQHandler() {
	uint32_t pending = EXTI->PR;
	if (pending & BTN_1_Pin) {
		HAL_GPIO_EXTI_IRQHandler(BTN_1_Pin);
	}
	if (pending & BTN_2_Pin) {
		HAL_GPIO_EXTI_IRQHandler(BTN_2_Pin);
	}
	if (pending & BTN_3_Pin) {
		HAL_GPIO_EXTI_IRQHandler(BTN_3_Pin);
	}
	if (pending & BTN_4_Pin) {
		HAL_GPIO_EXTI_IRQHandler(BTN_4_Pin);
	}
	if (pending & GPIO_PIN_9) { // clear IT flag for non connected pin in case of false activation
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
	}
}

void EXTI4_IRQHandler() {
	HAL_GPIO_EXTI_IRQHandler(BTN_MAIN_Pin);
}

void TIM3_IRQHandler() {
	HAL_TIM_IRQHandler(&htim3);
}

UIIO_BtnTypedef* UIIO_getButton(uint32_t id) {
	return &UIIO_btnTable[id];
}
void UIIO_handlePress(UIIO_BtnTypedef* btn) {
	if (HAL_GPIO_ReadPin(btn->port, btn->pin) == GPIO_PIN_SET) {
		if (HAL_GetTick() - btn->pressTime > UIIO_DEBOUNCE_TIME_MS &&	// time passed since first press IT (press bounce)
			HAL_GetTick() - btn->releaseTime > UIIO_DEBOUNCE_TIME_MS) {	// time passed since first release IT (release bounce)
			btn->pressTime = HAL_GetTick();
			btn->state = UIIO_BTN_STATE_PRESSED;
			if (btn->onPress != NULL) {
				btn->onPress(btn); // TODO: this should not be called in ISR
			}
		}
	}
}
void UIIO_handleRelease(UIIO_BtnTypedef* btn) {
	if (HAL_GPIO_ReadPin(btn->port, btn->pin) == GPIO_PIN_RESET) {
		if (HAL_GetTick() - btn->pressTime > UIIO_DEBOUNCE_TIME_MS &&	// time passed since first press IT (press bounce)
			HAL_GetTick() - btn->releaseTime > UIIO_DEBOUNCE_TIME_MS) {	// time passed since first release IT (release bounce)
			btn->releaseTime = HAL_GetTick();
			btn->state = UIIO_BTN_STATE_RELEASED;
			if (btn->onRelease != NULL) {
				btn->onRelease(btn); // TODO: this should not be called in ISR
			}
			if (HAL_GetTick() - btn->pressTime < UIIO_LONG_PRESS_TIME_MS) {
				if (btn->onClick != NULL) { // TODO: only call click event if no scroll occurred while pressed
					btn->onClick(btn); // TODO: this should not be called in ISR
				}
			}
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	for (uint32_t i = 0; i < UIIO_numButtons; i ++) {
		if (UIIO_btnTable[i].pin == GPIO_Pin) {
			UIIO_handlePress(&UIIO_btnTable[i]);
			UIIO_handleRelease(&UIIO_btnTable[i]);
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM3) {
		UIIO_scrollTime = HAL_GetTick();
	}
}

void UIIO_update() {
	for (uint32_t i = 0; i < UIIO_numButtons; i ++) {
		if (UIIO_btnTable[i].state & UIIO_BTN_STATE_PRESSED) {
			if (HAL_GPIO_ReadPin(UIIO_btnTable[i].port, UIIO_btnTable[i].pin) == GPIO_PIN_RESET) { // in case the IT didn't record a true release event because of bounce condition
				UIIO_handleRelease(&UIIO_btnTable[i]);
			} else { // previous state is pressed and is still pressed
				if ((UIIO_btnTable[i].state & UIIO_BTN_STATE_LONG_PRESSED) == 0) { // only fire long press event once
					if (HAL_GetTick() - UIIO_btnTable[i].pressTime > UIIO_LONG_PRESS_TIME_MS) {
						UIIO_btnTable[i].state |= UIIO_BTN_STATE_LONG_PRESSED;
						if (UIIO_scrollTime > UIIO_btnTable[i].pressTime) { // TODO: di/ei
							// a scroll event has occurred after button press: don't send long press event
						} else {
							if (UIIO_btnTable[i].onLongPress != NULL) {
								UIIO_btnTable[i].onLongPress(&UIIO_btnTable[i]);
							}
						}
					}
				}
			}
		} else if (UIIO_btnTable[i].state == UIIO_BTN_STATE_RELEASED) { // last state is release
			UIIO_handlePress(&UIIO_btnTable[i]);
		}
	}
	if (UIIO_onScrollEvent != NULL) {
		if (UIIO_lastScrollCounter != TIM3->CNT) { // TODO: di/ei
			UIIO_onScrollEvent(TIM3->CNT - UIIO_lastScrollCounter);
			UIIO_lastScrollCounter = TIM3->CNT;
		}
	}
}

void UIIO_init() {
	MX_GPIO_Init();
	MX_TIM3_Init();
	
	UIIO_onScrollEvent = NULL;
	UIIO_lastScrollCounter = 0;
	
	/*
#define BTN_MAIN_Pin GPIO_PIN_4
#define BTN_1_Pin GPIO_PIN_5
#define BTN_2_Pin GPIO_PIN_6
#define BTN_3_Pin GPIO_PIN_7
#define BTN_4_Pin GPIO_PIN_8
	*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0); // BTN 1-4
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	
	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0); // BTN_MAIN
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	
	HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);
}



/**
* @brief TIM_Encoder MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_encoder: TIM_Encoder handle pointer
* @retval None
*/
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_encoder) {

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(htim_encoder->Instance==TIM3) {
	/* USER CODE BEGIN TIM3_MspInit 0 */

	/* USER CODE END TIM3_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM3_CLK_ENABLE();
	
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**TIM3 GPIO Configuration		
		PA6		 ------> TIM3_CH1
		PA7		 ------> TIM3_CH2 
		*/
		GPIO_InitStruct.Pin = ROT_ENC_A_Pin|ROT_ENC_B_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(ROT_ENC_A_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN TIM3_MspInit 1 */

	/* USER CODE END TIM3_MspInit 1 */
	}

}


/**
* @brief TIM_Encoder MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_encoder: TIM_Encoder handle pointer
* @retval None
*/

void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef* htim_encoder) {

	if(htim_encoder->Instance==TIM3) {
	/* USER CODE BEGIN TIM3_MspDeInit 0 */

	/* USER CODE END TIM3_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM3_CLK_DISABLE();
	
		/**TIM3 GPIO Configuration		
		PA6		 ------> TIM3_CH1
		PA7		 ------> TIM3_CH2 
		*/
		HAL_GPIO_DeInit(GPIOA, ROT_ENC_A_Pin|ROT_ENC_B_Pin);

	/* USER CODE BEGIN TIM3_MspDeInit 1 */

	/* USER CODE END TIM3_MspDeInit 1 */
	}

}



/**
	* @brief TIM3 Initialization Function
	* @param None
	* @retval None
	*/
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_Encoder_InitTypeDef sConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Instance->CNT = 0;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 0xFFFF;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI2;
	sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = 0x0F;
	sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = 0x0F;
	if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */
}

/**
	* @brief GPIO Initialization Function
	* @param None
	* @retval None
	*/
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();


	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, LED_USER_Pin|IN2_AC_Pin|IN1_AC_Pin|IN1_AA_Pin|IN2_AA_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LED_G_Pin|LED_R_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : BLUE_USER_BTN_Pin */
	GPIO_InitStruct.Pin = BTN_USER_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(BTN_USER_GPIO_Port, &GPIO_InitStruct);
	
	/*Configure GPIO pins : LD2_Pin IN2_AC_Pin IN1_AC_Pin IN1_AA_Pin IN2_AA_Pin */
	GPIO_InitStruct.Pin = LED_USER_Pin|IN2_AC_Pin|IN1_AC_Pin|IN1_AA_Pin|IN2_AA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : LED_G_Pin GLCD_DI_Pin GLCD_RW_Pin GLCD_CS2_Pin LED_R_Pin */
	GPIO_InitStruct.Pin = LED_G_Pin|LED_R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : BTN_MAIN_Pin BTN_1_Pin BTN_2_Pin BTN_3_Pin BTN_4_Pin */
	GPIO_InitStruct.Pin = BTN_MAIN_Pin|BTN_1_Pin|BTN_2_Pin|BTN_3_Pin|BTN_4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}









