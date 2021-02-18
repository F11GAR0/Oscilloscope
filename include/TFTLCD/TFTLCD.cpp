#include "TFTLCD.h"

// register names from Peter Barrett's Microtouch code

#define TFTLCD_START_OSC			0x00
#define TFTLCD_DRIV_OUT_CTRL		0x01
#define TFTLCD_DRIV_WAV_CTRL		0x02
#define TFTLCD_ENTRY_MOD			0x03
#define TFTLCD_RESIZE_CTRL			0x04
#define TFTLCD_DISP_CTRL1			0x07
#define TFTLCD_DISP_CTRL2			0x08
#define TFTLCD_DISP_CTRL3			0x09
#define TFTLCD_DISP_CTRL4			0x0A
#define TFTLCD_RGB_DISP_IF_CTRL1	0x0C
#define TFTLCD_FRM_MARKER_POS		0x0D
#define TFTLCD_RGB_DISP_IF_CTRL2	0x0F
#define TFTLCD_POW_CTRL1			0x10
#define TFTLCD_POW_CTRL2			0x11
#define TFTLCD_POW_CTRL3			0x12
#define TFTLCD_POW_CTRL4			0x13
#define TFTLCD_GRAM_HOR_AD			0x2a
#define TFTLCD_GRAM_VER_AD			0x2b
#define TFTLCD_RW_GRAM				0x22
#define TFTLCD_POW_CTRL7			0x29
#define TFTLCD_FRM_RATE_COL_CTRL	0x2B
#define TFTLCD_GAMMA_CTRL1			0x30
#define TFTLCD_GAMMA_CTRL2			0x31
#define TFTLCD_GAMMA_CTRL3			0x32
#define TFTLCD_GAMMA_CTRL4			0x35 
#define TFTLCD_GAMMA_CTRL5			0x36
#define TFTLCD_GAMMA_CTRL6			0x37
#define TFTLCD_GAMMA_CTRL7			0x38
#define TFTLCD_GAMMA_CTRL8			0x39
#define TFTLCD_GAMMA_CTRL9			0x3C
#define TFTLCD_GAMMA_CTRL10			0x3D
#define TFTLCD_HOR_START_AD			0x50
#define TFTLCD_HOR_END_AD			0x51
#define TFTLCD_VER_START_AD			0x52
#define TFTLCD_VER_END_AD			0x53
#define TFTLCD_GATE_SCAN_CTRL1		0x60
#define TFTLCD_GATE_SCAN_CTRL2		0x61
#define TFTLCD_GATE_SCAN_CTRL3		0x6A
#define TFTLCD_PART_IMG1_DISP_POS	0x80
#define TFTLCD_PART_IMG1_START_AD	0x81
#define TFTLCD_PART_IMG1_END_AD		0x82
#define TFTLCD_PART_IMG2_DISP_POS	0x83
#define TFTLCD_PART_IMG2_START_AD	0x84
#define TFTLCD_PART_IMG2_END_AD		0x85
#define TFTLCD_PANEL_IF_CTRL1		0x90
#define TFTLCD_PANEL_IF_CTRL2		0x92
#define TFTLCD_PANEL_IF_CTRL3		0x93
#define TFTLCD_PANEL_IF_CTRL4		0x95
#define TFTLCD_PANEL_IF_CTRL5		0x97
#define TFTLCD_PANEL_IF_CTRL6		0x98

#define TFTLCD_DELAYCMD                 0xFF

#define swap(a, b) { int16_t t = a; a = b; b = t; }

#ifdef USE_ADAFRUIT_SHIELD_PINOUT
// special defines for the dataport
 #define DATAPORT1 PORTD
 #define DATAPIN1 PIND
 #define DATADDR1 DDRD

 #define DATAPORT2 PORTB
 #define DATAPIN2 PINB
 #define DATADDR2 DDRB

 #define DATA1_MASK 0xD0
 #define DATA2_MASK 0x2F

// for mega & shield usage, we just hardcoded it (its messy)

#else
 // for the breakout board tutorial, two ports are used :/
 #define DATAPORT1 PORTD
 #define DATAPIN1  PIND
 #define DATADDR1  DDRD

 #define DATAPORT2 PORTB
 #define DATAPIN2  PINB
 #define DATADDR2  DDRB

 #define DATA1_MASK 0xFC  // top 6 bits
 #define DATA2_MASK 0x03  // bottom 2 bits


 #define MEGA_DATAPORT PORTA
 #define MEGA_DATAPIN  PINA
 #define MEGA_DATADDR  DDRA
#endif


#include "glcdfont.c"
#include <avr/pgmspace.h>
#include "pins_arduino.h"
#include "wiring_private.h"


void TFTLCD::goHome(void) {
  goTo(0,0);
}

uint16_t TFTLCD::width(void) {
  return _width;
}
uint16_t TFTLCD::height(void) {
  return _height;
}

void TFTLCD::goTo(int x, int y) {
  writeRegister(0x0020, x);     // GRAM Address Set (Horizontal Address) (R20h)
  writeRegister(0x0021, y);     // GRAM Address Set (Vertical Address) (R21h)
  writeCommand(0x0022);            // Write Data to GRAM (R22h)
}

void TFTLCD::setCursor(uint16_t x, uint16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void TFTLCD::setTextSize(uint8_t s) {
  textsize = s;
}

void TFTLCD::setTextColor(uint16_t c) {
  textcolor = c;
}

size_t TFTLCD::write(uint8_t c) {
  if (c == '\n') {
    cursor_y += textsize*8;
    cursor_x = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    drawChar(cursor_x, cursor_y, c, textcolor, textsize);
    cursor_x += textsize*6;
  }

  return 1;
}

#define Y1 A3  
#define X1 A2  
#define Y2 9   
#define X2 8 
uint16_t ROW_F=110; // TS first row
uint16_t ROW_L=920; // TS last row
uint16_t COL_F=110; // TS first column
uint16_t COL_L=930; // TS last column
byte ReadTouch(int *rrow, int *rcol) {
  //Y1 A3  
  //X1 A2   
  //Y2 9   
  //X2 8   
  int16_t row, col;
  int8_t touch, wait_touch, valid;
  wait_touch=1;
  valid=0;
  while (wait_touch) {
    pinMode(Y1, INPUT); 
    pinMode(Y2, INPUT_PULLUP); 
    
    pinMode(X1, OUTPUT);
    pinMode(X2, OUTPUT);
    digitalWrite(X1, LOW);
    digitalWrite(X2, LOW);
    
    touch = !digitalRead(Y1); // 0 - touched
    if (touch) {
      //delay(5);
      digitalWrite(X1, HIGH);   // X variant A
      //digitalWrite(X2, HIGH);   // X variant B
      delay(1);
      row = analogRead(Y1);
      delay(4); 
      if (abs(analogRead(Y1)-row)>3) { return 0;}
      delay(3);
      if (abs(analogRead(Y1)-row)>3) { return 0;}
      //if (analogRead(Y1)!=row) { return 0;}
      
      pinMode(X1, INPUT); 
      pinMode(X2, INPUT_PULLUP); 
      
      pinMode(Y1, OUTPUT);
      pinMode(Y2, OUTPUT);
      //digitalWrite(Y1, HIGH);  // Y variant A
      //digitalWrite(Y2, LOW);  // Y variant A
      digitalWrite(Y1, LOW);  // Y variant B
      digitalWrite(Y2, HIGH);  // Y variant B
      delay(1);
      col = analogRead(X1);
      delay(4);  
      if (abs(analogRead(X1)-col)>3) { return 0;}
      delay(3);
      if (abs(analogRead(X1)-col)>3) { return 0;}
      //if (analogRead(X1)!=col) { return 0;}
      
      //digitalWrite(Y1, LOW);  // Y variant A
      digitalWrite(Y2, LOW);  // Y variant B
      //delay(5);
      touch = !digitalRead(X1); // 0 - dotyk
      if (touch) {
        int16_t rows=ROW_L-ROW_F;
        int16_t cols=COL_L-COL_F;
        float row1=float(row-ROW_F)/rows*240;
        float col1=float(col-COL_F)/cols*320;
        *rrow=int(row1);
        *rcol=int(col1);
        valid=1;
      }
      wait_touch=0;
    }
  }
  return valid;
}


void TFTLCD::drawString(uint16_t x, uint16_t y, char *c, 
			uint16_t color, uint8_t size) {
  while (c[0] != 0) {
    drawChar(x, y, c[0], color, size);
    x += size*6;
    c++;
  }
}
// draw a character
void TFTLCD::drawChar(uint16_t x, uint16_t y, char c, 
		      uint16_t color, uint8_t size) {
  for (uint8_t i =0; i<5; i++ ) {
    uint8_t line = pgm_read_byte(font+(c*5)+i);
    for (uint8_t j = 0; j<8; j++) {
      if (line & 0x1) {
	if (size == 1) // default size
	  drawPixel(x+i, y+j, color);
	else {  // big size
	  fillRect(x+i*size, y+j*size, size, size, color);
	} 
      }
      line >>= 1;
    }
  }
}

// draw a triangle!
void TFTLCD::drawTriangle(uint16_t x0, uint16_t y0,
			  uint16_t x1, uint16_t y1,
			  uint16_t x2, uint16_t y2, uint16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color); 
}

void TFTLCD::fillTriangle ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color)
{
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  int32_t dx1, dx2, dx3; // Interpolation deltas
  int32_t sx1, sx2, sy; // Scanline co-ordinates

  sx2=(int32_t)x0 * (int32_t)1000; // Use fixed point math for x axis values
  sx1 = sx2;
  sy=y0;

  // Calculate interpolation deltas
  if (y1-y0 > 0) dx1=((x1-x0)*1000)/(y1-y0);
    else dx1=0;
  if (y2-y0 > 0) dx2=((x2-x0)*1000)/(y2-y0);
    else dx2=0;
  if (y2-y1 > 0) dx3=((x2-x1)*1000)/(y2-y1);
    else dx3=0;

  // Render scanlines (horizontal lines are the fastest rendering method)
  if (dx1 > dx2)
  {
    for(; sy<=y1; sy++, sx1+=dx2, sx2+=dx1)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
    sx2 = x1*1000;
    sy = y1;
    for(; sy<=y2; sy++, sx1+=dx2, sx2+=dx3)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
  }
  else
  {
    for(; sy<=y1; sy++, sx1+=dx1, sx2+=dx2)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
    sx1 = x1*1000;
    sy = y1;
    for(; sy<=y2; sy++, sx1+=dx3, sx2+=dx2)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
  }
}

uint16_t TFTLCD::Color565(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t c;
  c = r >> 3;
  c <<= 6;
  c |= g >> 2;
  c <<= 5;
  c |= b >> 3;

  return c;
}

// draw a rectangle
void TFTLCD::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
		      uint16_t color) {
  // smarter version
  drawHorizontalLine(x, y, w, color);
  drawHorizontalLine(x, y + h - 1, w, color);
  drawVerticalLine(x, y, h, color);
  drawVerticalLine(x + w - 1, y, h, color);
}

// draw a rounded rectangle
void TFTLCD::drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r,
			   uint16_t color) {
  // smarter version
  drawHorizontalLine(x+r, y, w-2*r, color);
  drawHorizontalLine(x+r, y+h-1, w-2*r, color);
  drawVerticalLine(x, y+r, h-2*r, color);
  drawVerticalLine(x+w-1, y+r, h-2*r, color);
  // draw four corners
  drawCircleHelper(x+r, y+r, r, 1, color);
  drawCircleHelper(x+w-r-1, y+r, r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r, y+h-r-1, r, 8, color);
}


// fill a rounded rectangle
void TFTLCD::fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r,
			   uint16_t color) {
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r, y+r, r, 2, h-2*r-1, color);
}

// fill a circle
void TFTLCD::fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
  writeRegister(TFTLCD_ENTRY_MOD, 0x1030);
  drawVerticalLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}


// used to do circles and roundrects!
void TFTLCD::fillCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername, uint16_t delta,
			uint16_t color) {

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    if (cornername & 0x1) {
      drawVerticalLine(x0+x, y0-y, 2*y+1+delta, color);
      drawVerticalLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawVerticalLine(x0-x, y0-y, 2*y+1+delta, color);
      drawVerticalLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}


// draw a circle outline

void TFTLCD::drawCircle(uint16_t x0, uint16_t y0, uint16_t r, 
			uint16_t color) {
  drawPixel(x0, y0+r, color);
  drawPixel(x0, y0-r, color);
  drawPixel(x0+r, y0, color);
  drawPixel(x0-r, y0, color);

  drawCircleHelper(x0, y0, r, 0xF, color);
}

void TFTLCD::drawCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername,
			uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;


  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    } 
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

// fill a rectangle
void TFTLCD::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
		      uint16_t fillcolor) {
  // smarter version
  while (h--)
    drawHorizontalLine(x, y++, w, fillcolor);
}


void TFTLCD::drawVerticalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color)
{
  if (x >= _width) return;
  drawLine(x, y, x, y + length, color);
  //drawFastLine(x,y,length,color,1);
}

void TFTLCD::drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color)
{
  if (y >= _height) return;
  //drawFastLine(x,y,length,color,0);
  drawLine(x, y, x + length, y, color);
}

void TFTLCD::drawFastLine(uint16_t x, uint16_t y, uint16_t length, 
			  uint16_t color, uint8_t rotflag)
{
  //uint16_t newentrymod;
  //newentrymod = 0x1028;   // we want a 'vertical line'1028
    
  
  //writeRegister(0x03, newentrymod);

if(rotflag == 0){
  for(int i = 0; i < length; i++){
      drawPixel(x + i, y, color);
  }
} else {
    for(int i = 0; i < length; i++){
      drawPixel(x, y + i, color);
    }
}
/*
  writeRegister(0x20, x); // GRAM Address Set (Horizontal Address) (R20h)
  writeRegister(0x21, y); // GRAM Address Set (Vertical Address) (R21h)
  writeCommand(0x22);  // Write Data to GRAM (R22h)


  *portOutputRegister(csport) &= ~cspin;
  //digitalWrite(_cs, LOW);
  *portOutputRegister(cdport) |= cdpin;
  //digitalWrite(_cd, HIGH);
  *portOutputRegister(rdport) |= rdpin;
  //digitalWrite(_rd, HIGH);
  *portOutputRegister(wrport) |= wrpin;
  //digitalWrite(_wr, HIGH);

  setWriteDir();
  while (length--) {
    writeData_unsafe(color); 
  }

  // set back to default
  *portOutputRegister(csport) |= cspin;
  //digitalWrite(_cs, HIGH);
  writeRegister(TFTLCD_ENTRY_MOD, 0x1028);
  */
}



// bresenham's algorithm - thx wikpedia
void TFTLCD::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
		      uint16_t color) {
  // if you're in rotation 1 or 3, we need to swap the X and Y's

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  //dy = abs(y1 - y0);
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;}

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}


void TFTLCD::fillScreen(uint16_t color) {
  //goHome();
  //fillRect(0,0, 320, 240, color);
  writeCommand(0x2a); // Column Address Set
  writeData(1>>8);
  writeData(1);
  writeData((1+1-1)>>8);
  writeData(1+1-1);
  writeCommand(0x2b); // Page Address Set
  writeData(1>>8); 
  writeData(1);
  writeData((1+1-1)>>8);
  writeData(1+1-1);
  writeCommand(0x2c); // Memory Write
 
  byte chigh=color >> 8;
  byte clow=color;
  int i,j;
  for(i=0;i< 320;i++)
    for(j=0;j< 240;j++){
      writeData(chigh);
      writeData(clow);
    }
  //digitalWrite(_cs, HIGH);
  
}

void TFTLCD::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
   // drawFastLine(x, y, 2, color, 1);
    
  writeCommand(0x2a); // Column Address Set
  writeData(y>>8);
  writeData(y);
  writeData((y+1-1)>>8);
  writeData(y+1-1);
  writeCommand(0x2b); // Page Address Set
  writeData(x>>8); 
  writeData(x);
  writeData((x+1-1)>>8);
  writeData(x+1-1);
  writeCommand(0x2c); // Memory Write
 
  byte chigh=color >> 8;
  byte clow=color;
  int i,j;
      writeData(chigh);
      writeData(clow);

/*
  writeRegister(TFTLCD_GRAM_HOR_AD, x); // GRAM Address Set (Horizontal Address) (R20h)
  writeRegister(TFTLCD_GRAM_VER_AD, y); // GRAM Address Set (Vertical Address) (R21h)
  writeCommand(TFTLCD_RW_GRAM);  // Write Data to GRAM (R22h)
  writeData(color);
  */
}

void TFTLCD::initDisplay(void) {
    setWriteDir();
  
  // Set pins A0-A4 as output
    DDRC = DDRC | B00011111; 
    PORTC = PORTC | B00010000; // 1
    delay(10);
    PORTC = PORTC & B11101111; // 0
    delay(20);
    PORTC = PORTC | B00010000; // 1
    delay(20);
    
    // CS HIGH, WR HIGH, RD HIGH, CS LOW
    PORTC = PORTC | B00001000; // CS 1
    PORTC = PORTC | B00000010; // WR 1
    PORTC = PORTC | B00000001; // RD 1
    PORTC = PORTC & B11110111; // CS 0
    
    
  
    writeCommand(0xF7); // Pump ratio control
    writeData(0x20); // 
    
    writeCommand(0x3A); // COLMOD: Pixel Format Set
    writeData(0x55); 
    
    writeCommand(0x36); // Memory Access Control 
    // MY  - Row Address Order (bit7)
    // MX  - Column Address Order
    // MV  - Row / Column Exchange
    // ML  - Vertical Refresh Order
    // BGR - RGB-BGR Order
    // MH  - Horizontal Refresh ORDER(bit2)
    writeData(B00001000); 
    
    writeCommand(0x11); // Sleep OUT
    writeCommand(0x29); // Display ON
    
    delay(50);
}

uint8_t TFTLCD::getRotation(void) {
  return rotation;
}

void TFTLCD::setRotation(uint8_t x) {
  //  writeRegister(0x03, 0x1028); // ??? commented out by non on 2014/2/11

  x %= 4;  // cant be higher than 3
  rotation = x;
  switch (x) {
  case 0:
    _width = TFTWIDTH; 
    _height = TFTHEIGHT;
    break;
  case 1:
    _width = TFTHEIGHT; 
    _height = TFTWIDTH;
    break;
  case 2:
    _width = TFTWIDTH; 
    _height = TFTHEIGHT;
    break;
  case 3:
    _width = TFTHEIGHT; 
    _height = TFTWIDTH;
    break;
 }
}

/********************************* low level pin initialization */

TFTLCD::TFTLCD(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset) {

    
   

  cursor_y = cursor_x = 0;
  textsize = 1;
  textcolor = 0xFFFF;
}


/********************************** low level pin interface */

void TFTLCD::reset(void) {
  if (_reset)
    digitalWrite(_reset, LOW);
  delay(2); 
  if (_reset)
    digitalWrite(_reset, HIGH);

  // resync
  writeData(0);
  writeData(0);
  writeData(0);  
  writeData(0);
}

inline void TFTLCD::setWriteDir(void) {
// Pins 7-2 as output, no change for pins 1,0 (RX TX)
  DDRD = DDRD | B11111100; 
  // Pins 8-9 as output
  DDRB = DDRB | B00000011; 
}

inline void TFTLCD::setReadDir(void) {
// Pins 7-2 as input, no change for pins 1,0 (RX TX)
  DDRD = DDRD & B00000011; 
  // Pins 8-9 as input
  DDRB = DDRB & B11111100; 
}

inline void TFTLCD::write8(uint8_t d) {
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328) || (__AVR_ATmega8__)

  DATAPORT2 = (DATAPORT2 & DATA1_MASK) | 
    (d & DATA2_MASK);
  DATAPORT1 = (DATAPORT1 & DATA2_MASK) | 
    (d & DATA1_MASK); // top 6 bits
  
#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) 


#ifdef USE_ADAFRUIT_SHIELD_PINOUT

  // bit 6/7 (PH3 & 4)
  // first two bits 0 & 1 (PH5 & 6)
  PORTH &= ~(0x78);
  PORTH |= ((d&0xC0) >> 3) | ((d&0x3) << 5);

  // bits 2 & 3 (PB4 & PB5)
  // bit 5 (PB7)
  PORTB &= ~(0xB0); 
  PORTB |= ((d & 0x2C) << 2);

  // bit 4  (PG5)
  if (d & _BV(4))
    PORTG |= _BV(5);
  else
    PORTG &= ~_BV(5);

  #else
     MEGA_DATAPORT = d;  
  #endif

#else
  #error "No pins defined!"
#endif
}

inline uint8_t TFTLCD::read8(void) {
 uint8_t d;
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328) || (__AVR_ATmega8__)

 d = DATAPIN1 & DATA1_MASK; 
 d |= DATAPIN2 & DATA2_MASK; 

#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__)  || defined(__AVR_ATmega1280__) 

#ifdef USE_ADAFRUIT_SHIELD_PINOUT

  // bit 6/7 (PH3 & 4)
  // first two bits 0 & 1 (PH5 & 6)
 d = (PINH & 0x60) >> 5;
 d |= (PINH & 0x18) << 3;

  // bits 2 & 3 & 5 (PB4 & PB5, PB7)
 d |= (PINB & 0xB0) >> 2;

  // bit 4  (PG5)
  if (PING & _BV(5))
    d |= _BV(4);

#else
 d = MEGA_DATAPIN;  
#endif

#else

  #error "No pins defined!"

#endif

 return d;
}

/********************************** low level readwrite interface */
void BD_as_input(void) {
  // Pins 7-2 as input, no change for pins 1,0 (RX TX)
  DDRD = DDRD & B00000011; 
  // Pins 8-9 as input
  DDRB = DDRB & B11111100; 
}
void BD_as_output(void) {
  // Pins 7-2 as output, no change for pins 1,0 (RX TX)
  DDRD = DDRD | B11111100; 
  // Pins 8-9 as output
  DDRB = DDRB | B00000011; 
}
void LCD_write(uint8_t d) {
  // ILI9341 reads data pins when WR rises from LOW to HIGH (A1 pin on arduino)
  PORTC = PORTC & B11111101; // WR 0
  
  // data pins of ILI9341 connected to two arduino ports
  PORTD = (PORTD & B00000011) | ((d) & B11111100); 
  PORTB = (PORTB & B11111100) | ((d) & B00000011); 
  
  PORTC = PORTC | B00000010; // WR 1
}

// the C/D pin is high during write
void TFTLCD::writeData(uint16_t data) {
  PORTC = PORTC | B00000100; // LCD_RS = 1, arduino pin A2
  // write data pins
  LCD_write(data);
}

// this is a 'sped up' version, with no direction setting, or pin initialization
// not for external usage, but it does speed up stuff like a screen fill
inline void TFTLCD::writeData_unsafe(uint16_t data) {
  volatile uint8_t *wrportreg = portOutputRegister(wrport);

  write8(data >> 8);

  *wrportreg &= ~wrpin;
  //digitalWrite(_wr, LOW);
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);

  write8(data);

  *wrportreg &= ~wrpin;
  //digitalWrite(_wr, LOW);
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);
}

// the C/D pin is low during write
void TFTLCD::writeCommand(uint16_t cmd) {
  PORTC = PORTC & B11111011; // LCD_RS = 0, arduino pin A2
  // write data pins
  LCD_write(cmd);
}

uint16_t TFTLCD::readData() {
 // CS LOW, WR HIGH, RD HIGH->LOW>HIGH, RS(D/C) HIGH 
  PORTC = PORTC | B00000100; // RS 1
  
  // LCD_RD - arduino pin A0
  // After RD falls from HIGH to LOW ILI9341 outputs data until RD returns to HIGH
  PORTC = PORTC & B11111110; // RD 0
  BD_as_input(); // Set arduino pins as input
  uint8_t pin72 = PIND & B11111100; // Read data pins 7-2
  uint8_t pin10 = PINB & B00000011; // Read data pins 1-0
  PORTC = PORTC | B00000001; // RD 1
  BD_as_output(); // Re-Set arduino pins as output
  return pin72 | pin10;
}


/************************************* medium level data reading/writing */

uint16_t TFTLCD::readRegister(uint16_t addr) {
   writeCommand(addr);
   return readData();
}

void TFTLCD::writeRegister(uint16_t addr, uint16_t data) {
   writeCommand(addr);
   writeData(data);
}




