
#ifndef __BSP_DAC_H
#define __BSP_DAC_H


#include "main.h"
#include <stdint.h>

#define DAC_OUTPUT_BUFFER_SIZE	1000
// Clock frequency is 90MHz
// 90MHz / (DAC_OUTPUT_BUFFER_SIZE * Timer Period)
#define DAC_VREF		3.3f
#define DAC_ACCURACY	4096


uint32_t g_DACBuffer1[DAC_OUTPUT_BUFFER_SIZE];
uint32_t g_DACBuffer2[DAC_OUTPUT_BUFFER_SIZE];

typedef enum  {
	DAC_FIP_PERIOD			= 0x00U,	// 
	DAC_FIP_BUFFER_SIZE		= 0x01U
} DAC_FrequencyInterpolateParamTypedef;

typedef enum  {
	DAC_WAVEFORM_SINE		= 0x00U,	// 
	DAC_WAVEFORM_TRIANGLE	= 0x01U,
	DAC_WAVEFORM_SQUARE		= 0x02U
} DAC_WaveformTypedef;

typedef struct {
	float frequency;
	uint32_t prescaler;		// uint16_t
	uint32_t period;		// uint16_t
	uint32_t buffSize;		// uint16_t
	DAC_FrequencyInterpolateParamTypedef interpolate; // only for table & conversion
} DAC_FrequencySettingsTypedef;

typedef struct {
	DAC_FrequencySettingsTypedef frequencySettings;
	DAC_WaveformTypedef waveform;
	uint32_t min;
	uint32_t max;
	uint32_t shape; // duty for square wave, shape for triangle wave
	uint32_t rise; // only for square wave
} DAC_SettingsTypedef;

DAC_SettingsTypedef DAC_ch1_config;
DAC_SettingsTypedef DAC_ch2_config;

void DAC_createSineBuffer(uint32_t* buffer, uint32_t bufferSize, uint32_t min, uint32_t max);
void DAC_createTriangleBuffer(uint32_t* buffer, uint32_t bufferSize, uint32_t min, uint32_t max, uint32_t shape);
void DAC_createSquareBuffer(uint32_t* buffer, uint32_t bufferSize, uint32_t min, uint32_t max, uint32_t duty, uint32_t rise);

void DAC_disable(uint32_t channel);
void DAC_enable(uint32_t channel);
void DAC_configure(uint32_t channel, uint32_t bufferEnable, float frequency_Hz, DAC_WaveformTypedef waveform, float min_V, float max_V, float shape_Percent, float rise_Percent);
void DAC_createFreqSettings(DAC_FrequencySettingsTypedef* out, float frequency_Hz);

DMA_HandleTypeDef hdma1dac1;
DMA_HandleTypeDef hdma1dac2;
DAC_HandleTypeDef hdac;
TIM_HandleTypeDef htim6; // DAC1
TIM_HandleTypeDef htim7; // DAC2

#define DAC_OUT1_Pin		GPIO_PIN_4
#define DAC_OUT1_GPIO_Port	GPIOA
#define LED_USER_Pin		GPIO_PIN_5	// this is the same pin for DAC2
#define LED_USER_GPIO_Port	GPIOA

void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac);
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac);



void DAC_Init();




#endif
