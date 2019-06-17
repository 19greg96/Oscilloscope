
#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define ADC_VREF		(VDD_VALUE / 1000.0f)		// VDD_VALUE = Vdd in mV
#define ADC_VMAX		(ADC_VREF * (3.0f / 2.0f))	// because of input division

#define ADC_INPUT_BUFFER_SIZE	4096				// this has to be twice the size of usable buffer.
#define ADC_INPUT_BUFFER_MASK	(ADC_INPUT_BUFFER_SIZE-1)
volatile uint32_t ADC_conversionEnd;		// flag: true after trigger event and buffer has been filled
volatile uint32_t ADC_triggered;			// flag: set by trigger, tells DMA to stop when buffer is full.
volatile uint32_t ADC_Oversampled_triggered_at;			// pointer to position in g_ADCOversampledBuffer.
volatile uint32_t ADC_bufferInitialized;	// flag: false when DMA is started, and Trigger has not been enabled


typedef enum {
	SCOPE_TRIGGER_DISABLE			= 0x00U,	// in SCOPE_TRIGGER_DISABLE, trigger is disabled, new diagrams are drawn at every buffer full event.
	SCOPE_TRIGGER_CONTINUOUS		= 0x01U,	// in SCOPE_TRIGGER_CONTINUOUS, trigger is enabled.
	SCOPE_TRIGGER_SINGLE			= 0x02U		// in SCOPE_TRIGGER_SINGLE, we wait for trigger event. When it occurs, we disable triggering. Re enable with: SCOPE_enableTrigger();
} SCOPE_TriggerModeTypedef;
typedef enum {
	SCOPE_TRIGGER_EDGE_DISABLED		= 0x00U,	// this is needed for faster ISR logic
	SCOPE_TRIGGER_EDGE_FALLING		= 0x01U,
	SCOPE_TRIGGER_EDGE_RISING		= 0x02U
} SCOPE_TriggerEdgeTypedef;
typedef enum {
	SCOPE_TRIGGER_SOURCE_DISABLED	= 0x00U,	// 
	SCOPE_TRIGGER_SOURCE_CH1		= 0x01U,
	SCOPE_TRIGGER_SOURCE_CH2		= 0x02U
} SCOPE_TriggerSourceTypedef;

typedef struct {
	float frequency;
	uint32_t prescaler;		// ADC_CLOCK_SYNC_PCLK_DIV4
	uint32_t resolution;	// ADC_RESOLUTION_6B
	uint32_t sampleTime;	// ADC_SAMPLETIME_3CYCLES
	uint32_t oversampling;	// number of adjacent samples to sum
} ADC_FrequencySettingsTypedef;

SCOPE_TriggerModeTypedef SCOPE_triggerMode;
SCOPE_TriggerEdgeTypedef SCOPE_triggerEdge;
SCOPE_TriggerSourceTypedef SCOPE_triggerSource;
float SCOPE_triggerLevel_V; // keep copy of trigger level in volts, so we can update trigger level on oversampling value change
uint32_t SCOPE_triggerLevel;						// scope trigger level

typedef void (*ADC_StopHandlerTypedef)();
ADC_StopHandlerTypedef SCOPE_handleOnStop;			// called when single trigger occurs
void SCOPE_setRunEnabled(uint8_t enabled);			// called to toggle user LEDs & to wait for conversion to end, then stop conversion
void SCOPE_startConversion();

uint32_t g_ADCBuffer1[ADC_INPUT_BUFFER_SIZE];				// DMA target buffer for ADC1	// TODO: reduce number of redundant ADC buffers
uint32_t g_ADCBuffer2[ADC_INPUT_BUFFER_SIZE];				// DMA target buffer for ADC2	// TODO: reduce number of redundant ADC buffers
uint32_t g_ADCOversampledBuffer1[ADC_INPUT_BUFFER_SIZE];	// secondary buffer to store avaraged values for higher time divisions	// TODO: reduce number of redundant ADC buffers
uint32_t g_ADCOversampledBuffer2[ADC_INPUT_BUFFER_SIZE];	// secondary buffer to store avaraged values for higher time divisions	// TODO: reduce number of redundant ADC buffers
uint32_t ADC_oversampling;
volatile uint32_t ADC_oversampledBufferWritePos1;
volatile uint32_t ADC_oversampledBufferWritePos2;
volatile uint32_t ADC_nSamplesAtCurrPos1;
volatile uint32_t ADC_nSamplesAtCurrPos2;
volatile uint32_t ADC_bufferDelta; // DMA write position difference
uint32_t ADC_accuracy;

DMA_HandleTypeDef hdma2adc1;
DMA_HandleTypeDef hdma2adc2;
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
TIM_HandleTypeDef htim2;

#define ADC_IN1_Pin			GPIO_PIN_0
#define ADC_IN1_GPIO_Port	GPIOA
#define ADC_IN2_Pin			GPIO_PIN_1
#define ADC_IN2_GPIO_Port	GPIOA

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc);
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc);

extern ADC_FrequencySettingsTypedef ADC_FrequencyTable[];
void ADC_Init();
void ADC_setTriggerLevel(float level_V);
void ADC_update();

uint32_t numADCFrequencySettings;
void ADC_SetFrequencyID(uint32_t id);
ADC_FrequencySettingsTypedef* ADC_GetFrequencySettings();


#endif
