/*
 * ili9341.c
 *
 *  Created on: Jun 01, 2020
 *      Author: Marek Ryn
 */

#include "ili9341.h"

//Sinus
uint16_t sinus[] = {0, 4, 8, 13, 17, 22, 26, 31, 35, 40, 44, 49, 53, 57, 62, 66,
		71, 75, 80, 84, 88, 93, 97, 102, 106, 110, 115, 119, 123, 128, 132, 136,
		141, 145, 149, 153, 158, 162, 166, 170, 175, 179, 183, 187, 191, 195, 200, 204,
		208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 259, 263, 267,
		271, 275, 278, 282, 286, 289, 293, 297, 300, 304, 308, 311, 315, 318, 322, 325,
		329, 332, 335, 339, 342, 345, 349, 352, 355, 358, 362, 365, 368, 371, 374, 377,
		380, 383, 386, 389, 392, 395, 397, 400, 403, 406, 408, 411, 414, 416, 419, 421,
		424, 426, 429, 431, 434, 436, 438, 441, 443, 445, 447, 449, 452, 454, 456, 458,
		460, 462, 464, 465, 467, 469, 471, 473, 474, 476, 477, 479, 481, 482, 484, 485,
		486, 488, 489, 490, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503,
		504, 504, 505, 506, 507, 507, 508, 508, 509, 509, 510, 510, 510, 511, 511, 511,
		511, 511, 511, 511, 512};

//Macros
#define RD_ACTIVE	TFT_CNTRL->BRR = TFT_RD
#define RD_IDLE     TFT_CNTRL->BSRR = TFT_RD
#define WR_ACTIVE   TFT_CNTRL->BRR = TFT_WR
#define WR_IDLE     TFT_CNTRL->BSRR = TFT_WR
#define CD_COMMAND  TFT_CNTRL->BRR = TFT_RS
#define CD_DATA     TFT_CNTRL->BSRR = TFT_RS
#define CS_ACTIVE   TFT_CNTRL->BRR = TFT_CS
#define CS_IDLE     TFT_CNTRL->BSRR = TFT_CS
#define RST_LO		TFT_CNTRL->BRR = TFT_RST
#define RST_HI		TFT_CNTRL->BSRR = TFT_RST
#define BKLT_OFF	TFT_CNTRL->BRR = TFT_BKLT
#define BKLT_ON		TFT_CNTRL->BSRR = TFT_BKLT


#define RD_STROBE	{TFT_CNTRL->BRR = TFT_RD; TFT_CNTRL->BSRR = TFT_RD;}
#define WR_STROBE	{TFT_CNTRL->BRR = TFT_WR; TFT_CNTRL->BSRR = TFT_WR;}


// *********************************************************************
// Private functions
// *********************************************************************

void write16(uint16_t c) {
	CS_ACTIVE;
	TFT_DATA->ODR = c;
	WR_STROBE;
	CS_IDLE;
}


void write16special(uint16_t c) {
	TFT_DATA->ODR = c;
	WR_STROBE;
}


void writecommand(uint8_t c) {
	CD_COMMAND;
	write16(c);
}


void writedata(uint8_t c) {
	CD_DATA;
	write16(c);
}


void setadrwindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	CD_COMMAND;
	write16special(ILI9341_CASET); // Column addr set
	CD_DATA;
	write16special(x0 >> 8);
	write16special(x0);     // XSTART
	write16special(x1 >> 8);
	write16special(x1);     // XEND

	CD_COMMAND;

	write16special(ILI9341_PASET); // Row addr set
	CD_DATA;
	write16special(y0 >> 8);
	write16special(y0);     // YSTART
	write16special(y1 >> 8);
	write16special(y1);     // YEND
	CD_COMMAND;
	write16special(ILI9341_RAMWR); // write to RAM
}


uint16_t mixcolors(uint16_t color0, uint16_t color1, uint8_t mix) {

	uint32_t r0 = (color0 >> 8) & 0x00F8;
	uint32_t g0 = (color0 >> 3) & 0x00FC;
	uint32_t b0 = (color0 << 3) & 0x00F8;

	uint32_t r1 = (color1 >> 8) & 0x00F8;
	uint32_t g1 = (color1 >> 3) & 0x00FC;
	uint32_t b1 = (color1 << 3) & 0x00F8;

	uint32_t r = (((16 - mix) * r0) + (mix * r1)) >> 4;
	uint32_t g = (((16 - mix) * g0) + (mix * g1)) >> 4;
	uint32_t b = (((16 - mix) * b0) + (mix * b1)) >> 4;

	uint16_t rr = (r & 0xF8) << 8;
	uint16_t gg = (g & 0xFC) << 3;
	uint16_t bb = (b >> 3);

	return (rr | gg | bb);
}


uint16_t character(uint16_t x, uint16_t y, uint8_t font_data[], uint16_t font_color, uint16_t bg_color, char ch) {

	uint16_t colors[4] = {mixcolors(bg_color, font_color, 0),
						  mixcolors(bg_color, font_color, 5),
						  mixcolors(bg_color, font_color, 10),
						  mixcolors(bg_color, font_color, 16)};
	uint16_t *adr0;
	uint16_t *adr1;
	uint8_t m;
	uint8_t i;
	uint8_t height = font_data[0];
	uint8_t swidth = font_data[1];

	// Character outside allowable range
	if ((ch < 33) || (ch > 126)) {
		TFT_FillRect(x, y, swidth, height, colors[0]);
		return (x + swidth);
	}

	uint16_t a = (ch - 33) * 2;
	adr0 = (uint16_t*)&font_data[a+2];
	adr1 = (uint16_t*)&font_data[a+4];

	// Character outside defined subset
	if (*adr0 == *adr1) {
		TFT_FillRect(x, y, swidth, height, colors[0]);
		return (x + swidth);
	}

	uint8_t width = font_data[*adr0];

	CS_ACTIVE;
	setadrwindow(x, y, x + width - 1, y + height - 1);
	CD_DATA;

	for (uint16_t j = *adr0+1; j<*adr1; j++) {
		m = font_data[j] >> 6;
		switch(m) {

			case 0:
			case 3:
				TFT_DATA->ODR = colors[m];
				for (i=0; i<(font_data[j] & 0x3F); i++) WR_STROBE;
				break;

			case 1:
			case 2:
				TFT_DATA->ODR = colors[font_data[j] >> 6];
				WR_STROBE;
				TFT_DATA->ODR = colors[(font_data[j] >> 4) & 0x03];
				WR_STROBE;
				TFT_DATA->ODR = colors[(font_data[j] >> 2) & 0x03];
				WR_STROBE;
				TFT_DATA->ODR = colors[font_data[j] & 0x03];
				WR_STROBE;
				break;
		}
	}
	CS_IDLE;
	TFT_FillRect((x + width), y, (swidth / 8), height, colors[0]);
	return (x + width + (swidth / 8));
}


// *********************************************************************
// Public functions
// *********************************************************************

// TFT Backlight ON
void TFT_BkLt_On(void) {
	BKLT_ON;
}


// TFT Backlight OFF
void TFT_BkLt_Off(void) {
	BKLT_OFF;
}


// TFT Initialization
void TFT_Init(void) {

	// Initial Control Lines Setup
	CS_IDLE;
	CD_DATA;
	WR_IDLE;
	RD_IDLE;

	// Reset sequence
	RST_HI;
	HAL_Delay(5);
	RST_LO;
	HAL_Delay(20);
	RST_HI;
	HAL_Delay(150);

	// ??
	writecommand(0xEF);
	writedata(0x03);
	writedata(0x80);
	writedata(0x02);

	// Power Control A
	writecommand(0xCB);
	writedata(0x39);
	writedata(0x2C);
	writedata(0x00);
	writedata(0x34);
	writedata(0x02);

	// Power Control B
	writecommand(0xCF);
	writedata(0x00);
	writedata(0XC1);
	writedata(0X30);

	// Driver Timing Control A
	writecommand(0xE8);
	writedata(0x85);
	writedata(0x00);
	writedata(0x78);

	// Driver Timing Control B
	writecommand(0xEA);
	writedata(0x00);
	writedata(0x00);

	// Power On Sequence Control
	writecommand(0xED);
	writedata(0x64);
	writedata(0x03);
	writedata(0X12);
	writedata(0X81);

	// Pump Ratio Control
	writecommand(0xF7);
	writedata(0x20);

	// Pump Control, VRH[5:0]
	writecommand(ILI9341_PWCTR1);
	writedata(0x23);

	// Pump Control, SAP[2:0];BT[3:0]
	writecommand(ILI9341_PWCTR2);
	writedata(0x10);

	// VCM Control 1
	writecommand(ILI9341_VMCTR1);
	writedata(0x3e);
	writedata(0x28);

	// VCM Control 2
	writecommand(ILI9341_VMCTR2);
	writedata(0x86);

	// Memory Access Control
	writecommand(ILI9341_MADCTL);
	writedata(0x48);

	// Pixel Format
	writecommand(ILI9341_PIXFMT);
	writedata(0x55);

	// Frame Ratio Control, Standard RGB Color
	writecommand(ILI9341_FRMCTR1);
	writedata(0x00);
	writedata(0x18);

	// Display Function Control
	writecommand(ILI9341_DFUNCTR);
	writedata(0x08);
	writedata(0x82);
	writedata(0x27);

	//3Gamma Function Disable
	writecommand(0xF2);
	writedata(0x00);

	//Gamma curve selected
	writecommand(ILI9341_GAMMASET);
	writedata(0x01);

	//Positive Gamma Correction
	writecommand(ILI9341_GMCTRP1);
	writedata(0x0F);
	writedata(0x31);
	writedata(0x2B);
	writedata(0x0C);
	writedata(0x0E);
	writedata(0x08);
	writedata(0x4E);
	writedata(0xF1);
	writedata(0x37);
	writedata(0x07);
	writedata(0x10);
	writedata(0x03);
	writedata(0x0E);
	writedata(0x09);
	writedata(0x00);

	//Negative Gamma Correction
	writecommand(ILI9341_GMCTRN1);
	writedata(0x00);
	writedata(0x0E);
	writedata(0x14);
	writedata(0x03);
	writedata(0x11);
	writedata(0x07);
	writedata(0x31);
	writedata(0xC1);
	writedata(0x48);
	writedata(0x08);
	writedata(0x0F);
	writedata(0x0C);
	writedata(0x31);
	writedata(0x36);
	writedata(0x0F);

	//Invert Off
	writecommand(ILI9341_INVOFF);
	HAL_Delay(120);

	//Exit Sleep
	writecommand(ILI9341_SLPOUT);
	HAL_Delay(120);

	//Display On
	writecommand(ILI9341_DISPON);
}


// Draws single pixel
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
	//TODO: Input Check
	CS_ACTIVE;
	setadrwindow(x, y, x, y);
	CD_DATA;
	write16special(color);
	CS_IDLE;
}


// Draws horizontal line
void TFT_HLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color) {
	//TODO: Input Check
	CS_ACTIVE;
	setadrwindow(x, y, x + w - 1, y);
	CD_DATA;
	TFT_DATA->ODR = color;
	for(uint32_t i=0; i<w; i++) WR_STROBE;
	CS_IDLE;
}


// Draws vertical line
void TFT_VLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color) {
	//TODO: Input Check
	CS_ACTIVE;
	setadrwindow(x, y, x, y + h - 1);
	CD_DATA;
	TFT_DATA->ODR = color;
	for(uint32_t i=0; i<h; i++) WR_STROBE;
	CS_IDLE;
}


// Draws filled rectangle
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	//TODO: Input Check
	CS_ACTIVE;
	setadrwindow(x, y, x + w - 1, y + h - 1);
	CD_DATA;
 	TFT_DATA->ODR = color;
 	for(uint32_t i=0; i<w*h; i++) WR_STROBE;
 	CS_IDLE;
}


// Draws rectangle
void TFT_Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	//TODO: Input Check
	TFT_HLine(x, y, w, color);
	TFT_HLine(x, y + h - 1, w, color);
	TFT_VLine(x, y, h, color);
	TFT_VLine(x + w - 1, y, h, color);
}


// Draws line between two points
void TFT_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
	//TODO: Input Check

	int32_t dx, dy, sx, sy;

	if (x2 >= x1) { dx = x2 - x1; sx = 1; } else { dx = x1 - x2; sx = -1; }
	if (y2 >= y1) {	dy = y1 - y2; sy = 1; } else { dy = y2 - y1; sy = -1; }

	int32_t dx2 = dx << 1;
	int32_t dy2 = dy << 1;
	int32_t err = dx2 + dy2;
	CS_ACTIVE;
	while (1) {
		setadrwindow(x1, y1, x1, y1);
		CD_DATA;
		write16special(color);
	    if (err >= dy) {
	    	if (x1 == x2) break;
	        err += dy2;
	        x1 += sx;
	    }
	    if (err <= dx) {
	    	if (y1 == y2) break;
	        err += dx2;
	        y1 += sy;
	    }
	}
	CS_IDLE;
}


// Draws circle
void TFT_Circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
	//TODO: Input Check

	uint16_t x, y, a;

	for (a=0; a<180; a++) {
		x = (sinus[a] * r) >> 9;
		y = (sinus[180-a] * r) >> 9;
		TFT_DrawPixel(x0 + x, y0 + y, color);
		TFT_DrawPixel(x0 + x, y0 - y, color);
		TFT_DrawPixel(x0 - x, y0 + y, color);
		TFT_DrawPixel(x0 - x, y0 - y, color);
	}
}


// Draws filled circle
void TFT_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
	//TODO: Input Check

	uint16_t x, y, a;

	for (a=0; a<180; a++) {
		x = (sinus[a] * r) >> 9;
		y = (sinus[180-a] * r) >> 9;
		TFT_HLine(x0 - x, y0 + y, 2 * x, color);
		TFT_HLine(x0 - x, y0 - y, 2 * x, color);
	}
}


// Decompress and display image encoded in array
//(for encoding image use python script attached to this repository)
void TFT_ShowImage(uint16_t x, uint16_t y, uint8_t data[], uint16_t size) {
	uint16_t xx = 0;
	uint16_t yy = 0;
	uint16_t bx = 0;
	uint16_t by = 0;
	uint16_t w = 0;
	uint16_t h = 0;
	uint8_t flag = 0;
	uint8_t b = 0;
	uint8_t c = 0;
	uint16_t *color;
	uint16_t *t;

	// Reading color palette and setting data index
	uint8_t cnum = data[0];
	color = (uint16_t*)&data[c*2+1];
	uint16_t i = 1 + 2 * cnum;

	while (i<size) {
		b = data[i]; i++;

		// Special Function
		if ((b & 0xC0) == 0x00) {
			if ((b & 0x3F) == 0x00) {
				// Color change, image sector to (0,0)
				c++;
				color = (uint16_t*)&data[c*2+1];
				bx = 0;
				by = 0;
			} else {
				// Image sector change
				bx = (b & 0x38) >> 3;
				by =  b & 0x07;
			}
		}

		// H-Line
		if ((b & 0xC0) == 0x40) {
			w = b & 0x3F;
			if (w == 0x3F) {
				flag = 1;
				while (flag) {
					w += data[i];
					if (data[i] < 0xFF) flag = 0;
					i++;
				}
			}
			xx = x + bx * 256 + data[i]; i++;
			yy = y + by * 256 + data[i]; i++;
			TFT_HLine(xx, yy, w, *color);
		}

		// V-Line
		if ((b & 0xC0) == 0x80) {
			h = b & 0x3F;
			if (h == 0x3F) {
				flag = 1;
				while (flag) {
					h += data[i];
					if (data[i] < 0xFF) flag = 0;
					i++;
				}
			}
			xx = x + bx * 256 + data[i]; i++;
			yy = y + by * 256 + data[i]; i++;
			TFT_VLine(xx, yy, h, *color);
		}

		// Block
		if ((b & 0xC0) == 0xC0) {
			if ((b & 0x02) == 0x10) {
				t = (uint16_t*)&data[i];
				i+=2;
				w = *t;
			} else {
				w = data[i]; i++;
			}
			if ((b & 0x01) == 0x01) {
				t = (uint16_t*)&data[i];
				i+=2;
				h = *t;
			} else {
				h = data[i]; i++;
			}
			xx = x + bx * 256 + data[i]; i++;
			yy = y + by * 256 + data[i]; i++;
			TFT_FillRect(xx, yy, w, h, *color);
		}
	}

}


// Displays text with encoded font
//(for encoding image use python script attached to this repository)
uint16_t TFT_Text(uint16_t x, uint16_t y, uint8_t font_data[], uint16_t font_color, uint16_t bg_color, char *str) {
	while (*str > 0) {
		x = character(x, y, font_data, font_color, bg_color, *str);
		str += 1;
	}
	return x;
}
