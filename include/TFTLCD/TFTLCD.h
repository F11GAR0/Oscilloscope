#ifndef __TFTLCD_H__
#define __TFTLCD_H__

#include <Arduino.h>

#define	BLACK           0x0000
#define	BLUE            0x001F
#define CYAN            0x07FF
#define	GREEN           0x07E0
#define MAGENTA         0xF81F
#define	RED             0xF800
#define YELLOW          0xFFE0 
#define WHITE           0xFFFF

// comment or uncomment the next line for special pinout!
//#define USE_ADAFRUIT_SHIELD_PINOUT

class TFTLCD : public Print {
 public:
  TFTLCD(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset);

  uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);

  // drawing primitives!
  void drawPixel(uint16_t x, uint16_t y, uint16_t color);
  void fillScreen(uint16_t color);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void drawFastLine(uint16_t x0, uint16_t y0, uint16_t l, uint16_t color, uint8_t flag);
  void drawVerticalLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color);
  void drawHorizontalLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color);
  void drawTriangle(uint16_t x0, uint16_t y0,
			  uint16_t x1, uint16_t y1,
			  uint16_t x2, uint16_t y2, uint16_t color);
  void fillTriangle(int32_t x0, int32_t y0,
			  int32_t x1, int32_t y1,
		    int32_t x2, int32_t y2, 
		    uint16_t color);
  void drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
  void fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
  void drawRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
  void fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
  void drawCircle(uint16_t x0, uint16_t y0, uint16_t r,	uint16_t color);
  void fillCircle(uint16_t x0, uint16_t y0, uint16_t r,	uint16_t color);

  void setCursor(uint16_t x, uint16_t y);
  void setTextColor(uint16_t c);
  void setTextSize(uint8_t s);
  virtual size_t write(uint8_t);

  void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t s = 1);
  void drawString(uint16_t x, uint16_t y, char *c, uint16_t color, uint8_t s = 1);

  byte ReadTouch(int *row, int *col);

  // commands
  void initDisplay(void);
  void goHome(void);
  void goTo(int x, int y);

  void reset(void);
  void setRotation(uint8_t x);
  uint8_t getRotation();

  /* low level */

  void writeData(uint16_t d);
  void writeCommand(uint16_t c);
  uint16_t readData(void);
  uint16_t readRegister(uint16_t addr);
  void writeRegister(uint16_t addr, uint16_t data);


  uint16_t width();
  uint16_t height();

  static const uint16_t TFTWIDTH = 240;
  static const uint16_t TFTHEIGHT = 320;

  void writeData_unsafe(uint16_t d);

  void setWriteDir(void);
  void setReadDir(void);


  void write8(uint8_t d);

 private:
  void drawCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t corner, uint16_t color);
  void fillCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t corner, uint16_t delta, uint16_t color);

  uint8_t read8(void);

  uint8_t _cs, _cd, _reset, _wr, _rd;

  uint8_t csport, cdport, wrport, rdport;
  uint8_t cspin, cdpin, wrpin, rdpin;

  uint16_t _width, _height;
  uint8_t textsize;
  uint16_t cursor_x, cursor_y;
  uint16_t textcolor;
  uint8_t rotation;
};
#endif
