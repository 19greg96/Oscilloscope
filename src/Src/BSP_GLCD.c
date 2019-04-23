
#include "BSP_GLCD.h"
#include "BSP_globalfunctions.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>


uint8_t* GLCD_old_buffer;
uint8_t* GLCD_new_buffer;
uint32_t GLCD_buffer_size;


void GLCD_init(uint32_t w, uint32_t h) {
	GLCD_width = w;
	GLCD_height = h;
	
	GLCD_buffer_size = GLCD_width * GLCD_height / 8;
	GLCD_new_buffer = (uint8_t*)malloc(sizeof(uint8_t) * GLCD_buffer_size); // data size is 8
	GLCD_old_buffer = (uint8_t*)malloc(sizeof(uint8_t) * GLCD_buffer_size);
	
	MX_GLCD_GPIO_Init();
	MX_TIM8_Init();
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
	// GLCD_setBacklight(0x000F);
	
	GLCDEN(1);
	
	// Init GLCD functions
	HAL_GPIO_WritePin(GLCD_E_Port, GLCD_E_Pin, GPIO_PIN_RESET);			// GLCD_E = 0
	HAL_GPIO_WritePin(GLCD_RST_Port, GLCD_RST_Pin, GPIO_PIN_SET);	// GLCD_RESET = 1
	
	GLCD_Write(GLCD_CS_12, GLCD_DI_COMMAND, GLCD_REG_DISPLAY_STATE | GLCD_DISPLAY_STATE_ON);	// GLCD on
	GLCD_Write(GLCD_CS_12, GLCD_DI_COMMAND, GLCD_REG_START_LINE | (0 & GLCD_START_LINE_MASK));	// GLCD Start Line
	
	GLCD_clear();
}

void GLCD_update() {
	/*
	for (uint32_t i = 0; i < GLCD_buffer_size; i ++) {
		int8_t newBlock = GLCD_new_buffer[i];
		if (newBlock != GLCD_old_buffer[i]) {
			GLCD_Write_Block(newBlock, i & 0x7F, (i >> 7) & 0x07);
			// x goes from 0 to 127
			// y goes from 0 to 7
			
			GLCD_old_buffer[i] = newBlock;
		}
	}
	*/
	
	int8_t x, y;
	int8_t newBlock;
	uint32_t pos = 0;
	for (y = 0; y < 8; y ++) {
		GLCD_Write(GLCD_CS_1, GLCD_DI_COMMAND, GLCD_REG_COLUMN_ADDR);
		GLCD_Write(GLCD_CS_1, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | y));
		for (x = 0; x < 64; x ++) {
			newBlock = GLCD_new_buffer[pos];
			GLCD_Write(GLCD_CS_1, GLCD_DI_DATA, newBlock);
			GLCD_old_buffer[pos] = newBlock;
			pos++;
		}
		GLCD_Write(GLCD_CS_2, GLCD_DI_COMMAND, GLCD_REG_COLUMN_ADDR);
		GLCD_Write(GLCD_CS_2, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | y));
		for (x = 0; x < 64; x ++) {
			newBlock = GLCD_new_buffer[pos];
			GLCD_Write(GLCD_CS_2, GLCD_DI_DATA, newBlock);
			GLCD_old_buffer[pos] = newBlock;
			pos++;
		}
	}
}
void GLCD_clear() {
	for (uint32_t i = 0; i < GLCD_buffer_size; i ++) {
		GLCD_new_buffer[i] = 0;
		GLCD_old_buffer[i] = 0xFF;
	}
}


void GLCD_flood_fill_util(int32_t x, int32_t y, uint8_t prevC, uint8_t newC);
void GLCD_draw_circle_util(int32_t x0, int32_t y0, uint32_t r, uint8_t cornername, uint8_t color);
void GLCD_fill_circle_util(int32_t x0, int32_t y0, uint32_t r, uint8_t cornername, int32_t delta, uint8_t color);

uint8_t GLCD_set_pixel(int32_t x, int32_t y, uint8_t v) {
	if (x < 0 || x >= GLCD_width || y < 0 || y >= GLCD_height) {
		return -1;
	}
	if (v == GLCD_COLOR_ON) {
		//GLCD_new_buffer[30 * y + (x >> 3)] |= 0x80 >> (x & 7);
		GLCD_new_buffer[x + ((y >> 3) << 7)] |= 0x01 << (y & 7);
	} else if (v == GLCD_COLOR_INVERT) {
		//GLCD_new_buffer[30 * y + (x >> 3)] ^= 0x80 >> (x & 7);
		GLCD_new_buffer[x + ((y >> 3) << 7)] ^= 0x01 << (y & 7);
	} else if (v == GLCD_COLOR_OFF) {
		//GLCD_new_buffer[30 * y + (x >> 3)] &= ~(0x80 >> (x & 7));
		GLCD_new_buffer[x + ((y >> 3) << 7)] &= ~(0x01 << (y & 7));
	}
	return v;
}
uint8_t GLCD_get_pixel(int32_t x, int32_t y) {
	if (x < 0 || x > GLCD_width || y < 0 || y > GLCD_height) {
		return -1;
	}
	// return (GLCD_new_buffer[30 * y + (x >> 3)] >> ((~x) & 7)) & 1;
	return (GLCD_new_buffer[x + ((y >> 3) << 7)] >> (y & 7)) & 1;
}

void GLCD_flood_fill_util(int32_t x, int32_t y, uint8_t prevC, uint8_t newC) {
	if (GLCD_get_pixel(x, y) != prevC) {
		return;
	}
	
	GLCD_set_pixel(x, y, newC);
	
	GLCD_flood_fill_util(x + 1, y, prevC, newC);
	GLCD_flood_fill_util(x - 1, y, prevC, newC);
	GLCD_flood_fill_util(x,	y + 1, prevC, newC);
	GLCD_flood_fill_util(x,	y - 1, prevC, newC);
}
void GLCD_flood_fill(int32_t x, int32_t y, uint8_t newC) {
	GLCD_flood_fill_util(x, y, !newC, newC);
}


// Draw a circle outline
void GLCD_draw_circle(int32_t x0, int32_t y0, uint32_t r, uint8_t color) {
	GLCD_set_pixel(x0, y0+r, color);
	GLCD_set_pixel(x0, y0-r, color);
	GLCD_set_pixel(x0+r, y0, color);
	GLCD_set_pixel(x0-r, y0, color);
	
	GLCD_draw_circle_util(x0, y0, r, 15, color);
}

void GLCD_draw_circle_util(int32_t x0, int32_t y0, uint32_t r, uint8_t cornername, uint8_t color) {
	int32_t f		 = 1 - r;
	int32_t ddF_x = 1;
	int32_t ddF_y = -2 * r;
	int32_t x		 = 0;
	int32_t y		 = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			GLCD_set_pixel(x0 + x, y0 + y, color);
			GLCD_set_pixel(x0 + y, y0 + x, color);
		} 
		if (cornername & 0x2) {
			GLCD_set_pixel(x0 + x, y0 - y, color);
			GLCD_set_pixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			GLCD_set_pixel(x0 - y, y0 + x, color);
			GLCD_set_pixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			GLCD_set_pixel(x0 - y, y0 - x, color);
			GLCD_set_pixel(x0 - x, y0 - y, color);
		}
	}
}

void GLCD_fill_circle(int32_t x0, int32_t y0, uint32_t r, uint8_t color) {
	GLCD_draw_line(x0, y0-r, x0, 2*r, color);
	GLCD_fill_circle_util(x0, y0, r, 3, 0, color);
}

// Used to draw circles and roundrects
void GLCD_fill_circle_util(int32_t x0, int32_t y0, uint32_t r, uint8_t cornername, int32_t delta, uint8_t color) {
	int32_t f		 = 1 - r;
	int32_t ddF_x = 1;
	int32_t ddF_y = -2 * r;
	int32_t x		 = 0;
	int32_t y		 = r;

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f		 += ddF_y;
		}
		x++;
		ddF_x += 2;
		f		 += ddF_x;

		if (cornername & 0x1) {
			GLCD_draw_line(x0+x, y0-y, x0+x, 2*y+delta, color);
			GLCD_draw_line(x0+y, y0-x, x0+y, 2*x+delta, color);
		}
		if (cornername & 0x2) {
			GLCD_draw_line(x0-x, y0-y, x0-x, 2*y+delta, color);
			GLCD_draw_line(x0-y, y0-x, x0-y, 2*x+delta, color);
		}
	}
}

void GLCD_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint8_t color) {
	GLCD_draw_line_style(x0, y0, x1, y1, GLCD_LINE_STYLE_SOLID, color);
}
// Bresenham's algorithm - thx wikpedia
void GLCD_draw_line_style(int32_t x0, int32_t y0, int32_t x1, int32_t y1, GLCD_LineStyle style, uint8_t color) {
	if (style == GLCD_LINE_STYLE_NONE) {
		return;
	}
	int32_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		// std::swap(x0, y0);
		int32_t tmp = x0;
		x0 = y0;
		y0 = tmp;
		
		// std::swap(x1, y1);
		tmp = x1;
		x1 = y1;
		y1 = tmp;
	}

	if (x0 > x1) {
		// std::swap(x0, x1);
		int32_t tmp = x0;
		x0 = x1;
		x1 = tmp;
		
		// std::swap(y0, y1);
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	int32_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int32_t err = dx / 2;
	int32_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}
	uint8_t enable = 1;
	for (; x0 <= x1; x0 ++) {
		if (style == GLCD_LINE_STYLE_DOTS) {
			enable = (x0 & 1);
		} else if (style == GLCD_LINE_STYLE_DASHES) {
			enable = (x0 & 2);
		} else if (style == GLCD_LINE_STYLE_SPARSE_DOTS) {
			enable = ((x0 & 3) == 3);
		}
		if (enable) {
			if (steep) {
				GLCD_set_pixel(y0, x0, color);
			} else {
				GLCD_set_pixel(x0, y0, color);
			}
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

// Draw a rectangle
void GLCD_draw_rect(int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t color) {
	w--;
	h--;
	GLCD_draw_line(x, y, x + w, y, color); // top
	GLCD_draw_line(x, y + h, x + w, y + h, color); // bottom
	GLCD_draw_line(x, y, x, y + h, color); // left
	GLCD_draw_line(x + w, y, x + w, y + h, color); // right
}

void GLCD_fill_rect(int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t color) {
	h --;
	if (w == 0 || h == 0) {
		return;
	}
	for (int32_t i = x; i < x + w; i ++) {
		GLCD_draw_line(i, y, i, y + h, color);
	}
}

// Draw a rounded rectangle
void GLCD_draw_round_rect(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t r, uint8_t color) {
	// smarter version
	GLCD_draw_line(x + r, y, x + w - 2 * r, y, color); // top
	GLCD_draw_line(x + r, y + h - 1, x + w - 2 * r, y + h - 1, color); // bottom
	GLCD_draw_line(x, y + r, x, y + h - 2 * r, color); // left
	GLCD_draw_line(x + w - 1, y + r, x + w - 1, y + h - 2 * r, color); // right
	// draw four corners
	GLCD_draw_circle_util(x+r, y+r, r, 1, color);
	GLCD_draw_circle_util(x+w-r-1, y+r, r, 2, color);
	GLCD_draw_circle_util(x+w-r-1, y+h-r-1, r, 4, color);
	GLCD_draw_circle_util(x+r, y+h-r-1, r, 8, color);
}

// Fill a rounded rectangle
void GLCD_fill_round_rect(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t r, uint8_t color) {
	// smarter version
	GLCD_fill_rect(x+r, y, w-2*r, h, color);

	// draw four corners
	GLCD_fill_circle_util(x+w-r-1, y+r, r, 1, h-2*r-1, color);
	GLCD_fill_circle_util(x+r, y+r, r, 2, h-2*r-1, color);
}

// Draw a triangle
void GLCD_draw_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color) {
	GLCD_draw_line(x0, y0, x1, y1, color);
	GLCD_draw_line(x1, y1, x2, y2, color);
	GLCD_draw_line(x2, y2, x0, y0, color);
}

// Fill a triangle
void GLCD_fill_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color) {
	int32_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	int32_t tmp;
	if (y0 > y1) {
		// std::swap(y0, y1);
		tmp = y0;
		y0 = y1;
		y1 = tmp;
		
		// std::swap(x0, x1);
		tmp = x0;
		x0 = x1;
		x1 = tmp;
	}
	if (y1 > y2) {
		// std::swap(y2, y1);
		tmp = y2;
		y2 = y1;
		y1 = tmp;
		
		// std::swap(x2, x1);
		tmp = x2;
		x2 = x1;
		x1 = tmp;
	}
	if (y0 > y1) {
		// std::swap(y0, y1);
		tmp = y0;
		y0 = y1;
		y1 = tmp;
		
		// std::swap(x0, x1);
		tmp = x0;
		x0 = x1;
		x1 = tmp;
	}

	if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if(x1 < a)			a = x1;
		else if(x1 > b) b = x1;
		if(x2 < a)			a = x2;
		else if(x2 > b) b = x2;
		// drawFastHLine(a, y0, b-a+1, color);
		GLCD_draw_line(a, y0, b + 1, y0, color);
		return;
	}

	int32_t
		dx01 = x1 - x0,
		dy01 = y1 - y0,
		dx02 = x2 - x0,
		dy02 = y2 - y0,
		dx12 = x2 - x1,
		dy12 = y2 - y1;
	int32_t
		sa	 = 0,
		sb	 = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.	If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if(y1 == y2) last = y1;	 // Include y1 scanline
	else				 last = y1-1; // Skip it

	for(y=y0; y<=last; y++) {
		a	 = x0 + sa / dy01;
		b	 = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) {
			// std::swap(a,b);
			tmp = a;
			a = b;
			b = tmp;
		}
		// drawFastHLine(a, y, b-a+1, color);
		GLCD_draw_line(a, y, b + 1, y, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.	This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y<=y2; y++) {
		a	 = x1 + sa / dy12;
		b	 = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) {
			// std::swap(a,b);
			tmp = a;
			a = b;
			b = tmp;
		}
		// drawFastHLine(a, y, b-a+1, color);
		GLCD_draw_line(a, y, b + 1, y, color);
	}
}




void GLCD_setBacklight(float d) {
	__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint32_t)(d * (float)0xFFFF));
}
void GLCDEN(int newState) {
	if (newState) {
		HAL_GPIO_WritePin(GLCD_EN_Port, GLCD_EN_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(GLCD_EN_Port, GLCD_EN_Pin, GPIO_PIN_SET);
	}
}
/**
* 	cs_s, 1 = CS1, 2 = CS2, 3 = CS1&CS2
* 	d_i, 0 = instruction, 1 = data
*/
void GLCD_Write(int8_t cs_s, int8_t d_i, int8_t g_data) {
	uint16_t data = 0x0000;
	switch (cs_s) {
		case GLCD_CS_1:
			HAL_GPIO_WritePin(GLCD_CS1_Port, GLCD_CS1_Pin, GPIO_PIN_SET);
			break;
		case GLCD_CS_2:
			HAL_GPIO_WritePin(GLCD_CS2_Port, GLCD_CS2_Pin, GPIO_PIN_SET);
			break;
		case GLCD_CS_12:
			HAL_GPIO_WritePin(GLCD_CS1_Port, GLCD_CS1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GLCD_CS2_Port, GLCD_CS2_Pin, GPIO_PIN_SET);
			break;
	}
	switch (d_i) {
		case GLCD_DI_COMMAND:
			HAL_GPIO_WritePin(GLCD_DI_Port, GLCD_DI_Pin, GPIO_PIN_RESET);
			break;
		case GLCD_DI_DATA:
			HAL_GPIO_WritePin(GLCD_DI_Port, GLCD_DI_Pin, GPIO_PIN_SET);
			break;
	}

	data = GLCD_DATA_Port->IDR;
	data &= 0xFF00;
	data |= g_data & 0x00FF;
	GLCD_DATA_Port->ODR = data;
	
	Sys_DelayUs(1);
	HAL_GPIO_WritePin(GLCD_E_Port, GLCD_E_Pin, GPIO_PIN_SET);
	Sys_DelayUs(2);
	HAL_GPIO_WritePin(GLCD_E_Port, GLCD_E_Pin, GPIO_PIN_RESET);
	Sys_DelayUs(4);
	HAL_GPIO_WritePin(GLCD_CS1_Port, GLCD_CS1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GLCD_CS2_Port, GLCD_CS2_Pin, GPIO_PIN_RESET);
}
/*
void GLCD_Clear(void) {
	int8_t x, y;
	for (x = 0; x < 8; x++) {
		GLCD_Write(GLCD_CS_12, GLCD_DI_COMMAND, GLCD_REG_COLUMN_ADDR);
		GLCD_Write(GLCD_CS_12, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | x));
		for (y = 0; y < 64; y ++) {
			GLCD_Write(GLCD_CS_12, GLCD_DI_DATA, 0x00);
		}
	}
}
*/

/**
 * 		m_data: adott metszet rajzolata in hex format
 * 		cX: sor (0-7)
 * 		cY: oszlop (0-127) */
void GLCD_Write_Block(int8_t m_data, int8_t cY, int8_t cX) {
	int8_t chip = GLCD_CS_1;
	if (cX >= 64) {
		chip = GLCD_CS_2;
		cX -= 64;
	}
	GLCD_Write(chip, GLCD_DI_COMMAND, (GLCD_REG_COLUMN_ADDR | cX));
	GLCD_Write(chip, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | cY));
	GLCD_Write(chip, GLCD_DI_DATA, m_data);
}

/*
void GLCD_WriteString(const int8_t* string, int8_t Y, int8_t X) {
	int8_t temp = 0;
	int i = 0;
	while (string[i] != '\0') {
		temp = string[i];
		GLCD_Write_Char(temp - 32, X, Y + 6 * i);
		i++;
	}
}

void GLCD_Write_Char(int8_t cPlace, int8_t cX, int8_t cY) {
	int8_t i = 0;
	int8_t chip = GLCD_CS_1;
	if(cY >= 64) {
		chip = GLCD_CS_2;
		cY -= 64;
	}
	GLCD_Write(chip, GLCD_DI_COMMAND, (GLCD_REG_COLUMN_ADDR | cY));
	GLCD_Write(chip, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | cX));
	for (i = 0; i < 5; i++) {
		if (cY + i >= 64) {
			chip = 2;
			GLCD_Write(chip, GLCD_DI_COMMAND, (GLCD_REG_COLUMN_ADDR | (cY + i - 64)));
			GLCD_Write(chip, GLCD_DI_COMMAND, (GLCD_REG_PAGE_ADDR | cX));
		}
		GLCD_Write(chip, GLCD_DI_DATA, fontdata[cPlace * 5 + i]);
	}
}
*/


/**
* @brief TIM_PWM MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_pwm: TIM_PWM handle pointer
* @retval None
*/
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	if(htim_pwm->Instance==TIM8) {
		/* Peripheral clock enable */
		__HAL_RCC_TIM8_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**TIM8 GPIO Configuration
		PC8		 ------> TIM8_CH3 
		*/
		GPIO_InitStruct.Pin = GLCD_BACKLIGHT_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
		HAL_GPIO_Init(GLCD_BACKLIGHT_Port, &GPIO_InitStruct);
	}

}

/**
* @brief TIM_PWM MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_pwm: TIM_PWM handle pointer
* @retval None
*/

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm) {

	if(htim_pwm->Instance==TIM8) {
	/* USER CODE BEGIN TIM8_MspDeInit 0 */

	/* USER CODE END TIM8_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM8_CLK_DISABLE();
	/* USER CODE BEGIN TIM8_MspDeInit 1 */

	/* USER CODE END TIM8_MspDeInit 1 */
	}

}




/**
	* @brief TIM8 Initialization Function
	* @param None
	* @retval None
	*/
void MX_TIM8_Init(void) {

	/* USER CODE BEGIN TIM8_Init 0 */

	/* USER CODE END TIM8_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

	/* USER CODE BEGIN TIM8_Init 1 */

	/* USER CODE END TIM8_Init 1 */
	htim8.Instance = TIM8;
	htim8.Init.Prescaler = 0;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim8.Init.Period = 0xFFFF;
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim8.Init.RepetitionCounter = 0;
	if (HAL_TIM_PWM_Init(&htim8) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0x7FFFU;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM8_Init 2 */

	/* USER CODE END TIM8_Init 2 */
}






/**
	* @brief GPIO Initialization Function
	* @param None
	* @retval None
	*/
void MX_GLCD_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GLCD_DB0_Pin|GLCD_DB1_Pin|GLCD_DB2_Pin|GLCD_DB3_Pin 
							|GLCD_DB4_Pin|GLCD_DB5_Pin|GLCD_DB6_Pin|GLCD_DB7_Pin 
							|GLCD_E_Pin|GLCD_EN_Pin|GLCD_RST_Pin|GLCD_CS1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GLCD_DI_Pin|GLCD_RW_Pin|GLCD_CS2_Pin, GPIO_PIN_RESET);
	
	/*Configure GPIO pins : GLCD_DB0_Pin GLCD_DB1_Pin GLCD_DB2_Pin GLCD_DB3_Pin 
													 GLCD_DB4_Pin GLCD_DB5_Pin GLCD_DB6_Pin GLCD_DB7_Pin 
													 GLCD_E_Pin GLCD_EN_Pin GLCD_RST_Pin GLCD_CS1_Pin */
	GPIO_InitStruct.Pin = GLCD_DB0_Pin|GLCD_DB1_Pin|GLCD_DB2_Pin|GLCD_DB3_Pin 
						 |GLCD_DB4_Pin|GLCD_DB5_Pin|GLCD_DB6_Pin|GLCD_DB7_Pin 
						 |GLCD_E_Pin|GLCD_EN_Pin|GLCD_RST_Pin|GLCD_CS1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : GLCD_DI_Pin GLCD_RW_Pin GLCD_CS2_Pin */
	GPIO_InitStruct.Pin = GLCD_DI_Pin|GLCD_RW_Pin|GLCD_CS2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}









