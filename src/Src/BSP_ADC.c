

#include "BSP_ADC.h"
#include <stdio.h>
#include <string.h>
#include "math.h"
#include <stdlib.h>
#include "bode.h"

// TODO: B channel triggering causes DMA abort to stall (possible fix: decrease DMA priority?)
// TODO: When triggering on channel B, buffer offset is incorrect (possible fixes: calculate ADC_bufferDelta in a different manner,
// or process buffers differently in ADC_update)

static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);

static inline void stopConversion(ADC_HandleTypeDef* AdcHandle) {
	if (AdcHandle->DMA_Handle->State == HAL_DMA_STATE_BUSY) {
		if (HAL_ADC_Stop_DMA(AdcHandle) != HAL_OK) { // stops DMA immediately
			Error_Handler();
		}
	}
	ADC_conversionEnd = 1;
}

static inline void processBufferNoTrigger(uint32_t buffStart, uint32_t *sourceBuffer, uint32_t *targetBuffer, volatile uint32_t* targetWritePos, volatile uint32_t* nSamplesAtCurrPos) {
	if (ADC_oversampling == 0) { // split logic for faster ISR
		for (uint32_t i = buffStart; i < (buffStart + ADC_INPUT_BUFFER_SIZE / 2); i ++) {
			targetBuffer[i] = sourceBuffer[i];
		}
	} else {
		uint32_t avg_step = (1 << (ADC_oversampling)); // when to avarage.
		
		targetBuffer[*targetWritePos] = 0;
		*nSamplesAtCurrPos = 0;
		
		for (uint32_t i = buffStart; i < (buffStart + ADC_INPUT_BUFFER_SIZE / 2); i ++) {
			if ((*nSamplesAtCurrPos) == avg_step) {
				*targetWritePos = ((*targetWritePos) + 1) & ADC_INPUT_BUFFER_MASK;
				targetBuffer[*targetWritePos] = 0;
				*nSamplesAtCurrPos = 0;
			}
			targetBuffer[*targetWritePos] += sourceBuffer[i]; // & ADC_INPUT_BUFFER_MASK
			(*nSamplesAtCurrPos)++;
		}
	}
}

static inline void processBuffer(uint32_t buffStart, uint32_t *sourceBuffer, uint32_t *targetBuffer, volatile uint32_t* targetWritePos, volatile uint32_t* nSamplesAtCurrPos) {
	uint32_t last;
	uint32_t curr;
	
	if (ADC_oversampling == 0) { // split logic for faster ISR
		if (ADC_bufferInitialized && SCOPE_triggerMode == SCOPE_TRIGGER_DISABLE && !ADC_triggered) { // this is here and not in loop for faster ISR
			ADC_triggered = 1;
			ADC_Oversampled_triggered_at = buffStart;
		}
		
		last = sourceBuffer[(buffStart - 1) & ADC_INPUT_BUFFER_MASK];
		for (uint32_t i = buffStart; i < (buffStart + ADC_INPUT_BUFFER_SIZE / 2); i ++) {
			curr = sourceBuffer[i];
			targetBuffer[i] = curr;
			if (!ADC_triggered && ADC_bufferInitialized) {
				if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_RISING) {
					if (last < SCOPE_triggerLevel && curr >= SCOPE_triggerLevel) {
						// TODO: implement hysteresis trigger to prevent trigger caused by noise on flat signals
						ADC_triggered = 1;
						ADC_Oversampled_triggered_at = i;
						// break;
					}
				} else if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_FALLING) {
					if (last > SCOPE_triggerLevel && curr <= SCOPE_triggerLevel) {
						ADC_triggered = 1;
						ADC_Oversampled_triggered_at = i;
						// break;
					}
				}
			}
			last = curr;
		}
	} else {
		uint32_t avg_step = (1 << (ADC_oversampling)); // when to avarage.
		
		targetBuffer[*targetWritePos] = 0;
		*nSamplesAtCurrPos = 0;
		
		if (ADC_bufferInitialized && SCOPE_triggerMode == SCOPE_TRIGGER_DISABLE && !ADC_triggered) { // this is here and not in loop for faster ISR
			ADC_triggered = 1;
			ADC_Oversampled_triggered_at = *targetWritePos;
		}
		
		for (uint32_t i = buffStart; i < (buffStart + ADC_INPUT_BUFFER_SIZE / 2); i ++) {
			if ((*nSamplesAtCurrPos) == avg_step) {
				if (!ADC_triggered && ADC_bufferInitialized) {
					last = targetBuffer[((*targetWritePos) - 1) & ADC_INPUT_BUFFER_MASK];
					curr = targetBuffer[*targetWritePos]; //  & ADC_INPUT_BUFFER_MASK
					if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_RISING) {
						if (last < SCOPE_triggerLevel && curr >= SCOPE_triggerLevel) {
							// TODO: implement hysteresis trigger to prevent trigger caused by noise on flat signals
							ADC_triggered = 1;
							ADC_Oversampled_triggered_at = *targetWritePos;
						}
					} else if (SCOPE_triggerEdge == SCOPE_TRIGGER_EDGE_FALLING) {
						if (last > SCOPE_triggerLevel && curr <= SCOPE_triggerLevel) {
							ADC_triggered = 1;
							ADC_Oversampled_triggered_at = *targetWritePos;
						}
					}
				}
				
				*targetWritePos = ((*targetWritePos) + 1) & ADC_INPUT_BUFFER_MASK;
				targetBuffer[*targetWritePos] = 0;
				*nSamplesAtCurrPos = 0;
			}
			targetBuffer[*targetWritePos] += sourceBuffer[i]; // & ADC_INPUT_BUFFER_MASK
			(*nSamplesAtCurrPos)++;
		}
	}
}

ADC_HandleTypeDef* ADC_TriggerSource_Handle; // hadc1
uint32_t* ADC_TriggerSource_Buffer; // g_ADCBuffer1
uint32_t* ADC_TriggerSource_OversampledBuffer; // g_ADCOversampledBuffer1
volatile uint32_t* ADC_TriggerSource_OversampledBufferWritePos; // ADC_oversampledBufferWritePos1
volatile uint32_t* ADC_TriggerSource_NSamplesAtCurrPos; // ADC_nSamplesAtCurrPos1

ADC_HandleTypeDef* ADC_Other_Handle;
uint32_t* ADC_Other_Buffer;
uint32_t* ADC_Other_OversampledBuffer;
volatile uint32_t* ADC_Other_OversampledBufferWritePos;
volatile uint32_t* ADC_Other_NSamplesAtCurrPos;


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle) {
	if (AdcHandle->Instance == ADC_Other_Handle->Instance) {
		if (!ADC_conversionEnd) { // other channel has to be still active
			// (ADC_INPUT_BUFFER_SIZE - hadc2.DMA_Handle->Instance->NDTR) - (ADC_INPUT_BUFFER_SIZE - hadc1.DMA_Handle->Instance->NDTR)
			// ADC_bufferDelta = (ADC_TriggerSource_Handle->DMA_Handle->Instance->NDTR - ADC_Other_Handle->DMA_Handle->Instance->NDTR) & ADC_INPUT_BUFFER_MASK;
			ADC_bufferDelta = (hadc1.DMA_Handle->Instance->NDTR - hadc2.DMA_Handle->Instance->NDTR) & ADC_INPUT_BUFFER_MASK;
		}
		processBufferNoTrigger(ADC_INPUT_BUFFER_SIZE / 2, ADC_Other_Buffer, ADC_Other_OversampledBuffer, ADC_Other_OversampledBufferWritePos, ADC_Other_NSamplesAtCurrPos);
		if (ADC_conversionEnd) {
			stopConversion(AdcHandle);
		}
		return;
	}
	
	if (ADC_triggered) { // trigger occured in HAL_ADC_ConvHalfCpltCallback, we waited for buffer to fill properly
		if (ADC_oversampling == 0) {
			stopConversion(ADC_TriggerSource_Handle);
			//stopConversion(ADC_Other_Handle);
			for (uint32_t i = ADC_INPUT_BUFFER_SIZE / 2; i < ADC_INPUT_BUFFER_SIZE; i ++) {
				ADC_TriggerSource_OversampledBuffer[i] = ADC_TriggerSource_Buffer[i];
				ADC_Other_OversampledBuffer[i] = ADC_Other_Buffer[i];
			}
			return;
		} else {
			uint32_t buffAfterTriggerSize = ((*ADC_TriggerSource_OversampledBufferWritePos) - ADC_Oversampled_triggered_at) & ADC_INPUT_BUFFER_MASK;
			if (buffAfterTriggerSize >= (ADC_INPUT_BUFFER_SIZE / 4)) {
				stopConversion(ADC_TriggerSource_Handle);
				return;
			}
		}
	}
	
	processBuffer(ADC_INPUT_BUFFER_SIZE / 2, ADC_TriggerSource_Buffer, ADC_TriggerSource_OversampledBuffer, ADC_TriggerSource_OversampledBufferWritePos, ADC_TriggerSource_NSamplesAtCurrPos);
	
	/*
	if (ADC_triggered && (ADC_Oversampled_triggered_at < (ADC_INPUT_BUFFER_SIZE * 3 / 4))) { // triggered before last quarter
		stopConversion(AdcHandle);
	}*/
	if (ADC_triggered) { // triggered before last quarter
		if (ADC_oversampling == 0) {
			if (ADC_Oversampled_triggered_at < (ADC_INPUT_BUFFER_SIZE * 3 / 4)) {
				stopConversion(ADC_TriggerSource_Handle);
				//stopConversion(ADC_Other_Handle);
			}
		} else {
			uint32_t buffAfterTriggerSize = ((*ADC_TriggerSource_OversampledBufferWritePos) - ADC_Oversampled_triggered_at) & ADC_INPUT_BUFFER_MASK;
			if (buffAfterTriggerSize >= (ADC_INPUT_BUFFER_SIZE / 4)) {
				stopConversion(ADC_TriggerSource_Handle);
			}
		}
	}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* AdcHandle) {
	if (AdcHandle->Instance == ADC_Other_Handle->Instance) {
		if (!ADC_conversionEnd) { // other channel has to be still active
			// (ADC_INPUT_BUFFER_SIZE - hadc2.DMA_Handle->Instance->NDTR) - (ADC_INPUT_BUFFER_SIZE - hadc1.DMA_Handle->Instance->NDTR)
			// ADC_bufferDelta = (ADC_TriggerSource_Handle->DMA_Handle->Instance->NDTR - ADC_Other_Handle->DMA_Handle->Instance->NDTR) & ADC_INPUT_BUFFER_MASK;
			ADC_bufferDelta = (hadc1.DMA_Handle->Instance->NDTR - hadc2.DMA_Handle->Instance->NDTR) & ADC_INPUT_BUFFER_MASK;
		}
		
		processBufferNoTrigger(0, ADC_Other_Buffer, ADC_Other_OversampledBuffer, ADC_Other_OversampledBufferWritePos, ADC_Other_NSamplesAtCurrPos);
		if (ADC_conversionEnd) {
			stopConversion(ADC_Other_Handle);
		}
		return;
	}
	
	if (ADC_triggered) { // trigger occured in HAL_ADC_ConvCpltCallback, we waited for buffer to fill properly
		if (ADC_oversampling == 0) {
			stopConversion(ADC_TriggerSource_Handle);
			//stopConversion(ADC_Other_Handle);
			for (uint32_t i = 0; i < ADC_INPUT_BUFFER_SIZE / 2; i ++) {
				ADC_TriggerSource_OversampledBuffer[i] = ADC_TriggerSource_Buffer[i];
				ADC_Other_OversampledBuffer[i] = ADC_Other_Buffer[i];
			}
			return;
		} else {
			uint32_t buffAfterTriggerSize = ((*ADC_TriggerSource_OversampledBufferWritePos) - ADC_Oversampled_triggered_at) & ADC_INPUT_BUFFER_MASK;
			if (buffAfterTriggerSize >= (ADC_INPUT_BUFFER_SIZE / 4)) {
				stopConversion(ADC_TriggerSource_Handle);
				return;
			}
		}
	}
	
	processBuffer(0, ADC_TriggerSource_Buffer, ADC_TriggerSource_OversampledBuffer, ADC_TriggerSource_OversampledBufferWritePos, ADC_TriggerSource_NSamplesAtCurrPos);
	
	if (ADC_triggered) {
		if (ADC_oversampling == 0) {
			if ((ADC_Oversampled_triggered_at < (ADC_INPUT_BUFFER_SIZE / 4)) || (ADC_Oversampled_triggered_at > (ADC_INPUT_BUFFER_SIZE / 2))) {
				stopConversion(ADC_TriggerSource_Handle);
				//stopConversion(ADC_Other_Handle);
			}
		} else {
			uint32_t buffAfterTriggerSize = ((*ADC_TriggerSource_OversampledBufferWritePos) - ADC_Oversampled_triggered_at) & ADC_INPUT_BUFFER_MASK;
			if (buffAfterTriggerSize >= (ADC_INPUT_BUFFER_SIZE / 4)) {
				stopConversion(ADC_TriggerSource_Handle);
			}
		}
	}
	
	if (ADC_bufferInitialized == 0) {
		// don't check first half of buffer for trigger event, because if it occured in first quarter,
		// we wouldn't have a full useable half buffer of data available. We only need to do this if
		// trigger event occured in first quarter, but logic for that would be more complicated.
		if (ADC_oversampling == 0) {
			ADC_bufferInitialized = 1;
		} else if ((*ADC_TriggerSource_OversampledBufferWritePos) > ADC_INPUT_BUFFER_SIZE / 4) {
			ADC_bufferInitialized = 1;
		}
	}
	/*
	if (ADC_triggered && ((ADC_Oversampled_triggered_at < (ADC_INPUT_BUFFER_SIZE / 4)) || (ADC_Oversampled_triggered_at > (ADC_INPUT_BUFFER_SIZE / 2)))) {
		stopConversion(AdcHandle);
	}
	*/
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* AdcHandle) {
	Error_Handler();
}


void ADC_IRQHandler() {
	HAL_ADC_IRQHandler(ADC_TriggerSource_Handle);
	HAL_ADC_IRQHandler(ADC_Other_Handle);
}

void DMA2_Stream0_IRQHandler() {
	HAL_DMA_IRQHandler(&hdma2adc1);
}
void DMA2_Stream2_IRQHandler() {
	HAL_DMA_IRQHandler(&hdma2adc2);
}

uint8_t runEnabled;
void SCOPE_setRunEnabled(uint8_t enabled) {
	if (enabled) {
		runEnabled = 1;
		HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
	} else {
		if (SCOPE_handleOnStop != NULL) {
			SCOPE_handleOnStop();
		}
		runEnabled = 0;
		HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
	}
}

void SCOPE_startConversion() {
	//stopConversion(NULL);
	if (hadc1.DMA_Handle->State == HAL_DMA_STATE_BUSY) {
		if (HAL_ADC_Stop_DMA(&hadc1) != HAL_OK) { // stops DMA immediately
			Error_Handler();
		}
	}
	if (hadc2.DMA_Handle->State == HAL_DMA_STATE_BUSY) {
		if (HAL_ADC_Stop_DMA(&hadc2) != HAL_OK) { // stops DMA immediately
			Error_Handler();
		}
	}
	
	SCOPE_setRunEnabled(1);
	ADC_conversionEnd = 0;
	ADC_triggered = 0;
	ADC_bufferInitialized = 0;
	
	ADC_oversampledBufferWritePos1 = 0;
	ADC_nSamplesAtCurrPos1 = 0;
	
	ADC_oversampledBufferWritePos2 = 0;
	ADC_nSamplesAtCurrPos2 = 0;
	
	switch (SCOPE_triggerSource) {
		case SCOPE_TRIGGER_SOURCE_DISABLED:
		case SCOPE_TRIGGER_SOURCE_CH1: {
			ADC_TriggerSource_Handle = &hadc1;
			ADC_TriggerSource_Buffer = g_ADCBuffer1;
			ADC_TriggerSource_OversampledBuffer = g_ADCOversampledBuffer1;
			ADC_TriggerSource_OversampledBufferWritePos = &ADC_oversampledBufferWritePos1;
			ADC_TriggerSource_NSamplesAtCurrPos = &ADC_nSamplesAtCurrPos1;

			ADC_Other_Handle = &hadc2;
			ADC_Other_Buffer = g_ADCBuffer2;
			ADC_Other_OversampledBuffer = g_ADCOversampledBuffer2;
			ADC_Other_OversampledBufferWritePos = &ADC_oversampledBufferWritePos2;
			ADC_Other_NSamplesAtCurrPos = &ADC_nSamplesAtCurrPos2;
		} break;
		case SCOPE_TRIGGER_SOURCE_CH2: {
			ADC_TriggerSource_Handle = &hadc2;
			ADC_TriggerSource_Buffer = g_ADCBuffer2;
			ADC_TriggerSource_OversampledBuffer = g_ADCOversampledBuffer2;
			ADC_TriggerSource_OversampledBufferWritePos = &ADC_oversampledBufferWritePos2;
			ADC_TriggerSource_NSamplesAtCurrPos = &ADC_nSamplesAtCurrPos2;

			ADC_Other_Handle = &hadc1;
			ADC_Other_Buffer = g_ADCBuffer1;
			ADC_Other_OversampledBuffer = g_ADCOversampledBuffer1;
			ADC_Other_OversampledBufferWritePos = &ADC_oversampledBufferWritePos1;
			ADC_Other_NSamplesAtCurrPos = &ADC_nSamplesAtCurrPos1;
		} break;
	}
	
	if (HAL_ADC_Start_DMA(&hadc1, g_ADCBuffer1, ADC_INPUT_BUFFER_SIZE) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_ADC_Start_DMA(&hadc2, g_ADCBuffer2, ADC_INPUT_BUFFER_SIZE) != HAL_OK) {
		Error_Handler();
	}
}

// TODO: the correction values here depend on buffer size implying issue with oversampling
ADC_FrequencySettingsTypedef ADC_FrequencyTable[] = {
	{ .frequency = 2500000.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV4, .resolution = ADC_RESOLUTION_6B,		.sampleTime = ADC_SAMPLETIME_3CYCLES,	.oversampling = 0}, // 0	9us		good
	{ .frequency = 1500000.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV4, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_3CYCLES,	.oversampling = 0}, // 1	14us	good
	{ .frequency = 1000000.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_3CYCLES,	.oversampling = 0}, // 2	21us	good
	{ .frequency = 750000.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV8, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_3CYCLES,	.oversampling = 0}, // 3	28us	good
	{ .frequency = 500000.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_3CYCLES,	.oversampling = 1}, // 4	43us	good
	{ .frequency = 375000.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_28CYCLES,	.oversampling = 0}, // 5	57us	good
	{ .frequency = 250000.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_3CYCLES,	.oversampling = 2}, // 6	85us	good
	{ .frequency = 125000.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_3CYCLES,	.oversampling = 3}, // 7	171us	good
	{ .frequency = 62500.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_3CYCLES,	.oversampling = 4}, // 8	341us	good
	{ .frequency = 31250.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_3CYCLES,	.oversampling = 5}, // 9	683us	good
	{ .frequency = 10340.0f,			.prescaler = ADC_CLOCK_SYNC_PCLK_DIV8, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_56CYCLES,	.oversampling = 4}, // 10	2ms		good
	{ .frequency = 3000.0f,				.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_144CYCLES,	.oversampling = 5}, // 11	7ms		good
	{ .frequency = 1365.33f/*1500.0f*/,	.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_144CYCLES,	.oversampling = 6}, // 12	16ms	good with correction
	{ .frequency = 952.74f,				.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_480CYCLES,	.oversampling = 5}, // 13	22ms	good
	{ .frequency = 580.27f/*751.2f*/,	.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_144CYCLES,	.oversampling = 7}, // 14	37ms	good with correction
	{ .frequency = 426.66f/*357.28f*/,	.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_480CYCLES,	.oversampling = 6}, // 15	50ms	good with correction		(128/6)/Fs
	{ .frequency = 178.64f,				.prescaler = ADC_CLOCK_SYNC_PCLK_DIV6, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_480CYCLES,	.oversampling = 7}, // 16	210ms	good
	{ .frequency = 46.933f/*89.32f*/,	.prescaler = ADC_CLOCK_SYNC_PCLK_DIV8, .resolution = ADC_RESOLUTION_12B,	.sampleTime = ADC_SAMPLETIME_480CYCLES,	.oversampling = 8}, // 17	240ms	good with correction
};
// Correction values calculated by setting DAC frequency so one whole period fills 4 horizontal divisions
// Fs is then (128/6)*4*Fdac


uint32_t ADC_FrequencySettingID;

// Ret: 1 on failure, 0 on success
// values are measured on buff2 compared to buff1
// out_period is measured in samples
uint32_t ADC_measurePeriodPhaseAmplitude(uint32_t *buff1, uint32_t *buff2, float *out_period, float *out_phase_deg, float *out_amplitude_dB) {
	uint32_t val1, val2;
	uint32_t last_val1, last_val2;
	uint32_t min1 = buff1[0];
	uint32_t min2 = buff2[0];
	uint32_t max1 = buff1[0];
	uint32_t max2 = buff2[0];
	uint32_t pp1; // peak to peak
	uint32_t pp2;
	uint32_t avg1 = 0;
	uint32_t avg2 = 0;
	uint32_t triggerPoint1_ch1 = 0xFFFFFFFF;
	uint32_t triggerPoint2_ch1 = 0xFFFFFFFF;
	uint32_t triggerPoint1_ch2 = 0xFFFFFFFF;
	uint32_t triggerPoint2_ch2 = 0xFFFFFFFF;
	uint32_t nTriggerPointsFound = 0;
	uint32_t measuredPeriod1; // unit is in number of samples
	uint32_t measuredPeriod2;
	uint32_t deltaSamples1; // deltaT
	uint32_t deltaSamples2; // deltaT
	float avgDeltaSamples;
	float phase; // in degrees
	float amplitude; // of ch2 compared to ch1 in dB
	for (uint32_t i = 0; i < ADC_INPUT_BUFFER_SIZE / 2; i ++) { // only half the buffer is usable
		val1 = buff1[(i + ADC_Oversampled_triggered_at - (ADC_INPUT_BUFFER_SIZE / 4)) & ADC_INPUT_BUFFER_MASK];
		val2 = buff2[(i + ADC_bufferDelta + ADC_Oversampled_triggered_at - (ADC_INPUT_BUFFER_SIZE / 4)) & ADC_INPUT_BUFFER_MASK];
		
		avg1 += val1;
		avg2 += val2;
		
		if (val1 < min1) {
			min1 = val1;
		}
		if (val2 < min2) {
			min2 = val2;
		}
		if (val1 > max1) {
			max1 = val1;
		}
		if (val2 > max2) {
			max2 = val2;
		}
	}
	avg1 = avg1 / (ADC_INPUT_BUFFER_SIZE / 2);
	avg2 = avg2 / (ADC_INPUT_BUFFER_SIZE / 2);
	pp1 = max1 - min1;
	pp2 = max2 - min2;
	for (uint32_t i = 0; i < ADC_INPUT_BUFFER_SIZE / 2; i ++) { // only half the buffer is usable
		last_val1 = buff1[(i + ADC_Oversampled_triggered_at - (ADC_INPUT_BUFFER_SIZE / 4) - 1) & ADC_INPUT_BUFFER_MASK];
		last_val2 = buff2[(i + ADC_bufferDelta + ADC_Oversampled_triggered_at - (ADC_INPUT_BUFFER_SIZE / 4) - 1) & ADC_INPUT_BUFFER_MASK];
		
		val1 = buff1[(i + ADC_Oversampled_triggered_at - (ADC_INPUT_BUFFER_SIZE / 4)) & ADC_INPUT_BUFFER_MASK];
		val2 = buff2[(i + ADC_bufferDelta + ADC_Oversampled_triggered_at - ADC_INPUT_BUFFER_SIZE / 4) & ADC_INPUT_BUFFER_MASK];
		
		if (last_val1 < avg1 && val1 >= avg1) { // TODO: implement hysteresis trigger
			if (triggerPoint1_ch1 == 0xFFFFFFFF) {
				triggerPoint1_ch1 = i;
				nTriggerPointsFound++;
			} else if (triggerPoint2_ch1 == 0xFFFFFFFF) {
				triggerPoint2_ch1 = i;
				nTriggerPointsFound++;
			}
		}
		if (last_val2 < avg2 && val2 >= avg2) { // TODO: implement hysteresis trigger
			if (triggerPoint1_ch2 == 0xFFFFFFFF) {
				triggerPoint1_ch2 = i;
				nTriggerPointsFound++;
			} else if (triggerPoint2_ch2 == 0xFFFFFFFF) {
				triggerPoint2_ch2 = i;
				nTriggerPointsFound++;
			}
		}
		if (nTriggerPointsFound == 4) {
			break;
		}
	}
	if (nTriggerPointsFound < 4) {
		return 1; // not enough trigger points found (sample too flat => decrease sampling freq)
	}
	
	measuredPeriod1 = triggerPoint2_ch1 - triggerPoint1_ch1; // unit is in number of samples
	measuredPeriod2 = triggerPoint2_ch2 - triggerPoint1_ch2;
	if (abs(measuredPeriod1 - measuredPeriod2) > 5) {
		// magic const, if input and output frequencies differ too much, it means we measured noise
		// TODO: we could use this branch to determine when to find new trigger points in previous cycle
		// when it shows an error, we look for the next trigger point for every channel
		// if there are no more trigger points, we return error
		// also magic const should be %, not absolute error
		return 1;
	}
	*out_period = ((float)measuredPeriod1 + (float)measuredPeriod2) / 2.0f; // avarage
	
	if (triggerPoint1_ch1 > triggerPoint1_ch2) {
		deltaSamples1 = triggerPoint1_ch1 - triggerPoint1_ch2;
	} else {
		deltaSamples1 = triggerPoint1_ch2 - triggerPoint1_ch1;
	}
	if (triggerPoint2_ch1 > triggerPoint2_ch2) {
		deltaSamples2 = triggerPoint2_ch1 - triggerPoint2_ch2;
	} else {
		deltaSamples2 = triggerPoint2_ch2 - triggerPoint2_ch1;
	}
	if (abs(deltaSamples1 - deltaSamples2) > 5) {
		// TODO: magic const should be %, not absolute error
		return 1;
	}
	
	avgDeltaSamples = ((float)deltaSamples1 + (float)deltaSamples2) / 2.0f;
	
	if (avgDeltaSamples > (*out_period)) {
		return 1; // phase cannot be > 360°
	}
	
	char tmpBuff[32];
	
	phase = ((float)avgDeltaSamples / (float)(*out_period)) * 360.0f; // in degrees
	*out_phase_deg = phase;
	
	amplitude = 20.0f * log10f((float)pp2 / (float)pp1); // of ch2 compared to ch1 in dB
	*out_amplitude_dB = amplitude;
	
	sprintf(tmpBuff, "v1 %lu\n", triggerPoint1_ch1);
	UART_writeString(tmpBuff);
	sprintf(tmpBuff, "v1 %lu\n", triggerPoint2_ch1);
	UART_writeString(tmpBuff);
	sprintf(tmpBuff, "v2 %lu\n", triggerPoint1_ch2);
	UART_writeString(tmpBuff);
	sprintf(tmpBuff, "v2 %lu\n", triggerPoint2_ch2);
	UART_writeString(tmpBuff);
	
	return 0;
}

void ADC_SetFrequencyID(uint32_t id) {
	if (id >= numADCFrequencySettings) {
		return;
	}
	stopConversion(NULL);
	
	ADC_FrequencySettingID = id;
	ADC_oversampling = ADC_FrequencyTable[ADC_FrequencySettingID].oversampling;
	
	
	if (ADC_FrequencyTable[ADC_FrequencySettingID].resolution == ADC_RESOLUTION_12B) {
		ADC_accuracy = 4096;
	} else if (ADC_FrequencyTable[ADC_FrequencySettingID].resolution == ADC_RESOLUTION_10B) {
		ADC_accuracy = 1024;
	} else if (ADC_FrequencyTable[ADC_FrequencySettingID].resolution == ADC_RESOLUTION_8B) {
		ADC_accuracy = 256;
	} else if (ADC_FrequencyTable[ADC_FrequencySettingID].resolution == ADC_RESOLUTION_6B) {
		ADC_accuracy = 64;
	}
	
	ADC_setTriggerLevel(SCOPE_triggerLevel_V); // update trigger level based on new ADC_accuracy and ADC_oversampling
	
	MX_ADC1_Init();
	MX_ADC2_Init();
	
	SCOPE_startConversion(); // TODO: this will start conversion even when it was not enabled
}
ADC_FrequencySettingsTypedef* ADC_GetFrequencySettings() {
	return &ADC_FrequencyTable[ADC_FrequencySettingID];
}


void ADC_Init() {
	numADCFrequencySettings = sizeof(ADC_FrequencyTable) / sizeof(ADC_FrequencySettingsTypedef);
	
	SCOPE_handleOnStop = NULL;
	
	ADC_conversionEnd = 0;
	ADC_Oversampled_triggered_at = 0;
	
	SCOPE_triggerSource = SCOPE_TRIGGER_SOURCE_DISABLED;
	SCOPE_triggerEdge = SCOPE_TRIGGER_EDGE_DISABLED; // SCOPE_TRIGGER_FALLING
	SCOPE_triggerMode = SCOPE_TRIGGER_DISABLE; // SCOPE_TRIGGER_CONTINUOUS; // SCOPE_TRIGGER_DISABLE SCOPE_TRIGGER_SINGLE 
	
	ADC_SetFrequencyID(0);
	
	HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);
	
	// HAL_TIM_Base_Start(&htim2);
}
void ADC_setTriggerLevel(float level_V) {
	SCOPE_triggerLevel_V = level_V;
	if (HAL_GPIO_ReadPin(IN1_AC_GPIO_Port, IN1_AC_Pin) == GPIO_PIN_RESET) { // AC coupling is on
		SCOPE_triggerLevel = (uint32_t)((level_V / ADC_VMAX + 0.5f) * ADC_accuracy) << ADC_oversampling;
	} else { // AC coupling is off
		SCOPE_triggerLevel = (uint32_t)((level_V / ADC_VMAX) * ADC_accuracy) << ADC_oversampling;
	}
}

void ADC_update(float* outputBuffer1_V, float* outputBuffer2_V) {
	char tmpBuff[32];
	
	if (ADC_conversionEnd) {
		UART_writeString("\nclr\n");
		float rangeOffset;
		float divisor = (1 << ADC_oversampling);
		
		// TODO: investigate usage of full buffer to increase FFT frequency resolution
		// Process Channel A
		if (HAL_GPIO_ReadPin(IN1_AC_GPIO_Port, IN1_AC_Pin) == GPIO_PIN_RESET) { // AC coupling is on
			rangeOffset = 0.5f;
		} else { // AC coupling is off
			rangeOffset = 0.0f;
		}
		for (uint32_t i = 0; i < ADC_INPUT_BUFFER_SIZE / 2; i ++) {
			uint32_t val = g_ADCOversampledBuffer1[(i + ADC_Oversampled_triggered_at - ADC_INPUT_BUFFER_SIZE / 4) & ADC_INPUT_BUFFER_MASK];
			outputBuffer1_V[i] = ((float)val / divisor / (float)ADC_accuracy - rangeOffset) * ADC_VMAX;
			
			// TODO: if uart transmit enabled
			//sprintf(tmpBuff, "%ld\n", (int32_t)(outputBuffer1_V[i] * 4096.0f));
			//UART_writeString(tmpBuff);
		}
		// End process Channel A
		
		UART_writeString("\nrst\n");
		
		// Process Channel B
		if (HAL_GPIO_ReadPin(IN1_AC_GPIO_Port, IN2_AC_Pin) == GPIO_PIN_RESET) { // AC coupling is on
			rangeOffset = 0.5f;
		} else { // AC coupling is off
			rangeOffset = 0.0f;
		}
		int32_t bufferOffset = ((ADC_bufferDelta & 512) ? (-(((~(int32_t)ADC_bufferDelta) & ADC_INPUT_BUFFER_MASK) + 1)) : (int32_t)ADC_bufferDelta) / (int32_t)divisor; // convert from 10bit two's complement to hardware two's complement
		for (uint32_t i = 0; i < ADC_INPUT_BUFFER_SIZE / 2; i ++) {
			uint32_t val = g_ADCOversampledBuffer2[(i + (bufferOffset) + ADC_Oversampled_triggered_at - ADC_INPUT_BUFFER_SIZE / 4) & ADC_INPUT_BUFFER_MASK];
			outputBuffer2_V[i] = ((float)val / divisor / (float)ADC_accuracy - rangeOffset) * ADC_VMAX;
			
			// TODO: if uart transmit enabled
			//sprintf(tmpBuff, "%ld\n", (int32_t)(outputBuffer2_V[i] * 4096.0f));
			//UART_writeString(tmpBuff);
		}
		// End process Channel B
		
		
		BODE_processBuffer(outputBuffer1_V, outputBuffer2_V);
		
		/*
		// UART transmit
		for (uint32_t i = 0; i < ADC_INPUT_BUFFER_SIZE / 2; i ++) { // only half the buffer is usable
			// g_ADCBuffer[(i + ADC_triggered_at - ADC_INPUT_BUFFER_SIZE / 4) & ADC_INPUT_BUFFER_MASK]
			sprintf(tmpBuff, "%lu\n", (uint32_t)(g_ADCOversampledBuffer2[(i + ADC_bufferDelta + ADC_Oversampled_triggered_at - ADC_INPUT_BUFFER_SIZE / 4) & ADC_INPUT_BUFFER_MASK]));
			UART_writeString(tmpBuff);
		}
		
		
		
		uint32_t max = (ADC_INPUT_BUFFER_SIZE / 2);
		uint32_t val;
		for (uint32_t i = 0; i < max; i ++) { // only half the buffer is usable
			val = g_ADCOversampledBuffer1[(i + ADC_Oversampled_triggered_at - (max >> 1)) & ADC_INPUT_BUFFER_MASK];
			sprintf(tmpBuff, "%lu\n", val >> ADC_oversampling); // center the trigger point
			UART_writeString(tmpBuff);
		}
		*/
		sprintf(tmpBuff, "capat %d\n", (ADC_INPUT_BUFFER_SIZE / 4));
		UART_writeString(tmpBuff);
		sprintf(tmpBuff, "triglv %lu\n", SCOPE_triggerLevel >> ADC_oversampling);
		UART_writeString(tmpBuff);
		// End UART transmit
		
		if (SCOPE_triggerMode == SCOPE_TRIGGER_CONTINUOUS || SCOPE_triggerMode == SCOPE_TRIGGER_DISABLE) {
			if (runEnabled) {
				SCOPE_startConversion();
			}
		} else {
			SCOPE_setRunEnabled(0);
		}
	}
}

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hadc->Instance==ADC1) {
	/* USER CODE BEGIN ADC1_MspInit 0 */

	/* USER CODE END ADC1_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_ADC1_CLK_ENABLE();
	
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**ADC1 GPIO Configuration		
		PA0-WKUP		 ------> ADC1_IN0 
		*/
		GPIO_InitStruct.Pin = ADC_IN1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(ADC_IN1_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN ADC1_MspInit 1 */
		
	/* USER CODE END ADC1_MspInit 1 */
	} else if(hadc->Instance==ADC2) {
	/* USER CODE BEGIN ADC2_MspInit 0 */

	/* USER CODE END ADC2_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_ADC2_CLK_ENABLE();
	
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**ADC2 GPIO Configuration		
		PA1		 ------> ADC2_IN1 
		*/
		GPIO_InitStruct.Pin = ADC_IN2_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(ADC_IN2_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN ADC2_MspInit 1 */
		
	/* USER CODE END ADC2_MspInit 1 */
	}

}

/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc) {

	if(hadc->Instance==ADC1) {
	/* USER CODE BEGIN ADC1_MspDeInit 0 */

	/* USER CODE END ADC1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_ADC1_CLK_DISABLE();
	
		/**ADC1 GPIO Configuration		
		PA0-WKUP		 ------> ADC1_IN0 
		*/
		HAL_GPIO_DeInit(ADC_IN1_GPIO_Port, ADC_IN1_Pin);

	/* USER CODE BEGIN ADC1_MspDeInit 1 */

	/* USER CODE END ADC1_MspDeInit 1 */
	}
	else if(hadc->Instance==ADC2) {
	/* USER CODE BEGIN ADC2_MspDeInit 0 */

	/* USER CODE END ADC2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_ADC2_CLK_DISABLE();
	
		/**ADC2 GPIO Configuration		
		PA1		 ------> ADC2_IN1 
		*/
		HAL_GPIO_DeInit(ADC_IN2_GPIO_Port, ADC_IN2_Pin);

	/* USER CODE BEGIN ADC2_MspDeInit 1 */

	/* USER CODE END ADC2_MspDeInit 1 */
	}

}


/**
	* @brief ADC1 Initialization Function
	* @param None
	* @retval None
	*/
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = {0};
	// ADC_AnalogWDGConfTypeDef sAWDConfig = {0};

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */
	/**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_FrequencyTable[ADC_FrequencySettingID].prescaler; // ADC_FrequencyTable[ADC_FrequencySettingID].prescaler; // 90MHz / 4 = Fadc = 22.5MHz
	hadc1.Init.Resolution = ADC_FrequencyTable[ADC_FrequencySettingID].resolution; // ADC_FrequencyTable[ADC_FrequencySettingID].resolution; // Tconv = SamplingTime + Resolution [1/Fadc]
	
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = ENABLE; // ENABLE
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; // ADC_EXTERNALTRIGCONVEDGE_RISING; // ADC_EXTERNALTRIGCONVEDGE_NONE
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1; // ADC_EXTERNALTRIGCONV_T2_TRGO; // ADC_EXTERNALTRIGCONV_T1_CC1
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = ENABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}
	
	/**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. */
	/*
	
	SampleTime:
	ADC_SAMPLETIME_3CYCLES
	ADC_SAMPLETIME_15CYCLES
	ADC_SAMPLETIME_28CYCLES
	ADC_SAMPLETIME_56CYCLES
	ADC_SAMPLETIME_84CYCLES
	ADC_SAMPLETIME_112CYCLES
	ADC_SAMPLETIME_144CYCLES
	ADC_SAMPLETIME_480CYCLES
	
	Prescaler:
	ADC_CLOCK_SYNC_PCLK_DIV4
	ADC_CLOCK_SYNC_PCLK_DIV6
	ADC_CLOCK_SYNC_PCLK_DIV8
	
	Resolution:
	ADC_RESOLUTION_12B
	ADC_RESOLUTION_10B
	ADC_RESOLUTION_8B
	ADC_RESOLUTION_6B
	
	
	Fadc = 90MHz / Prescaler = 22.5MHz
	
	
	Tconv = SampleTime + Resolution [1/Fadc]
	Fconv = Fadc / (SampleTime + Resolution)
	
	*/
	
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_FrequencyTable[ADC_FrequencySettingID].sampleTime; // ADC_FrequencyTable[ADC_FrequencySettingID].sampleTime; // 480, 28, 3
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	
	// sAWDConfig.Channel = ADC_CHANNEL_0;
	// sAWDConfig.HighThreshold = SCOPE_triggerLevel;
	// sAWDConfig.LowThreshold = 0;
	// sAWDConfig.ITMode = ENABLE;
	// sAWDConfig.WatchdogMode = ADC_ANALOGWATCHDOG_ALL_REG;
	// if (HAL_ADC_AnalogWDGConfig(&hadc1, &sAWDConfig) != HAL_OK) {
		// Error_Handler();
	// }
	/* USER CODE BEGIN ADC1_Init 2 */
	/* USER CODE END ADC1_Init 2 */

	// RM0390
	// https://www.st.com/content/ccc/resource/technical/document/reference_manual/4d/ed/bc/89/b5/70/40/dc/DM00135183.pdf/files/DM00135183.pdf/jcr:content/translations/en.DM00135183.pdf
	__DMA2_CLK_ENABLE(); 
	hdma2adc1.Instance = DMA2_Stream0;
  
	hdma2adc1.Init.Channel  = DMA_CHANNEL_0;
	hdma2adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma2adc1.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma2adc1.Init.MemInc = DMA_MINC_ENABLE;
	hdma2adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma2adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	hdma2adc1.Init.Mode = DMA_CIRCULAR;
	hdma2adc1.Init.Priority = DMA_PRIORITY_HIGH;
	hdma2adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma2adc1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
	hdma2adc1.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma2adc1.Init.PeriphBurst = DMA_PBURST_SINGLE; 
	
	if (HAL_DMA_Init(&hdma2adc1) != HAL_OK) {
		Error_Handler();
	}
	
	__HAL_LINKDMA(&hadc1, DMA_Handle, hdma2adc1);
 
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);   
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
	
	
	/*
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	
	__TIM2_CLK_ENABLE();

	htim2.Instance = TIM2;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Prescaler = 35;
	htim2.Init.Period = 2 * 2; // 180 value to measure 1us, 2.5MHz -> 0.4us, 180*0.4 = 72
	htim2.State = HAL_TIM_STATE_RESET;
	HAL_TIM_Base_Init(&htim2);
	
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	*/
}

/**
	* @brief ADC2 Initialization Function
	* @param None
	* @retval None
	*/
static void MX_ADC2_Init(void) {

	/* USER CODE BEGIN ADC2_Init 0 */

	/* USER CODE END ADC2_Init 0 */

	ADC_ChannelConfTypeDef sConfig = {0};

	/* USER CODE BEGIN ADC2_Init 1 */

	/* USER CODE END ADC2_Init 1 */
	/**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
	*/
	hadc2.Instance = ADC2;
	hadc2.Init.ClockPrescaler = ADC_FrequencyTable[ADC_FrequencySettingID].prescaler;
	hadc2.Init.Resolution = ADC_FrequencyTable[ADC_FrequencySettingID].resolution;
	hadc2.Init.ScanConvMode = DISABLE;
	hadc2.Init.ContinuousConvMode = ENABLE;
	hadc2.Init.DiscontinuousConvMode = DISABLE;
	hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
	hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc2.Init.NbrOfConversion = 1;
	hadc2.Init.DMAContinuousRequests = ENABLE;
	hadc2.Init.EOCSelection = DISABLE;
	if (HAL_ADC_Init(&hadc2) != HAL_OK) {
		Error_Handler();
	}
	/**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
	*/
	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_FrequencyTable[ADC_FrequencySettingID].sampleTime;
	if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC2_Init 2 */
	
	// RM0390
	// https://www.st.com/content/ccc/resource/technical/document/reference_manual/4d/ed/bc/89/b5/70/40/dc/DM00135183.pdf/files/DM00135183.pdf/jcr:content/translations/en.DM00135183.pdf
	__DMA2_CLK_ENABLE(); 
	hdma2adc2.Instance = DMA2_Stream2;
  
	hdma2adc2.Init.Channel  = DMA_CHANNEL_1;
	hdma2adc2.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma2adc2.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma2adc2.Init.MemInc = DMA_MINC_ENABLE;
	hdma2adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma2adc2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	hdma2adc2.Init.Mode = DMA_CIRCULAR;
	hdma2adc2.Init.Priority = DMA_PRIORITY_HIGH;
	hdma2adc2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma2adc2.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
	hdma2adc2.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma2adc2.Init.PeriphBurst = DMA_PBURST_SINGLE;
	
	if (HAL_DMA_Init(&hdma2adc2) != HAL_OK) {
		Error_Handler();
	}
	
	__HAL_LINKDMA(&hadc2, DMA_Handle, hdma2adc2);
 
	HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);   
	HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
	
	/* USER CODE END ADC2_Init 2 */

}








