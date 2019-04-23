/* USER CODE BEGIN Header */
/**
	******************************************************************************
	* @file					 : main.c
	* @brief					: Main program body
	******************************************************************************
	** This notice applies to any and all portions of this file
	* that are not between comment pairs USER CODE BEGIN and
	* USER CODE END. Other portions of this file, whether 
	* inserted by the user or by software development tools
	* are owned by their respective copyright owners.
	*
	* COPYRIGHT(c) 2019 STMicroelectronics
	*
	* Redistribution and use in source and binary forms, with or without modification,
	* are permitted provided that the following conditions are met:
	*	 1. Redistributions of source code must retain the above copyright notice,
	*			this list of conditions and the following disclaimer.
	*	 2. Redistributions in binary form must reproduce the above copyright notice,
	*			this list of conditions and the following disclaimer in the documentation
	*			and/or other materials provided with the distribution.
	*	 3. Neither the name of STMicroelectronics nor the names of its contributors
	*			may be used to endorse or promote products derived from this software
	*			without specific prior written permission.
	*
	* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	*
	******************************************************************************
	*/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>

#include "GUI.h"
#include "BSP_GLCD.h"
#include "GUI_font_legible3x5_6pt.h"
#include "GUI_setup.h"
#include "bode.h"

float graphBuffer1_V[ADC_INPUT_BUFFER_SIZE / 2];	// TODO: reduce number of redundant ADC buffers
float graphBuffer2_V[ADC_INPUT_BUFFER_SIZE / 2];	// TODO: reduce number of redundant ADC buffers

int main(void) {
	HAL_HardwareInit();
	
	GUI_init();
	GUI_setup_init(graphBuffer1_V, graphBuffer2_V);
	BODE_init();
	
	while (1) {
		UART_update(); // read commands from uart
		ADC_update(graphBuffer1_V, graphBuffer2_V); // process ADC buffer
		
		UIIO_update(); // update button states
		GUI_render(); // draw GUI elements to buffer
		
		GLCD_update(); // render buffer on GLCD
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/




