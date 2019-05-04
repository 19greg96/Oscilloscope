

#include "BSP_DAC.h"
#include "math.h"

static void MX_DAC_Init(void);
void MX_DAC_ChannelConfig(uint32_t channel, uint32_t bufferEnable);

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
	// record ADC DMA position, trigger offset from that is deltaT
}
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
	// record ADC DMA position, trigger offset from that is deltaT
}
void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac) {
	Error_Handler();
}
void HAL_DAC_DMAUnderrunCallbackCh1(DAC_HandleTypeDef *hdac) {
	Error_Handler();
}

void DAC_IRQHandler() {
	HAL_DAC_IRQHandler(&hdac);
}
void DMA1_Stream5_IRQHandler() {
	// https://www.st.com/content/ccc/resource/technical/document/reference_manual/4d/ed/bc/89/b5/70/40/dc/DM00135183.pdf/files/DM00135183.pdf/jcr:content/translations/en.DM00135183.pdf
	// RM0390
	// Table 28 / 29
	HAL_DMA_IRQHandler(hdac.DMA_Handle1);
}
void DMA1_Stream6_IRQHandler() {
	HAL_DMA_IRQHandler(hdac.DMA_Handle2);
}




void DAC_Init() {
	MX_DAC_Init();
	
	//HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0); // DAC2
	//HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn); // DAC2
	/*
	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	*/
}

void DAC_createSineBuffer(uint32_t* buffer, uint32_t bufferSize, uint32_t min, uint32_t max) {
	// max/4096 * 3.3V = 3.1V
	if (bufferSize > DAC_OUTPUT_BUFFER_SIZE) {
		bufferSize = DAC_OUTPUT_BUFFER_SIZE;
	}
	if (min > 4096) {
		min = 4096;
	}
	if (max > 4096) {
		max = 4096;
	}
	for (uint32_t i = 0; i < bufferSize; i ++) {
		buffer[i] = (sinf(i * 2.0f * M_PI / bufferSize) + 1.0f) * ((max - min) / 2) + min;
		// for faster value generation, table lookup + lin interpolation is available: arm_sin_f32
	}
}
void DAC_createTriangleBuffer(uint32_t* buffer, uint32_t bufferSize, uint32_t min, uint32_t max, uint32_t shape) {
	if (bufferSize > DAC_OUTPUT_BUFFER_SIZE) {
		bufferSize = DAC_OUTPUT_BUFFER_SIZE;
	}
	if (min > 4096) {
		min = 4096;
	}
	if (max > 4096) {
		max = 4096;
	}
	if (shape > bufferSize) {
		shape = bufferSize;
	}
	for (uint32_t i = 0; i < bufferSize; i ++) { // triangle
		if (i < shape) {
			buffer[i] = min + (max - min) * i / shape; // rising edge
		} else {
			buffer[i] = max - ((max - min) * (i - shape) / (bufferSize - shape)); // falling edge
		}
	}
}
void DAC_createSquareBuffer(uint32_t* buffer, uint32_t bufferSize, uint32_t min, uint32_t max, uint32_t duty, uint32_t rise) {
	// duty is off time
	if (bufferSize > DAC_OUTPUT_BUFFER_SIZE) {
		bufferSize = DAC_OUTPUT_BUFFER_SIZE;
	}
	if (min > 4096) {
		min = 4096;
	}
	if (max > 4096) {
		max = 4096;
	}
	if (duty > bufferSize) {
		duty = bufferSize;
	}
	if (rise > bufferSize / 2) {
		rise = bufferSize / 2;
	}
	for (uint32_t i = 0; i < bufferSize; i ++) { // square
		if (i < rise / 2) {
			buffer[i] = min + (max - min) / 2 - ((max - min) * i / rise); // falling half edge (from center to bot)
		} else if (i < duty - rise / 2) {
			buffer[i] = min; // flat bottom
		} else if (i < duty + rise / 2) {
			buffer[i] = min + (max - min) * (i - (duty - rise / 2)) / rise; // rising edge
		} else if (i < bufferSize - rise / 2) {
			buffer[i] = max; // flat top
		} else {
			buffer[i] = max - ((max - min) * (i - (bufferSize - rise / 2)) / rise); // falling half edge (from top to center)
		}
	}
}

// htim6.Init.Period = 9125; // 9125 -> 10.00Hz (with 1000 sized buffer), clock is 90MHz, to calibrate this number, set Prescaler to 1000, measure frequency, measured frequency will be this number
// 91 -> 100kHz (with 10 sized buffer), clock is 90MHz

DAC_FrequencySettingsTypedef DAC_FrequencyTable[] = {
	{ .frequency = 1.0f,		.prescaler = 9, .period = 9125,	.buffSize = 1000,	.interpolate = DAC_FIP_PERIOD },		// 1Hz		0
	{ .frequency = 2.0f,		.prescaler = 9, .period = 4562,	.buffSize = 1000,	.interpolate = DAC_FIP_PERIOD },		// 2Hz		1
	{ .frequency = 5.0f,		.prescaler = 9, .period = 1825,	.buffSize = 1000,	.interpolate = DAC_FIP_PERIOD },		// 5Hz		2
	{ .frequency = 10.0f,		.prescaler = 0, .period = 9125,	.buffSize = 1000,	.interpolate = DAC_FIP_PERIOD },		// 10Hz		3
	{ .frequency = 20.0f,		.prescaler = 0, .period = 4562,	.buffSize = 1000,	.interpolate = DAC_FIP_PERIOD },		// 20Hz		4
	{ .frequency = 50.0f,		.prescaler = 0, .period = 1825,	.buffSize = 1000,	.interpolate = DAC_FIP_PERIOD },		// 50Hz		5
	{ .frequency = 100.0f,		.prescaler = 0, .period = 912,	.buffSize = 1000,	.interpolate = DAC_FIP_PERIOD },		// 100Hz	6
	{ .frequency = 200.0f,		.prescaler = 0, .period = 456,	.buffSize = 1000,	.interpolate = DAC_FIP_PERIOD },		// 200Hz	7
	{ .frequency = 500.0f,		.prescaler = 0, .period = 182,	.buffSize = 1000,	.interpolate = DAC_FIP_PERIOD },		// 500Hz	8
	{ .frequency = 1000.0f,		.prescaler = 0, .period = 91,	.buffSize = 1000,	.interpolate = DAC_FIP_BUFFER_SIZE },	// 1kHz		9
	{ .frequency = 2000.0f,		.prescaler = 0, .period = 91,	.buffSize = 500,	.interpolate = DAC_FIP_BUFFER_SIZE },	// 2kHz		10
	{ .frequency = 5000.0f,		.prescaler = 0, .period = 91,	.buffSize = 200,	.interpolate = DAC_FIP_BUFFER_SIZE },	// 5kHz		11
	{ .frequency = 10000.0f,	.prescaler = 0, .period = 91,	.buffSize = 100,	.interpolate = DAC_FIP_BUFFER_SIZE },	// 10kHz	12
	{ .frequency = 20000.0f,	.prescaler = 0, .period = 45,	.buffSize = 100,	.interpolate = DAC_FIP_BUFFER_SIZE },	// 20kHz	13
	{ .frequency = 50000.0f,	.prescaler = 0, .period = 17,	.buffSize = 100,	.interpolate = DAC_FIP_BUFFER_SIZE },	// 50kHz	14
	{ .frequency = 100000.0f,	.prescaler = 0, .period = 17,	.buffSize = 50,		.interpolate = DAC_FIP_BUFFER_SIZE },	// 100kHz	15
	{ .frequency = 200000.0f,	.prescaler = 0, .period = 17,	.buffSize = 25,		.interpolate = DAC_FIP_BUFFER_SIZE },	// 200kHz	16
	{ .frequency = 500000.0f,	.prescaler = 0, .period = 17,	.buffSize = 10,		.interpolate = DAC_FIP_BUFFER_SIZE },	// 500kHz	17
	{ .frequency = 1000000.0f,	.prescaler = 0, .period = 17,	.buffSize = 5,		.interpolate = DAC_FIP_BUFFER_SIZE }	// 1MHz		18
};
// TODO: pulse setting for frequency domain bode measurement:
// { .frequency = nan,	.prescaler = 0, .period = 17,	.buffSize = manual triggering (wait for one whole ADC buffer to fill),		.interpolate = N/A }

void DAC_disable(uint32_t channel) {
	if (channel == DAC_CHANNEL_1) {
		if (hdac.DMA_Handle1->State == HAL_DMA_STATE_BUSY) {
			if (HAL_DAC_Stop_DMA(&hdac, channel) != HAL_OK) {
				Error_Handler();
			}
		}
	} else if (channel == DAC_CHANNEL_2) {
		if (hdac.DMA_Handle2->State == HAL_DMA_STATE_BUSY) {
			if (HAL_DAC_Stop_DMA(&hdac, channel) != HAL_OK) {
				Error_Handler();
			}
		}
	}
}
void DAC_enable(uint32_t channel) {
	DAC_disable(channel);
	if (channel == DAC_CHANNEL_1) {
		if (HAL_DAC_Start_DMA(&hdac, channel, g_DACBuffer1, DAC_ch1_config.frequencySettings.buffSize, DAC_ALIGN_12B_R) != HAL_OK) {
			Error_Handler();
		}
	} else if (channel == DAC_CHANNEL_2) {
		if (HAL_DAC_Start_DMA(&hdac, channel, g_DACBuffer2, DAC_ch2_config.frequencySettings.buffSize, DAC_ALIGN_12B_R) != HAL_OK) {
			Error_Handler();
		}
	}
}

void DAC_configure(uint32_t channel, uint32_t bufferEnable, float frequency_Hz, DAC_WaveformTypedef waveform, float min_V, float max_V, float shape_Percent, float rise_Percent) {
	DAC_SettingsTypedef* in;
	uint32_t* buffer;
	if (channel == DAC_CHANNEL_1) {
		in = &DAC_ch1_config;
		buffer = g_DACBuffer1;
	} else if (channel == DAC_CHANNEL_2) {
		in = &DAC_ch2_config;
		buffer = g_DACBuffer2;
	} else {
		Error_Handler();
		return; // to avoid 'in uninitialized' warning
	}
	
	if (min_V < 0.0f) {
		min_V = 0.0f;
	}
	if (min_V > DAC_VREF) {
		min_V = DAC_VREF;
	}
	if (max_V < min_V) {
		max_V = min_V;
	}
	if (max_V > DAC_VREF) {
		max_V = DAC_VREF;
	}
	if (shape_Percent < 0.0f) {
		shape_Percent = 0.0f;
	}
	if (shape_Percent > 1.0f) {
		shape_Percent = 1.0f;
	}
	if (rise_Percent < 0.0f) {
		rise_Percent = 0.0f;
	}
	if (rise_Percent > 1.0f) {
		rise_Percent = 1.0f;
	}
	
	DAC_createFreqSettings(&in->frequencySettings, frequency_Hz);
	
	in->waveform = waveform;
	in->min = (min_V / DAC_VREF) * DAC_ACCURACY;
	in->max = (max_V / DAC_VREF) * DAC_ACCURACY;
	in->shape = shape_Percent * in->frequencySettings.buffSize;
	in->rise = rise_Percent * in->frequencySettings.buffSize / 2;
	
	if (in->waveform == DAC_WAVEFORM_SINE) {
		DAC_createSineBuffer(buffer, in->frequencySettings.buffSize, in->min, in->max);
	} else if (in->waveform == DAC_WAVEFORM_TRIANGLE) {
		DAC_createTriangleBuffer(buffer, in->frequencySettings.buffSize, in->min, in->max, in->shape);
	} else if (in->waveform == DAC_WAVEFORM_SQUARE) {
		DAC_createSquareBuffer(buffer, in->frequencySettings.buffSize, in->min, in->max, in->shape, in->rise);
	}
	
	if (channel == DAC_CHANNEL_1) {
		htim6.Instance->ARR = in->frequencySettings.period;
		htim6.Instance->PSC = in->frequencySettings.prescaler;
	} else if (channel == DAC_CHANNEL_2) {
		htim7.Instance->ARR = in->frequencySettings.period;
		htim7.Instance->PSC = in->frequencySettings.prescaler;
	}
	
	MX_DAC_ChannelConfig(channel, bufferEnable);
}
void DAC_createFreqSettings(DAC_FrequencySettingsTypedef* out, float frequency_Hz) {
	if (frequency_Hz < 1.0f) {
		frequency_Hz = 1.0f;
	} else if (frequency_Hz > 1000000.0f) {
		frequency_Hz = 1000000.0f;
	}
	uint32_t idx = roundf(log10f(frequency_Hz) * 3.0f); // index of table entry below requested frequency
	if (DAC_FrequencyTable[idx].frequency > frequency_Hz) { // this is needed because of rounding
		idx --;
	}
	
	if (fabs(DAC_FrequencyTable[idx].frequency - frequency_Hz) < 0.01f) {
		*out = DAC_FrequencyTable[idx];
	}
	
	float F_PERIPHERY = 90000000.0f; // frequency of timer
	
	out->prescaler = DAC_FrequencyTable[idx].prescaler;
	out->interpolate = DAC_FrequencyTable[idx].interpolate;
	if (DAC_FrequencyTable[idx].interpolate == DAC_FIP_PERIOD) {
		out->buffSize = DAC_FrequencyTable[idx].buffSize;
		out->period = F_PERIPHERY / ((out->prescaler + 1.0f) * out->buffSize * frequency_Hz) - 1.0f;
	} else if (DAC_FrequencyTable[idx].interpolate == DAC_FIP_BUFFER_SIZE) {
		out->period = DAC_FrequencyTable[idx].period;
		out->buffSize = F_PERIPHERY / ((out->prescaler + 1.0f) * ((float)out->period + 1.0f) * frequency_Hz);
	}
	
	out->frequency = F_PERIPHERY / ((out->prescaler + 1.0f) * (float)out->buffSize * ((float)out->period + 1.0f));
}


/**
* @brief DAC MSP Initialization
* This function configures the hardware resources used in this example
* @param hdac: DAC handle pointer
* @retval None
*/
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac) {

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hdac->Instance==DAC) {
	/* USER CODE BEGIN DAC_MspInit 0 */

	/* USER CODE END DAC_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_DAC_CLK_ENABLE();
	
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**DAC GPIO Configuration		
		PA4		 ------> DAC_OUT1 
		*/
		GPIO_InitStruct.Pin = DAC_OUT1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(DAC_OUT1_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN DAC_MspInit 1 */
		hdma1dac1.Instance = DMA1_Stream5;
		hdma1dac1.Init.Channel  = DMA_CHANNEL_7;
		hdma1dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma1dac1.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma1dac1.Init.MemInc = DMA_MINC_ENABLE;
		hdma1dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma1dac1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		hdma1dac1.Init.Mode = DMA_CIRCULAR;
		hdma1dac1.Init.Priority = DMA_PRIORITY_HIGH;
		hdma1dac1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		
		if (HAL_DMA_Init(&hdma1dac1) != HAL_OK) {
			Error_Handler();
		}
		
		__HAL_LINKDMA(hdac, DMA_Handle1, hdma1dac1);
		
		
		GPIO_InitStruct.Pin = LED_USER_Pin; // DAC ch2
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(LED_USER_GPIO_Port, &GPIO_InitStruct);
		
		hdma1dac2.Instance = DMA1_Stream6;
		hdma1dac2.Init.Channel  = DMA_CHANNEL_7;
		hdma1dac2.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma1dac2.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma1dac2.Init.MemInc = DMA_MINC_ENABLE;
		hdma1dac2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma1dac2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		hdma1dac2.Init.Mode = DMA_CIRCULAR;
		hdma1dac2.Init.Priority = DMA_PRIORITY_HIGH;
		hdma1dac2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		
		if (HAL_DMA_Init(&hdma1dac2) != HAL_OK) {
			Error_Handler();
		}
		
		__HAL_LINKDMA(hdac, DMA_Handle2, hdma1dac2);
	/* USER CODE END DAC_MspInit 1 */
	}

}

/**
* @brief DAC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdac: DAC handle pointer
* @retval None
*/

void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac) {

	if(hdac->Instance==DAC) {
	/* USER CODE BEGIN DAC_MspDeInit 0 */

	/* USER CODE END DAC_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_DAC_CLK_DISABLE();
	
		/**DAC GPIO Configuration		
		PA4		 ------> DAC_OUT1 
		*/
		HAL_GPIO_DeInit(DAC_OUT1_GPIO_Port, DAC_OUT1_Pin);

	/* USER CODE BEGIN DAC_MspDeInit 1 */

	/* USER CODE END DAC_MspDeInit 1 */
	}

}


void MX_DAC_ChannelConfig(uint32_t channel, uint32_t bufferEnable) {
	DAC_ChannelConfTypeDef sConfig = {0};
	
	// DAC_TRIGGER_T7_TRGO
	sConfig.DAC_Trigger = (channel == DAC_CHANNEL_1 ? DAC_TRIGGER_T6_TRGO : DAC_TRIGGER_T7_TRGO);
	sConfig.DAC_OutputBuffer = (bufferEnable ? DAC_OUTPUTBUFFER_ENABLE : DAC_OUTPUTBUFFER_DISABLE);
	if (HAL_DAC_ConfigChannel(&hdac, &sConfig, channel) != HAL_OK) {
		Error_Handler();
	}
}

/**
	* @brief DAC Initialization Function
	* @param None
	* @retval None
	*/
static void MX_DAC_Init(void) {

	/* USER CODE BEGIN DAC_Init 0 */
	__DMA1_CLK_ENABLE();
/*
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
	*/
	/* USER CODE END DAC_Init 0 */

	/* USER CODE BEGIN DAC_Init 1 */
	
	/* USER CODE END DAC_Init 1 */
	/**DAC Initialization  */
	hdac.Instance = DAC;
	
	if (HAL_DAC_Init(&hdac) != HAL_OK) {
		Error_Handler();
	}
	/**DAC channel OUT1 config */
	MX_DAC_ChannelConfig(DAC_CHANNEL_1, 1);
	/* USER CODE BEGIN DAC_Init 2 */
	
	__HAL_RCC_TIM6_CLK_ENABLE();
	
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 0;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 45; // 73khz any Period lower than 22 will be inacurate
	
	htim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
		Error_Handler();
	}
	
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	HAL_TIM_Base_Start(&htim6);
	
	MX_DAC_ChannelConfig(DAC_CHANNEL_2, 1);
	__HAL_RCC_TIM7_CLK_ENABLE();
	
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 0;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 45; // 73khz any Period lower than 22 will be inacurate
	
	htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
		Error_Handler();
	}
	
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	HAL_TIM_Base_Start(&htim7);
	
	/* USER CODE END DAC_Init 2 */

}








