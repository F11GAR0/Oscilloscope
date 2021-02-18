#ifndef __OSCILLOSCOPE_H__
#define __OSCILLOSCOPE_H__
#include "TFTLCD.h"

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

#define SAMPLES 260
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define BGCOLOR   BLACK
#define GRIDCOLOR BLUE
#define CH1COLOR  GREEN
#define CH2COLOR  BLUE

enum OsciTriggerMode{
    TRIG_AUTO,
    TRIG_NORM,
    TRIG_SCAN,
    TRIG_ONE
};

enum OsciMode{
    MODE_ON,
    MODE_INV,
    MODE_OFF
};

enum OsciRate{
    RATE_F11,
    RATE_F12,
    RATE_F2,
    RATE_5MS,
    RATE_10MS,
    RATE_20MS,
    RATE_50MS,
    RATE_01S,
    RATE_02S,
    RATE_05S,
    RATE_1S,
    RATE_2S,
    RATE_5S,
    RATE_10S
};

enum OsciRange{
    RANGE_1V = 0,
    RANGE_05V,
    RANGE_02V,
    RANGE_01V,
    RANGE_50MV
};

enum OsciTriggerEdge{
    TRIG_E_UP,
    TRIG_E_DN
};

typedef void(*OsciButtonCallback)(void*);

class OsciButton{
    public:
        OsciButton();
        void Init(long *oscip, int x, int y, int w, int h, int color, char* text);
        void SetOnClickCallback(OsciButtonCallback cb);
        void Process();
        void DrawBasics();
        void DrawClicked();
    private:    
        OsciButtonCallback m_cbCallback;
        long* m_pOscilloscope;
        bool m_bIsCallbackRegistered;
        int m_iXPos;
        int m_iYPos;
        int m_iWidth;
        int m_iHeight;
        int m_Color;
        char *m_szText;
};

class OscilloscopeUI{
    public:
        OscilloscopeUI();
        void Init(long *oscip, int dots_div, int lcd_width, int lcd_height);
        void SetChangeTriggerModeCallback(OsciButtonCallback cb);
        void SetChangeModeCallback(OsciButtonCallback cb);
        void SetChangeRateCallback(OsciButtonCallback cb);
        void SetChangeRangeCallback(OsciButtonCallback cb);
        void SetChangeEdgeCallback(OsciButtonCallback cb);
        /// Voltage, rate, grid, numbers, etc.
        void DrawUIBasics(OsciTriggerMode trigger_mode, OsciMode mode, OsciRate rate, OsciRange range, OsciTriggerEdge edge);
        void UpdateTextField(bool only_states, OsciTriggerMode trigger_mode, OsciMode mode, OsciRate rate, OsciRange range, OsciTriggerEdge edge);
        void ProcessButtons();
    private:
        void ClearTextField(bool only_states);
        void AddTextToTextField(const char* text, bool new_line, int color, int text_size, int inc = -1, bool set_pos = true);
        void SetTextFieldXStartPos(int x);
        void SetTextFieldYStartPos(int y);

        long *m_pOscilloscope;

        int m_iTFStartXPos;
        int m_iTFStartYPos;
        int m_iTFCurrentYPos;
        
        OsciButton m_TriggerModeChangeButton;
        OsciButton m_ModeChangeButton;
        OsciButton m_RateChangeButton;
        OsciButton m_RangeChangeButton;
        OsciButton m_EdgeChangeButton;

        int m_iDotsDiv;
        int m_iLCD_WIDTH;
        int m_iLCD_HEIGHT;
};

class Oscilloscope{
    public:
        Oscilloscope();
        void Init(int dots_div, int lcd_width, int lcd_height);
        void LoadUI();
        void Process();
    private:
        void ClearAndDrawDot(int i);

        friend void TriggerModeChange(Oscilloscope*);
        friend void ModeChange(Oscilloscope*);
        friend void RateChange(Oscilloscope*);
        friend void RangeChange(Oscilloscope*);
        friend void EdgeChange(Oscilloscope*);
        inline unsigned long getTransformedVoltage(byte channel);
        OscilloscopeUI m_UI;

        OsciTriggerMode m_TriggerMode;
        OsciMode m_Mode;
        OsciRate m_Rate;
        OsciRange m_Range;
        OsciTriggerEdge m_Edge;

        int m_iDotsDiv;
        int m_iLCD_WIDTH;
        int m_iLCD_HEIGHT;

};

#endif