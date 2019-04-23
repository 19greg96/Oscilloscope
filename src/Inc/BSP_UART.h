
#ifndef __BSP_UART_H
#define __BSP_UART_H


#include "main.h"
#include <stdint.h>

UART_HandleTypeDef huart2;

#define USART_TX_Pin		GPIO_PIN_2
#define USART_TX_GPIO_Port	GPIOA
#define USART_RX_Pin		GPIO_PIN_3
#define USART_RX_GPIO_Port	GPIOA

void MX_USART2_UART_Init(void);
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);

void USART2_IRQHandler();
void BSP_USART2_WriteString(uint8_t * str);

#define UART_INPUT_BUFFER_SIZE	64
#define UART_INPUT_BUFFER_MASK	(UART_INPUT_BUFFER_SIZE-1)
volatile uint32_t recvd;
volatile uint8_t rxBuffer[5];
volatile uint8_t UART_inputBuffer[UART_INPUT_BUFFER_SIZE];
volatile uint32_t UART_inputBufferWritePos;
uint32_t UART_inputBufferReadPos;

// API functions
void UART_init();
void UART_update();

uint32_t UART_available(); // get number of bytes in input buffer
uint8_t UART_read8();
uint16_t UART_read16();
uint32_t UART_read32();
float UART_readFloat();
void UART_readBuff(uint8_t* dst, uint32_t len);
// blocking calls:
void UART_write8(uint8_t val);
void UART_write16(uint16_t val);
void UART_write32(uint32_t val);
void UART_writeBuff(uint8_t* src, uint32_t len);
void UART_writeString(char* string);

#endif
