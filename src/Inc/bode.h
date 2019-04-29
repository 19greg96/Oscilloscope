/*
 * bode.h
 *
 *  Created on: 2019. ápr. 21.
 *      Author: Greg
 */

#ifndef BODE_H_
#define BODE_H_

#include "main.h"

float BODE_avgPhase_rad;
float BODE_avgGain_dB;
uint32_t BODE_numSamples;
int32_t BODE_status; // -1 is disabled, even is configuration step, odd is measurement step


typedef struct {
	float DAC_frequency_Hz;
	uint32_t ADC_frequency_ID;
	uint32_t numMeasurements;
} BODE_MeasurementConfigurationTypedef;

#define BODE_NUM_CONFIGURATIONS		61
#define BODE_CURR_CONFIGURATION_IDX	(BODE_status >> 1)

extern BODE_MeasurementConfigurationTypedef BODE_configurations[BODE_NUM_CONFIGURATIONS];

typedef struct {
	float frequency_Hz;
	float phase_deg;
	float gain_dB;
} BODE_MeasurementTypedef;

BODE_MeasurementTypedef BODE_measurements[BODE_NUM_CONFIGURATIONS];

uint32_t BODE_measurementTime; // ms time it takes to run a complete measurement
uint32_t BODE_measurementStartTime; // ms

void BODE_init();
void BODE_processBuffer(); // called by BSP_ADC.c::ADC_update
void BODE_startMeasurement(uint32_t startIdx);

#endif /* BODE_H_ */
