

#include "BSP_UART.h"
#include "BSP_ADC.h"
#include <string.h>

/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* UartHandle) {
	
	if (UartHandle->Instance == USART2) {
		uint8_t b = *(UartHandle->pRxBuffPtr - 1);
		
		UART_inputBuffer[UART_inputBufferWritePos] = b;
		UART_inputBufferWritePos = ((UART_inputBufferWritePos + 1) & UART_INPUT_BUFFER_MASK);
		
		HAL_UART_Receive_IT(UartHandle, &rxBuffer, 5);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* UartHandle) {
	Error_Handler();
}
void USART2_IRQHandler() {
	recvd++;
	HAL_UART_IRQHandler(&huart2);
}*/


void USART2_IRQHandler() {
	recvd++;
	uint8_t b;
	volatile uint32_t sr=huart2.Instance->SR;
	b = (huart2.Instance->DR & (uint16_t)0x01FFU);
	if (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE) != RESET) {
		UART_inputBuffer[UART_inputBufferWritePos] = b;
		UART_inputBufferWritePos = ((UART_inputBufferWritePos + 1) & UART_INPUT_BUFFER_MASK);
	}
	HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
}


void UART_init() {
	recvd = 0;
	
	MX_USART2_UART_Init();
	// HAL_UART_MspInit(&huart2);
	
	// HAL_UART_Receive_IT(&huart2, &rxBuffer, 5);
}
void UART_update() {
    float f;
    uint32_t t;
	
	if (UART_available()) {
		uint8_t inChar = UART_read8();
		switch (inChar) {
			case 'f': // dac frequency
				f = UART_readFloat();
				
				DAC_configure(DAC_CHANNEL_1, f, DAC_WAVEFORM_SINE, 0.2f, 3.1f, 0.0f, 0.0f);
				break;
			case 't': // trigger level
				t = UART_read8();
				
				SCOPE_triggerLevel = t << ADC_oversampling;

				break;
			default:
				UART_write8(inChar);
			break;
		}
	}
}


uint32_t UART_available() {
	//__HAL_UART_DISABLE_IT(&huart2, UART_FLAG_RXNE);
	uint32_t writePos = UART_inputBufferWritePos; // copy 32bit is atomic on this architecture?
	//__HAL_UART_ENABLE_IT(&huart2, UART_FLAG_RXNE);
	
	return (((writePos & UART_INPUT_BUFFER_MASK) - (UART_inputBufferReadPos & UART_INPUT_BUFFER_MASK)) & UART_INPUT_BUFFER_MASK);
}
uint8_t UART_read8() {
	uint8_t ret = 0;
	while (!UART_available()) {}
	ret = UART_inputBuffer[UART_inputBufferReadPos];
	UART_inputBufferReadPos = ((UART_inputBufferReadPos + 1) & UART_INPUT_BUFFER_MASK);
	return ret;
}
uint16_t UART_read16() {
	uint8_t high = UART_read8();
	uint8_t low = UART_read8();
	return (high << 8) | low;
}
uint32_t UART_read32() {
	uint16_t high = UART_read16();
	uint16_t low = UART_read16();
	return (high << 16) | low;
}
float UART_readFloat() {
    uint8_t b0 = UART_read8();
    uint8_t b1 = UART_read8();
    uint8_t b2 = UART_read8();
    uint8_t b3 = UART_read8();
    uint32_t tmp = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
	
	return *((float*)&tmp);
}
void UART_readBuff(uint8_t* dst, uint32_t len) {
	for (uint32_t i = 0; i < len; i ++) {
		dst[i] = UART_read8();
	}
}
void UART_write8(uint8_t val) {
	HAL_UART_Transmit(&huart2, (uint8_t *)&val, 1, HAL_MAX_DELAY);
}
void UART_write16(uint16_t val) {
	UART_write8((val >> 8) & 0xFF);
	UART_write8(val & 0xFF);
}
void UART_write32(uint32_t val) {
	UART_write16((val >> 16) & 0xFFFF);
	UART_write16(val & 0xFFFF);
}
void UART_writeBuff(uint8_t* src, uint32_t len) {
	HAL_UART_Transmit(&huart2, (uint8_t *)src, len, HAL_MAX_DELAY);
}
void UART_writeString(char* string) {
	HAL_UART_Transmit(&huart2, (uint8_t *)string, strlen(string), HAL_MAX_DELAY);
}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart) { // called second, to start clock

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(huart->Instance==USART2) {
	/* USER CODE BEGIN USART2_MspInit 0 */

	/* USER CODE END USART2_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_USART2_CLK_ENABLE();
	
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USART2 GPIO Configuration		
		PA2		 ------> USART2_TX
		PA3		 ------> USART2_RX 
		*/
		GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
		huart->Instance->CR1 |= UART_FLAG_RXNE; // enable IT
		//__HAL_UART_ENABLE_IT(&huart, UART_FLAG_RXNE);
	}

}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart) {

	if(huart->Instance==USART2) {
	/* USER CODE BEGIN USART2_MspDeInit 0 */

	/* USER CODE END USART2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART2_CLK_DISABLE();
	
		/**USART2 GPIO Configuration		
		PA2		 ------> USART2_TX
		PA3		 ------> USART2_RX 
		*/
		HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

	/* USER CODE BEGIN USART2_MspDeInit 1 */

	/* USER CODE END USART2_MspDeInit 1 */
	}

}

/**
	* @brief USART2 Initialization Function
	* @param None
	* @retval None
	*/
void MX_USART2_UART_Init(void) { // called first to initialize struct
	UART_inputBufferWritePos = 0;
	UART_inputBufferReadPos = 0;
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
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */
	/* USER CODE END USART2_Init 2 */

}


