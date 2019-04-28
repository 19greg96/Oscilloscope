/*
 * bode.c
 *
 *  Created on: 2019. ápr. 21.
 *      Author: Greg
 */


#include "bode.h"
#include "math.h"
#include "BSP_ADC.h"

arm_rfft_fast_instance_f32 BODE_FFTStruct;
float BODE_cFFTBuff[ADC_INPUT_BUFFER_SIZE / 2]; // half of ADC buffer is usable data, FFT is symmetric, so half of that is enough, but complex numbers use two floats per value	// TODO: reduce number of redundant ADC buffers
float BODE_tmpFFTBuff[ADC_INPUT_BUFFER_SIZE / 2]; // Temporary input buffer, so we don't overwrite input	// TODO: reduce number of redundant ADC buffers
float BODE_magFFTBuff[ADC_INPUT_BUFFER_SIZE / 4]; // complex -> magnitude, half the space is enough	// TODO: reduce number of redundant ADC buffers
/*
BODE_MeasurementConfigurationTypedef BODE_configurations[] = {
	{ .DAC_frequency_Hz = 1.0f,			.ADC_frequency_ID = 17,	.numMeasurements = 2},		// 0
	{ .DAC_frequency_Hz = 2.0f,			.ADC_frequency_ID = 16,	.numMeasurements = 2},		// 1
	{ .DAC_frequency_Hz = 5.0f,			.ADC_frequency_ID = 16,	.numMeasurements = 2},		// 2
	{ .DAC_frequency_Hz = 10.0f,		.ADC_frequency_ID = 15,	.numMeasurements = 4},		// 3
	{ .DAC_frequency_Hz = 20.0f,		.ADC_frequency_ID = 14,	.numMeasurements = 5},		// 4
	{ .DAC_frequency_Hz = 50.0f,		.ADC_frequency_ID = 12,	.numMeasurements = 5},		// 5
	{ .DAC_frequency_Hz = 100.0f,		.ADC_frequency_ID = 11,	.numMeasurements = 5},		// 6
	{ .DAC_frequency_Hz = 200.0f,		.ADC_frequency_ID = 10,	.numMeasurements = 5},		// 7	// 2ms
	{ .DAC_frequency_Hz = 500.0f,		.ADC_frequency_ID = 10,	.numMeasurements = 10},		// 8	// 2ms
	{ .DAC_frequency_Hz = 1000.0f,		.ADC_frequency_ID = 9,	.numMeasurements = 10},		// 9	// 683us
	{ .DAC_frequency_Hz = 2000.0f,		.ADC_frequency_ID = 8,	.numMeasurements = 10},		// 10	// 341us
	{ .DAC_frequency_Hz = 5000.0f,		.ADC_frequency_ID = 7,	.numMeasurements = 10},		// 11	// 171us
	{ .DAC_frequency_Hz = 10000.0f,		.ADC_frequency_ID = 6,	.numMeasurements = 10},		// 12	// 85us
	{ .DAC_frequency_Hz = 20000.0f,		.ADC_frequency_ID = 5,	.numMeasurements = 10},		// 13	// 57us
	{ .DAC_frequency_Hz = 50000.0f,		.ADC_frequency_ID = 3,	.numMeasurements = 10},		// 14	// 28us
	{ .DAC_frequency_Hz = 100000.0f,	.ADC_frequency_ID = 1,	.numMeasurements = 10},		// 15	// 14us // max frequency because of slew rate
	{ .DAC_frequency_Hz = 200000.0f,	.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 16	// 9us
	{ .DAC_frequency_Hz = 500000.0f,	.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 17	// 9us
	{ .DAC_frequency_Hz = 1000000.0f,	.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 18	// 9us
};*/
BODE_MeasurementConfigurationTypedef BODE_configurations[] = {
<<<<<<< HEAD
// 10^(x) where x increment is 0.1
=======
>>>>>>> 6fdfa67114a3318df925ebb261039420c6992317
	{ .DAC_frequency_Hz = 1.0f,				.ADC_frequency_ID = 17,	.numMeasurements = 2},		// 0
	{ .DAC_frequency_Hz = 1.25892541179f,	.ADC_frequency_ID = 17,	.numMeasurements = 2},		// 1
	{ .DAC_frequency_Hz = 1.58489319246f,	.ADC_frequency_ID = 17,	.numMeasurements = 2},		// 2
	{ .DAC_frequency_Hz = 1.99526231497f,	.ADC_frequency_ID = 17,	.numMeasurements = 2},		// 3
	{ .DAC_frequency_Hz = 2.51188643151f,	.ADC_frequency_ID = 16,	.numMeasurements = 2},		// 4
	{ .DAC_frequency_Hz = 3.16227766017f,	.ADC_frequency_ID = 16,	.numMeasurements = 2},		// 5
	{ .DAC_frequency_Hz = 3.98107170553f,	.ADC_frequency_ID = 16,	.numMeasurements = 2},		// 6
	{ .DAC_frequency_Hz = 5.01187233627f,	.ADC_frequency_ID = 16,	.numMeasurements = 2},		// 7
	{ .DAC_frequency_Hz = 6.3095734448f,	.ADC_frequency_ID = 16,	.numMeasurements = 2},		// 8
	{ .DAC_frequency_Hz = 7.94328234724f,	.ADC_frequency_ID = 15,	.numMeasurements = 4},		// 9
	{ .DAC_frequency_Hz = 10.0f,			.ADC_frequency_ID = 15,	.numMeasurements = 4},		// 10
	{ .DAC_frequency_Hz = 12.5892541179f,	.ADC_frequency_ID = 15,	.numMeasurements = 4},		// 11
	{ .DAC_frequency_Hz = 15.8489319246f,	.ADC_frequency_ID = 15,	.numMeasurements = 4},		// 12
	{ .DAC_frequency_Hz = 19.9526231497f,	.ADC_frequency_ID = 14,	.numMeasurements = 4},		// 13
	{ .DAC_frequency_Hz = 25.1188643151f,	.ADC_frequency_ID = 14,	.numMeasurements = 4},		// 14
	{ .DAC_frequency_Hz = 31.6227766017f,	.ADC_frequency_ID = 13,	.numMeasurements = 4},		// 15
	{ .DAC_frequency_Hz = 39.8107170553f,	.ADC_frequency_ID = 13,	.numMeasurements = 4},		// 16
	{ .DAC_frequency_Hz = 50.1187233627f,	.ADC_frequency_ID = 12,	.numMeasurements = 4},		// 17
	{ .DAC_frequency_Hz = 63.095734448f,	.ADC_frequency_ID = 12,	.numMeasurements = 4},		// 18
	{ .DAC_frequency_Hz = 79.4328234724f,	.ADC_frequency_ID = 12,	.numMeasurements = 4},		// 19
	{ .DAC_frequency_Hz = 100.0f,			.ADC_frequency_ID = 11,	.numMeasurements = 5},		// 20
	{ .DAC_frequency_Hz = 125.892541179f,	.ADC_frequency_ID = 11,	.numMeasurements = 5},		// 21
	{ .DAC_frequency_Hz = 158.489319246f,	.ADC_frequency_ID = 11,	.numMeasurements = 5},		// 22
	{ .DAC_frequency_Hz = 199.526231497f,	.ADC_frequency_ID = 11,	.numMeasurements = 5},		// 23
	{ .DAC_frequency_Hz = 251.188643151f,	.ADC_frequency_ID = 10,	.numMeasurements = 5},		// 24
	{ .DAC_frequency_Hz = 316.227766017f,	.ADC_frequency_ID = 10,	.numMeasurements = 5},		// 25
	{ .DAC_frequency_Hz = 398.107170553f,	.ADC_frequency_ID = 10,	.numMeasurements = 5},		// 26
	{ .DAC_frequency_Hz = 501.187233627f,	.ADC_frequency_ID = 10,	.numMeasurements = 5},		// 27
	{ .DAC_frequency_Hz = 630.95734448f,	.ADC_frequency_ID = 9,	.numMeasurements = 5},		// 28
	{ .DAC_frequency_Hz = 794.328234724f,	.ADC_frequency_ID = 9,	.numMeasurements = 5},		// 29
	{ .DAC_frequency_Hz = 1000.0f,			.ADC_frequency_ID = 9,	.numMeasurements = 10},		// 30	// 683us
	{ .DAC_frequency_Hz = 1258.92541179f,	.ADC_frequency_ID = 9,	.numMeasurements = 10},		// 31
	{ .DAC_frequency_Hz = 1584.89319246f,	.ADC_frequency_ID = 9,	.numMeasurements = 10},		// 32
	{ .DAC_frequency_Hz = 1995.26231497f,	.ADC_frequency_ID = 8,	.numMeasurements = 10},		// 33
	{ .DAC_frequency_Hz = 2511.88643151f,	.ADC_frequency_ID = 8,	.numMeasurements = 10},		// 34
	{ .DAC_frequency_Hz = 3162.27766017f,	.ADC_frequency_ID = 8,	.numMeasurements = 10},		// 35
	{ .DAC_frequency_Hz = 3981.07170553f,	.ADC_frequency_ID = 8,	.numMeasurements = 10},		// 36
	{ .DAC_frequency_Hz = 5011.87233627f,	.ADC_frequency_ID = 7,	.numMeasurements = 10},		// 37
	{ .DAC_frequency_Hz = 6309.5734448f,	.ADC_frequency_ID = 7,	.numMeasurements = 10},		// 38
	{ .DAC_frequency_Hz = 7943.28234724f,	.ADC_frequency_ID = 7,	.numMeasurements = 10},		// 39
	{ .DAC_frequency_Hz = 10000.0f,			.ADC_frequency_ID = 6,	.numMeasurements = 10},		// 40	// 85us
	{ .DAC_frequency_Hz = 12589.2541179f,	.ADC_frequency_ID = 6,	.numMeasurements = 10},		// 41
	{ .DAC_frequency_Hz = 15848.9319246f,	.ADC_frequency_ID = 6,	.numMeasurements = 10},		// 42
	{ .DAC_frequency_Hz = 19952.6231497f,	.ADC_frequency_ID = 5,	.numMeasurements = 10},		// 43
	{ .DAC_frequency_Hz = 25118.8643151f,	.ADC_frequency_ID = 5,	.numMeasurements = 10},		// 44
	{ .DAC_frequency_Hz = 31622.7766017f,	.ADC_frequency_ID = 5,	.numMeasurements = 10},		// 45
	{ .DAC_frequency_Hz = 39810.7170553f,	.ADC_frequency_ID = 4,	.numMeasurements = 10},		// 46
	{ .DAC_frequency_Hz = 50118.7233627f,	.ADC_frequency_ID = 3,	.numMeasurements = 10},		// 47
	{ .DAC_frequency_Hz = 63095.734448f,	.ADC_frequency_ID = 3,	.numMeasurements = 10},		// 48
	{ .DAC_frequency_Hz = 79432.8234724f,	.ADC_frequency_ID = 3,	.numMeasurements = 10},		// 49
	{ .DAC_frequency_Hz = 100000.0f,		.ADC_frequency_ID = 1,	.numMeasurements = 10},		// 50	// 14us
	{ .DAC_frequency_Hz = 125892.541179f,	.ADC_frequency_ID = 1,	.numMeasurements = 10},		// 51
	{ .DAC_frequency_Hz = 158489.319246f,	.ADC_frequency_ID = 1,	.numMeasurements = 10},		// 52
	{ .DAC_frequency_Hz = 199526.231497f,	.ADC_frequency_ID = 1,	.numMeasurements = 10},		// 53
	{ .DAC_frequency_Hz = 251188.643151f,	.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 54
	{ .DAC_frequency_Hz = 316227.766017f,	.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 55
	{ .DAC_frequency_Hz = 398107.170553f,	.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 56
	{ .DAC_frequency_Hz = 501187.233627f,	.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 57
	{ .DAC_frequency_Hz = 630957.34448f,	.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 58
	{ .DAC_frequency_Hz = 794328.234724f,	.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 59
	{ .DAC_frequency_Hz = 1000000.0f,		.ADC_frequency_ID = 0,	.numMeasurements = 10},		// 60	// 9us
};
// 1.0f
// 1.58489319246f
// 2.5f
// 4.0f
// 6.3
// 10

void BODE_init() {
	BODE_avgPhase_rad = 0.0f;
	BODE_avgGain_dB = 0.0f;
	BODE_numSamples = 0;
	BODE_status = -1; // -1 is disabled, even is configuration step, odd is measurement step
	
	arm_rfft_fast_init_f32(&BODE_FFTStruct, ADC_INPUT_BUFFER_SIZE / 2);
	
	for (uint32_t i = 0; i < BODE_NUM_CONFIGURATIONS; i ++) {
		float sampleTime = 1.0f / ADC_FrequencyTable[BODE_configurations[i].ADC_frequency_ID].frequency;
		float numSamples = ((float)ADC_INPUT_BUFFER_SIZE);
		float numMeasurements = (float)BODE_configurations[i].numMeasurements + 1.0f; // error compensation
		BODE_measurementTime += (uint32_t)((sampleTime * numSamples * numMeasurements) * 1000.0f); // plus processing time
	}
}

void BODE_getMaxBinParams(float* timeDomain, uint32_t* out_binIndex, float* out_mag, float* out_phase_rad) {
	memcpy(BODE_tmpFFTBuff, timeDomain, (ADC_INPUT_BUFFER_SIZE / 2) * sizeof(float));
	arm_rfft_fast_f32(&BODE_FFTStruct, BODE_tmpFFTBuff, BODE_cFFTBuff, 0);
/*

The FFT of a real N-point sequence has even symmetry in the frequency domain.
The second half of the data equals the conjugate of the first half flipped in frequency.
Looking at the data, we see that we can uniquely represent the FFT using only N/2 complex numbers.
These are packed into the output array in alternating real and imaginary components: 

X = { real[0], imag[0], real[1], imag[1], real[2], imag[2] ... real[(N/2)-1], imag[(N/2)-1 } 

It happens that the first complex number (real[0], imag[0]) is actually all real.
real[0] represents the DC offset, and imag[0] should be 0.
(real[1], imag[1]) is the fundamental frequency, (real[2], imag[2]) is the first harmonic and so on. 

*/
	memcpy(BODE_tmpFFTBuff, BODE_cFFTBuff, (ADC_INPUT_BUFFER_SIZE / 2) * sizeof(float));
	arm_cmplx_mag_squared_f32(BODE_tmpFFTBuff, BODE_magFFTBuff, ADC_INPUT_BUFFER_SIZE / 4); // faster than calculating the square root for all elements
	/*
	
Computes the magnitude squared of the elements of a complex data vector.
The pSrc points to the source data and pDst points to the where the result should be written.
numSamples specifies the number of complex samples in the input array and the data is stored in an interleaved fashion (real, imag, real, imag, ...).
The input array has a total of 2*numSamples values; the output array has a total of numSamples values.
The underlying algorithm is used:
for (n = 0; n < numSamples; n++) {
pDst[n] = sqrt(pSrc[(2*n)+0]^2 + pSrc[(2*n)+1]^2);
}
	
	*/
	BODE_magFFTBuff[0] = 0; // we don't care about DC (it should be 0 anyway)
	float maxBinValue;
	uint32_t maxBinIndex;
	float maxBinPhase;
	arm_max_f32(BODE_magFFTBuff, ADC_INPUT_BUFFER_SIZE / 4, &maxBinValue, &maxBinIndex);
	// maxBinValue = sqrtf(maxBinValue); // return power
	maxBinPhase = atan2f(BODE_cFFTBuff[maxBinIndex * 2 + 1], BODE_cFFTBuff[maxBinIndex * 2]); // find the complex number with the largest magnitude
	
	*out_binIndex = maxBinIndex;
	*out_mag = maxBinValue;
	*out_phase_rad = maxBinPhase;
}

void BODE_startMeasurement() {
	// TODO: save settings before BODE measurement
	HAL_GPIO_WritePin(IN1_AC_GPIO_Port, IN1_AC_Pin, GPIO_PIN_SET); // Disable AC, AA for all channels, so they do not
	HAL_GPIO_WritePin(IN1_AA_GPIO_Port, IN1_AA_Pin, GPIO_PIN_SET); // interfere with measurement
	// HAL_GPIO_WritePin(IN2_AC_GPIO_Port, IN2_AC_Pin, GPIO_PIN_SET); // disabled for testing
	// HAL_GPIO_WritePin(IN2_AA_GPIO_Port, IN2_AA_Pin, GPIO_PIN_SET);
	////// ADC_setTriggerLevel(ADC_VMAX / 2); // update trigger value for new offset
	SCOPE_triggerMode = SCOPE_TRIGGER_DISABLE;
	SCOPE_triggerEdge = SCOPE_TRIGGER_EDGE_DISABLED;
	SCOPE_triggerSource = SCOPE_TRIGGER_SOURCE_DISABLED;
	
	// TODO: Disable GUI except for cancel button
	BODE_status = 0;
	BODE_measurementStartTime = HAL_GetTick();
}
void BODE_endMeasurement() {
	// TODO: reset settings to saved values
	// TODO: enable GUI, remove cancel button
	
	BODE_status = -1;
}

float avarageAnglesRad(float a_rad, float b_rad) {
	float x = cos(a_rad) + cos(b_rad);
	float y = sin(a_rad) + sin(b_rad);
	
	return atan2f(y, x);
}

void BODE_processBuffer(float* inputBuffer_V, float* outputBuffer_V) {
	if (BODE_status == -1) {
		return;
	}
	// TODO: order of steps waists conversion cycles
	if ((BODE_status & 0x01) == 0) { // configuration step
		if (BODE_CURR_CONFIGURATION_IDX == BODE_NUM_CONFIGURATIONS) {
			BODE_endMeasurement();
			return;
		}
		
		DAC_configure(DAC_CHANNEL_1, 0, BODE_configurations[BODE_CURR_CONFIGURATION_IDX].DAC_frequency_Hz, DAC_WAVEFORM_SINE, 0.3f, 3.0f, 0.0f, 0.0f);
		DAC_enable(DAC_CHANNEL_1);
		ADC_SetFrequencyID(BODE_configurations[BODE_CURR_CONFIGURATION_IDX].ADC_frequency_ID);
		
		// SCOPE_startConversion();
		
		BODE_status++; // goto measurement step
		return;
	}
	if (BODE_numSamples >= BODE_configurations[BODE_CURR_CONFIGURATION_IDX].numMeasurements) {
		BODE_measurements[BODE_CURR_CONFIGURATION_IDX].frequency_Hz = BODE_configurations[BODE_CURR_CONFIGURATION_IDX].DAC_frequency_Hz;
		BODE_measurements[BODE_CURR_CONFIGURATION_IDX].gain_dB = BODE_avgGain_dB;
		BODE_measurements[BODE_CURR_CONFIGURATION_IDX].phase_deg = BODE_avgPhase_rad * 180.0f / M_PI;
		
		BODE_avgPhase_rad = 0.0f;
		BODE_avgGain_dB = 0.0f;
		BODE_numSamples = 0;
		
		BODE_status++; // goto configuration step
		return;
	}
	char tmpBuff[32];
	
	uint32_t binIdx1, binIdx2;
	float mag1, mag2; // power
	float phase_rad1, phase_rad2;
	// TODO: in theory, full buffer can be used here to double the frequency resolution of the FFT
	BODE_getMaxBinParams(inputBuffer_V, &binIdx1, &mag1, &phase_rad1); // channel 1 is DAC output fed straight back to ADC (Uin)
	BODE_getMaxBinParams(outputBuffer_V, &binIdx2, &mag2, &phase_rad2); // channel 2 goes through passive network (Uout)
	
	// dF = Fs / N
	float dF_Hz = ADC_GetFrequencySettings()->frequency / (float)(ADC_INPUT_BUFFER_SIZE / 2); // frequency resolution of DFT = Fs / N
	float f1_Hz = dF_Hz * (float)binIdx1;
	float f2_Hz = dF_Hz * (float)binIdx2;
	
	if (fabsf(f1_Hz - f2_Hz) > 0.5f) {
		return; // failed to measure
		// TODO: after a certain number of failed measurements, stop measurement
	}
	
	float phase_rad = phase_rad2 - phase_rad1;
	phase_rad += (phase_rad > M_PI) ? -M_TWOPI : (phase_rad < -M_PI) ? M_TWOPI : 0.0f; // thx https://stackoverflow.com/questions/1878907/the-smallest-difference-between-2-angles
	float gain_dB = 10.0f * log10f(mag2 / mag1); // magnitudes are squared
	
	if (gain_dB < -200.0f) {
		gain_dB = -200.0f;
	}
	if (gain_dB > 200.0f) {
		gain_dB = 200.0f;
	}
	
	// Cumulative moving average
	// BODE_avgPhase_rad = ((float)phase_rad + (float)BODE_numSamples * BODE_avgPhase_rad) / ((float)BODE_numSamples + 1.0f);
	// thx https://stackoverflow.com/questions/491738/how-do-you-calculate-the-average-of-a-set-of-circular-data
	BODE_avgPhase_rad = avarageAnglesRad(BODE_avgPhase_rad, phase_rad); // WARNING: this does not calculate CMA!
	BODE_avgGain_dB = ((float)gain_dB + (float)BODE_numSamples * BODE_avgGain_dB) / ((float)BODE_numSamples + 1.0f);
	BODE_numSamples++;
	
	
	sprintf(tmpBuff, "phase %ld\n", (int32_t)(phase_rad * (180.0f / M_PI) * 100.0f));
	UART_writeString(tmpBuff);
	
	sprintf(tmpBuff, "amp %ld\n", (int32_t)(gain_dB * 100.0f));
	UART_writeString(tmpBuff);
	
	sprintf(tmpBuff, "Fin %ld\n", (int32_t)(f1_Hz * 100.0f));
	UART_writeString(tmpBuff);
	
	sprintf(tmpBuff, "Fout %ld\n", (int32_t)(f2_Hz * 100.0f));
	UART_writeString(tmpBuff);
}

