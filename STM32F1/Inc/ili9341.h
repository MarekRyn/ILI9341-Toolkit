/*
 * ili9341.c
 *
 *  Created on: Jun 01, 2020
 *      Author: Marek Ryn
 */

#ifndef SRC_ILI9341_H_
#define SRC_ILI9341_H_

#include "stm32f1xx_hal.h"

#define ILI9341_TFTWIDTH  	240
#define ILI9341_TFTHEIGHT 	320

#define ILI9341_NOP     	0x00
#define ILI9341_SWRESET 	0x01
#define ILI9341_RDDID   	0xD3
#define ILI9341_RDDST   	0x09

#define ILI9341_SLPIN   	0x10
#define ILI9341_SLPOUT  	0x11
#define ILI9341_PTLON   	0x12
#define ILI9341_NORON   	0x13

#define ILI9341_RDMODE  	0x0A
#define ILI9341_RDMADCTL  	0x0B
#define ILI9341_RDPIXFMT  	0x0C
#define ILI9341_RDIMGFMT  	0x0D
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF  	0x20
#define ILI9341_INVON   	0x21
#define ILI9341_GAMMASET 	0x26
#define ILI9341_DISPOFF 	0x28
#define ILI9341_DISPON  	0x29

#define ILI9341_CASET   	0x2A
#define ILI9341_PASET   	0x2B
#define ILI9341_RAMWR   	0x2C
#define ILI9341_RAMRD   	0x2E

#define ILI9341_PTLAR   	0x30
#define ILI9341_MADCTL  	0x36
#define ILI9341_PIXFMT  	0x3A

#define ILI9341_FRMCTR1 	0xB1
#define ILI9341_FRMCTR2 	0xB2
#define ILI9341_FRMCTR3 	0xB3
#define ILI9341_INVCTR  	0xB4
#define ILI9341_DFUNCTR 	0xB6

#define ILI9341_PWCTR1  	0xC0
#define ILI9341_PWCTR2  	0xC1
#define ILI9341_PWCTR3  	0xC2
#define ILI9341_PWCTR4  	0xC3
#define ILI9341_PWCTR5  	0xC4
#define ILI9341_VMCTR1  	0xC5
#define ILI9341_VMCTR2  	0xC7

#define ILI9341_RDID1   	0xDA
#define ILI9341_RDID2   	0xDB
#define ILI9341_RDID3   	0xDC
#define ILI9341_RDID4   	0xDD

#define ILI9341_GMCTRP1 	0xE0
#define ILI9341_GMCTRN1 	0xE1

// Color definitions
#define C_BLACK       	0x0000      /*   0,   0,   0 */
#define C_NAVY        	0x000F      /*   0,   0, 128 */
#define C_DARKGREEN  	0x03E0      /*   0, 128,   0 */
#define C_DARKCYAN    	0x03EF      /*   0, 128, 128 */
#define C_MAROON     	0x7800      /* 128,   0,   0 */
#define C_PURPLE     	0x780F      /* 128,   0, 128 */
#define C_OLIVE       	0x7BE0      /* 128, 128,   0 */
#define C_LIGHTGREY   	0xC618      /* 192, 192, 192 */
#define C_DARKGREY    	0x7BEF      /* 128, 128, 128 */
#define C_BLUE        	0x001F      /*   0,   0, 255 */
#define C_GREEN      	0x07E0      /*   0, 255,   0 */
#define C_CYAN        	0x07FF      /*   0, 255, 255 */
#define C_RED         	0xF800      /* 255,   0,   0 */
#define C_MAGENTA     	0xF81F      /* 255,   0, 255 */
#define C_YELLOW      	0xFFE0      /* 255, 255,   0 */
#define C_WHITE       	0xFFFF      /* 255, 255, 255 */
#define C_ORANGE      	0xFD20      /* 255, 165,   0 */
#define C_GREENYELLOW 	0xAFE5      /* 173, 255,  47 */
#define C_PINK        	0xF81F



//Data Pins (16bit)
#define TFT_DATA       	GPIOB

//Control pins |RD |WR |DCX|CS |RST|
#define TFT_CNTRL      	GPIOC
#define TFT_RD         	GPIO_PIN_0
#define TFT_WR         	GPIO_PIN_1
#define TFT_RS         	GPIO_PIN_2 //DCX
#define TFT_CS         	GPIO_PIN_3
#define TFT_RST        	GPIO_PIN_4
#define TFT_BKLT		GPIO_PIN_5 //Driving transistor for TFT Backlight


void TFT_BkLt_On(void);
void TFT_BkLt_Off(void);
void TFT_Init(void);
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void TFT_HLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void TFT_VLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFT_Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFT_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void TFT_Circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void TFT_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void TFT_ShowImage(uint16_t x, uint16_t y, uint8_t data[], uint16_t size);
uint16_t TFT_Text(uint16_t x, uint16_t y, uint8_t font_data[], uint16_t font_color, uint16_t bg_color, char str[]);

#endif /* SRC_ILI9341_H_ */
