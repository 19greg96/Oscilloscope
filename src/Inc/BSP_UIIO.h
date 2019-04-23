
#ifndef __BSP_UIIO_H
#define __BSP_UIIO_H


#include "main.h"
#include <stdint.h>




#define IN2_AC_Pin GPIO_PIN_8
#define IN2_AC_GPIO_Port GPIOA
#define IN1_AC_Pin GPIO_PIN_9
#define IN1_AC_GPIO_Port GPIOA
#define IN1_AA_Pin GPIO_PIN_10
#define IN1_AA_GPIO_Port GPIOA
#define IN2_AA_Pin GPIO_PIN_11
#define IN2_AA_GPIO_Port GPIOA

#define BTN_MAIN_Pin GPIO_PIN_4
#define BTN_MAIN_GPIO_Port GPIOB
#define BTN_1_Pin GPIO_PIN_5
#define BTN_1_GPIO_Port GPIOB
#define BTN_2_Pin GPIO_PIN_6
#define BTN_2_GPIO_Port GPIOB
#define BTN_3_Pin GPIO_PIN_7
#define BTN_3_GPIO_Port GPIOB
#define BTN_4_Pin GPIO_PIN_8
#define BTN_4_GPIO_Port GPIOB

#define LED_R_Pin GPIO_PIN_9
#define LED_R_GPIO_Port GPIOB

#define LED_G_Pin GPIO_PIN_10
#define LED_G_GPIO_Port GPIOB

#define ROT_ENC_A_Pin GPIO_PIN_6
#define ROT_ENC_A_GPIO_Port GPIOA
#define ROT_ENC_B_Pin GPIO_PIN_7
#define ROT_ENC_B_GPIO_Port GPIOA

struct UIIO_BtnTypedef;
typedef void (*UIIO_CallbackTypedef)(struct UIIO_BtnTypedef* caller);
typedef void (*UIIO_ScrollEventCallbackTypedef)(int32_t delta);

#define UIIO_BTN_STATE_LONG_PRESSED	0x02
#define UIIO_BTN_STATE_PRESSED		0x01
#define UIIO_BTN_STATE_RELEASED		0x00

#define UIIO_DEBOUNCE_TIME_MS		50
#define UIIO_LONG_PRESS_TIME_MS		500

#define UIIO_BTN_MAIN	0
#define UIIO_BTN_1		1
#define UIIO_BTN_2		2
#define UIIO_BTN_3		3
#define UIIO_BTN_4		4

typedef struct UIIO_BtnTypedef {
	uint16_t pin;
	GPIO_TypeDef* port;
	uint32_t pressTime;
	uint32_t releaseTime;
	uint8_t state;
	UIIO_CallbackTypedef onPress;		// called once on button press
	UIIO_CallbackTypedef onRelease;		// called once on button release
	UIIO_CallbackTypedef onLongPress;	// called once after 1000ms passes while button is pressed
	UIIO_CallbackTypedef onClick;		// called once if release occurs less than 1000ms after press
} UIIO_BtnTypedef;

volatile int16_t UIIO_pendingScrollCounter;
volatile int16_t UIIO_lastScrollCounter;
UIIO_ScrollEventCallbackTypedef UIIO_onScrollEvent;


// API functions
void UIIO_init();
void UIIO_update();
UIIO_BtnTypedef* UIIO_getButton(uint32_t id);

#endif
