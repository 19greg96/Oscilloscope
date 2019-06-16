
#ifndef __BSP_GLCD_H
#define __BSP_GLCD_H


#include "main.h"
#include <stdint.h>


#define GLCD_DI_COMMAND	0
#define GLCD_DI_DATA	1

#define GLCD_CS_1	1
#define GLCD_CS_2	2
#define GLCD_CS_12	(GLCD_CS_1 | GLCD_CS_2)

#define GLCD_REG_DISPLAY_STATE		0x3E
#define GLCD_REG_START_LINE			0xC0
#define GLCD_REG_COLUMN_ADDR		0x40
#define GLCD_REG_PAGE_ADDR			0xB8

#define GLCD_DISPLAY_STATE_ON		0x01
#define GLCD_DISPLAY_STATE_OFF		0x01

#define GLCD_START_LINE_MASK		0x3F
#define GLCD_COLUMN_ADDR_MASK		0x3F
#define GLCD_PAGE_ADDR_MASK			0x07

#define GLCD_DATA_Port	GPIOC
#define GLCD_DB0_Pin	GPIO_PIN_0
#define GLCD_DB1_Pin	GPIO_PIN_1
#define GLCD_DB2_Pin	GPIO_PIN_2
#define GLCD_DB3_Pin	GPIO_PIN_3
#define GLCD_DB4_Pin	GPIO_PIN_4
#define GLCD_DB5_Pin	GPIO_PIN_5
#define GLCD_DB6_Pin	GPIO_PIN_6
#define GLCD_DB7_Pin	GPIO_PIN_7

#define GLCD_DI_Pin		GPIO_PIN_13
#define GLCD_DI_Port	GPIOB
#define GLCD_RW_Pin		GPIO_PIN_14
#define GLCD_RW_Port	GPIOB
#define GLCD_E_Pin		GPIO_PIN_9
#define GLCD_E_Port		GPIOC
#define GLCD_EN_Pin		GPIO_PIN_10
#define GLCD_EN_Port	GPIOC
#define GLCD_RST_Pin	GPIO_PIN_11
#define GLCD_RST_Port	GPIOC
#define GLCD_CS1_Pin	GPIO_PIN_12
#define GLCD_CS1_Port	GPIOC
#define GLCD_CS2_Pin	GPIO_PIN_15
#define GLCD_CS2_Port	GPIOB

#define GLCD_BACKLIGHT_Pin			GPIO_PIN_8
#define GLCD_BACKLIGHT_Port			GPIOC

TIM_HandleTypeDef htim4; // Signal timing
TIM_HandleTypeDef htim8; // PWM

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm);
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm);
void MX_TIM4_Init(void);
void MX_TIM8_Init(void);

void MX_GLCD_GPIO_Init(void);

// API functions
void GLCD_init(uint32_t w, uint32_t h);
void GLCD_update();


void GLCDEN(int newState);
void GLCD_Write(int8_t cs_s, int8_t d_i, int8_t g_data);
void GLCD_Write_Block(int8_t m_data, int8_t cX, int8_t cY);
/*
void GLCD_Write_Char(char cPlace,char cX,char cY);
void GLCD_WriteString(const char* string,char X, char Y);
*/
void GLCD_setBacklight(float d);


#endif
