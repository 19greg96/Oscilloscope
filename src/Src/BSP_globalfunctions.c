#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "BSP_globalfunctions.h"


volatile int sys_delay = 0; // decremented by 1 every us if larger than zero

TIM_HandleTypeDef Tim4Handle;

void GlobalFunctions_Init(void) {
	__TIM4_CLK_ENABLE();

	Tim4Handle.Instance = TIM4;
	Tim4Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	Tim4Handle.Init.CounterMode = TIM_COUNTERMODE_DOWN;
	Tim4Handle.Init.Prescaler = 1;
	Tim4Handle.Init.Period = 180; // 180 value to measure 1us
	Tim4Handle.State = HAL_TIM_STATE_RESET;
	HAL_TIM_Base_Init(&Tim4Handle);
	HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
	HAL_TIM_Base_Start_IT(&Tim4Handle);
}

void Sys_DelayUs(int us) { // blocking delay
	sys_delay = us;
	while(sys_delay);
}

void Sys_DelayMs(int ms) { // blocking delay
	sys_delay = ms*1000;
	while(sys_delay);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *handle) {
	UNUSED(handle);
	
	if (sys_delay>0) {
		sys_delay--;
	}
}

void TIM4_IRQHandler(void) {
	HAL_TIM_IRQHandler(&Tim4Handle);
}
