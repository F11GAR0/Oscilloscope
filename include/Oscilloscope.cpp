#include "Oscilloscope.h"

TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

const char *g_szModes[] = {" NORM", " INV", " OFF"};
const char *g_szTriggerModes[] = {" Auto", " Norm", " Scan", " One"};
const char *g_szRates[] = {"F1-1", "F1-2 ", "F2  ", "5ms", "10ms", "20ms", "50ms", "0.1s", "0.2s", "0.5s", "1s", "2s", "5s", "10s"};
const char *g_szRanges[] = {"1V ", "0.5V", "0.2V", "0.1V", "50mV"};

const byte ad_sw =  3;                    // Analog 3 pin for switches
const byte ad_ch0 = 4;                   // Analog 4 pin for channel 0
const byte ad_ch1 = 5;                   // Analog 5 pin for channel 1

const unsigned short VREF[] = {150, 300, 750, 1500, 3000};
const unsigned short VRATES[] = { 1, 2, 3, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000};

const byte g_CorrectionOffset = 204;

static short g_HistoryWaveTable[SAMPLES];
static short g_WaveTable[SAMPLES];

OsciButton::OsciButton(){ }

int strlen(char *str){
    int len = 0;
    int i = 0;
    while(str[i++] != 0){
        len++;
    }
    return len;
}

void OsciButton::DrawBasics(){
    for (int y= m_iYPos; y < m_iYPos + m_iHeight; y += 2) {
        tft.drawLine(m_iXPos, y, m_iXPos + m_iWidth, y, BGCOLOR);
        tft.drawLine(m_iXPos, y + 1, m_iXPos + m_iWidth, y + 1, BGCOLOR);
    }
    tft.drawString(m_iXPos + 5, m_iYPos + 6, m_szText, m_Color, 1);
    tft.drawRoundRect(m_iXPos, m_iYPos, m_iWidth, m_iHeight, 4, m_Color);
}

void OsciButton::DrawClicked(){
    tft.drawString(m_iXPos + 5, m_iYPos + 6, m_szText, CYAN, 1);
    tft.drawRoundRect(m_iXPos, m_iYPos, m_iWidth, m_iHeight, 4, CYAN);
}

void OsciButton::Init(long *oscip, int x, int y, int w, int h, int color, char* text){
    m_iXPos = x;
    m_iYPos = y;
    m_iWidth = w;
    m_iHeight = h;
    m_bIsCallbackRegistered = false;
    
    m_pOscilloscope = oscip;
    
    m_Color = color;
    int text_len = strlen(text);
    m_szText = (char*)malloc(text_len + 1);
    memcpy(m_szText, text, text_len + 1);
    m_szText[text_len] = '\0';
}

void OsciButton::SetOnClickCallback(OsciButtonCallback cb){
    m_cbCallback = cb;
    m_bIsCallbackRegistered = true;
}

void OsciButton::Process(){
    int x = 0;
    int y = 0;
    if(tft.ReadTouch(&y, &x)){
        y = 240 - y;
        x += 10;
        if(x > m_iXPos && x < m_iXPos + m_iWidth){
            if(y > m_iYPos && y < m_iYPos + m_iHeight){
                if(m_bIsCallbackRegistered == true) 
                    m_cbCallback((void*)m_pOscilloscope);
                DrawClicked();
                delay(60);
                DrawBasics();
            }
        }
    }
}

OscilloscopeUI::OscilloscopeUI(){ }

void OscilloscopeUI::Init(long *oscip, int dots_div, int lcd_width, int lcd_height){
    m_iDotsDiv = dots_div;
    m_iLCD_WIDTH = lcd_width;
    m_iLCD_HEIGHT = lcd_height;
    m_iTFStartXPos = SAMPLES + 3;
    m_iTFStartYPos = 20;
    m_iTFCurrentYPos = 20;
    
    m_pOscilloscope = oscip;

    tft.initDisplay();
    //fill screen with black
    for (int y=0; y < 239; y += 2) {
        tft.drawLine(0, y, m_iLCD_WIDTH, y, BGCOLOR);
        tft.drawLine(0, y + 1, m_iLCD_WIDTH, y + 1, BGCOLOR);
    }

    m_TriggerModeChangeButton.Init(m_pOscilloscope, m_iTFStartXPos + 30, 80, 20, 20, RED, "T");
    m_ModeChangeButton.Init(m_pOscilloscope, m_iTFStartXPos + 30,        110, 20, 20, RED, "M");
    m_RateChangeButton.Init(m_pOscilloscope, m_iTFStartXPos + 30,        140, 20, 20, RED, "RT");
    m_RangeChangeButton.Init(m_pOscilloscope, m_iTFStartXPos + 30,       170, 20, 20, RED, "RG");
    m_EdgeChangeButton.Init(m_pOscilloscope, m_iTFStartXPos + 30,        200, 20, 20, RED, "E");
}

void OscilloscopeUI::SetChangeTriggerModeCallback(OsciButtonCallback cb){
    m_TriggerModeChangeButton.SetOnClickCallback(cb);
}

void OscilloscopeUI::SetChangeModeCallback(OsciButtonCallback cb){
    m_ModeChangeButton.SetOnClickCallback(cb);
}

void OscilloscopeUI::SetChangeRateCallback(OsciButtonCallback cb){
    m_RateChangeButton.SetOnClickCallback(cb);
}

void OscilloscopeUI::SetChangeRangeCallback(OsciButtonCallback cb){
    m_RangeChangeButton.SetOnClickCallback(cb);
}

void OscilloscopeUI::SetChangeEdgeCallback(OsciButtonCallback cb){
    m_EdgeChangeButton.SetOnClickCallback(cb);
}

void OscilloscopeUI::ClearTextField(bool only_states){
    if(!only_states){
        for (int y = m_iTFStartYPos; y < m_iTFCurrentYPos + 10 - 1; y += 2) {
            tft.drawLine(m_iTFStartXPos, y, m_iLCD_WIDTH, y, BGCOLOR);
            tft.drawLine(m_iTFStartXPos, y + 1, m_iLCD_WIDTH, y + 1, BGCOLOR);
        }
        m_iTFCurrentYPos = m_iTFStartYPos;
    } else {
        for (int y = m_iTFStartYPos; y < m_iTFStartYPos + 50 + 10 - 1; y += 2) {
            tft.drawLine(m_iTFStartXPos, y, m_iLCD_WIDTH, y, BGCOLOR);
            tft.drawLine(m_iTFStartXPos, y + 1, m_iLCD_WIDTH, y + 1, BGCOLOR);
        }
    }
}

void OscilloscopeUI::AddTextToTextField(const char* text, bool new_line, int color, int text_size, int inc, bool set_pos = true){
    tft.setTextColor(color);
    tft.setTextSize(text_size);
    if(set_pos)
        tft.setCursor(m_iTFStartXPos, m_iTFCurrentYPos);
    if(new_line){
        tft.println(text);
        if(inc != -1){
            m_iTFCurrentYPos += inc;
        } else {
            m_iTFCurrentYPos += 10;
        }
    }
    else
        tft.print(text);
    
}

void OscilloscopeUI::UpdateTextField(bool only_states, OsciTriggerMode trigger_mode, OsciMode mode, OsciRate rate, OsciRange range, OsciTriggerEdge edge){
    ClearTextField(only_states);
    int temp_y;
    if(only_states){
        temp_y = m_iTFCurrentYPos;
        m_iTFCurrentYPos = m_iTFStartYPos;
    }
    AddTextToTextField(g_szRanges[(int)range], 0, YELLOW, 1);
    AddTextToTextField("/DIV", 1, YELLOW, 1, 10, false);
    AddTextToTextField(g_szRates[(int)rate], 0, YELLOW, 1);
    AddTextToTextField("/DIV", 1, YELLOW, 1, 10, false);
    AddTextToTextField(g_szTriggerModes[(int)trigger_mode], 1, YELLOW, 1);
    AddTextToTextField(edge == OsciTriggerEdge::TRIG_E_UP ? " UP" : " DN", 1, YELLOW, 1);
    AddTextToTextField(g_szModes[(int)mode], 1, YELLOW, 1, 30);

    if(only_states){
        m_iTFCurrentYPos = temp_y;
        m_TriggerModeChangeButton.DrawBasics();
        m_ModeChangeButton.DrawBasics();
        m_RateChangeButton.DrawBasics();
        m_RangeChangeButton.DrawBasics();
    } else {
        AddTextToTextField(" 1V ", 1, MAGENTA, 1, 30);
        AddTextToTextField(" 2V ", 1, MAGENTA, 1, 30);
        AddTextToTextField(" 3V ", 1, MAGENTA, 1, 30);
        AddTextToTextField(" 4V ", 1, MAGENTA, 1, 30);
        AddTextToTextField(" 5V ", 1, MAGENTA, 1, 30);

        m_TriggerModeChangeButton.DrawBasics();
        m_ModeChangeButton.DrawBasics();
        m_RateChangeButton.DrawBasics();
        m_RangeChangeButton.DrawBasics();
        m_EdgeChangeButton.DrawBasics();
    }
}

void OscilloscopeUI::DrawGrid(){
    for (int x=0; x<=SAMPLES; x += 2) { // Horizontal Line
      for (int y=0; y <= m_iLCD_HEIGHT; y += m_iDotsDiv) {
        tft.drawPixel(x, y, GRIDCOLOR);
      }
      if (m_iLCD_HEIGHT == 240)
        tft.drawPixel(x, m_iLCD_HEIGHT - 1, GRIDCOLOR);
    }
    for (int x=0; x <= SAMPLES; x += m_iDotsDiv ) { // Vertical Line
      for (int y=0; y <= m_iLCD_HEIGHT; y += 2) {
        tft.drawPixel(x, y, GRIDCOLOR);
      }
    }
}

void OscilloscopeUI::DrawUIBasics(OsciTriggerMode trigger_mode, OsciMode mode, OsciRate rate, OsciRange range, OsciTriggerEdge edge){
    DrawGrid();
    UpdateTextField(false, trigger_mode,mode,rate,range,edge);
}

void OscilloscopeUI::ProcessButtons(){
    m_TriggerModeChangeButton.Process();
    m_ModeChangeButton.Process();
    m_RateChangeButton.Process();
    m_RangeChangeButton.Process();
    m_EdgeChangeButton.Process();
}

Oscilloscope::Oscilloscope(){ }

void Oscilloscope::Init(int dots_div, int lcd_width, int lcd_height){
    m_UI.Init((long*)this, dots_div, lcd_width, lcd_height);
    m_TriggerMode = 0;
    m_Mode = 0;
    m_Rate = RATE_20MS;
    m_Range = RANGE_01V;
    m_Edge = 0;
    m_iDotsDiv = dots_div;
    m_iLCD_WIDTH = lcd_width;
    m_iLCD_HEIGHT = lcd_height;
}

void TriggerModeChange(Oscilloscope* self){
    switch(self->m_TriggerMode){
      case TRIG_AUTO:
        self->m_TriggerMode = TRIG_NORM;
        break;
      case TRIG_NORM:
        self->m_TriggerMode = TRIG_SCAN;
        break;
      case TRIG_SCAN:
        self->m_TriggerMode = TRIG_ONE;
        break;
      case TRIG_ONE:
        self->m_TriggerMode = TRIG_AUTO;
        break;
    }
    self->m_UI.UpdateTextField(true, self->m_TriggerMode, self->m_Mode, self->m_Rate, self->m_Range, self->m_Edge);
}

void ModeChange(Oscilloscope* self){
    switch(self->m_Mode){
        case MODE_ON:
            self->m_Mode = MODE_INV;
            break;
        case MODE_INV:
            self->m_Mode = MODE_OFF;
            break;
        case MODE_OFF:
            self->m_Mode = MODE_ON;
            break;
    }
    self->m_UI.UpdateTextField(true, self->m_TriggerMode, self->m_Mode, self->m_Rate, self->m_Range, self->m_Edge);
}

void RateChange(Oscilloscope* self){
    for(int i = 0; i < SAMPLES; i++){
        g_WaveTable[i] = 0;
        g_HistoryWaveTable[i] = 0;
        //g_iWavePartCount = 0;
        //g_iHistoryWavePartCount = 0;
    }
    switch(self->m_Rate){
        case RATE_F11:
            self->m_Rate = RATE_F12;
            break;
        case RATE_F12:
            self->m_Rate = RATE_F2;
            break;
        case RATE_F2:
            self->m_Rate = RATE_5MS;
            break;
        case RATE_5MS:
            self->m_Rate = RATE_10MS;
            break;
        case RATE_10MS:
            self->m_Rate = RATE_20MS;
            break;
        case RATE_20MS:
            self->m_Rate = RATE_50MS;
            break;
        case RATE_50MS:
            self->m_Rate = RATE_01S;
            break;
        case RATE_01S:
            self->m_Rate = RATE_02S;
            break;
        case RATE_02S:
            self->m_Rate = RATE_05S;
            break;
        case RATE_05S:
            self->m_Rate = RATE_1S;
            break;
        case RATE_1S:
            self->m_Rate = RATE_2S;
            break;
        case RATE_2S:
            self->m_Rate = RATE_5S;
            break;
        case RATE_5S:
            self->m_Rate = RATE_10S;
            break;
        case RATE_10S:
            self->m_Rate = RATE_F11;
            break;
    }
    self->m_UI.UpdateTextField(true, self->m_TriggerMode, self->m_Mode, self->m_Rate, self->m_Range, self->m_Edge);
}

void RangeChange(Oscilloscope* self){
    switch(self->m_Range){
        case RANGE_1V:
            self->m_Range = RANGE_05V;
            break;
        case RANGE_05V:
            self->m_Range = RANGE_02V;
            break;
        case RANGE_02V:
            self->m_Range = RANGE_01V;
            break;
        case RANGE_01V:
            self->m_Range = RANGE_50MV;
            break;
        case RANGE_50MV:
            self->m_Range = RANGE_1V;
            break;
    }
    self->m_UI.UpdateTextField(true, self->m_TriggerMode, self->m_Mode, self->m_Rate, self->m_Range, self->m_Edge);
}

void EdgeChange(Oscilloscope* self){
    switch(self->m_Edge){
        case TRIG_E_UP:
            self->m_Edge = TRIG_E_DN;
            break;
        case TRIG_E_DN:
            self->m_Edge = TRIG_E_UP;
            break;
    }
    self->m_UI.UpdateTextField(true, self->m_TriggerMode, self->m_Mode, self->m_Rate, self->m_Range, self->m_Edge);
}

void Oscilloscope::LoadUI(){
    m_UI.DrawUIBasics(m_TriggerMode, m_Mode, m_Rate, m_Range, m_Edge);
    m_UI.SetChangeTriggerModeCallback(TriggerModeChange);
    m_UI.SetChangeModeCallback(ModeChange);
    m_UI.SetChangeRateCallback(RateChange);
    m_UI.SetChangeRangeCallback(RangeChange);
    m_UI.SetChangeEdgeCallback(EdgeChange);
}

inline unsigned long Oscilloscope::getTransformedVoltage(byte channel){
    unsigned long a = analogRead(channel);

    a = (a * VREF[(int)m_Range]) >> 10; //0 - range [0 - 150]

    a = VREF[(int)m_Range] - a;
    a = a >= (m_iLCD_HEIGHT + 61) ? m_iLCD_HEIGHT - 1 : a;

    if (m_Mode == MODE_INV)
        return m_iLCD_HEIGHT - a;
    return a;
}

void Oscilloscope::DrawDebugStr(char *str){
    for (int y = 10; y < 20; y += 2) {
        tft.drawLine(10, y, 120, y, BGCOLOR);
        tft.drawLine(10, y + 1, 120, y + 1, BGCOLOR);
    }
    tft.drawString(10, 10, str, RED, 1);
}

inline void Oscilloscope::ClearAndDrawDot(int32_t i){
    
    if (i <= 1)
        return;
    tft.drawLine(i - 1, g_HistoryWaveTable[i - 1] + 60, i, g_HistoryWaveTable[i] + 60, BGCOLOR);
    if (m_Mode != MODE_OFF)
       tft.drawLine(i - 1, g_WaveTable[i - 1] + 60, i, g_WaveTable[i] + 60, CH1COLOR);
}
/*
void Oscilloscope::Process(){
    m_UI.ProcessButtons(); 
    for(int i = 0; i < SAMPLES; i++){
        g_WaveTable[i] = getTransformedVoltage(ad_ch1);
        g_iWavePartCount++;
        if((int)m_Rate > 6){
            static unsigned long time = millis();
            unsigned long tick = millis();
            if(time + VRATES[(int)m_Rate] < tick){
                time = tick;
                break;
            }
        }
        int32_t predelay = VRATES[(int)m_Rate] / SAMPLES;
        if(predelay > 0)
            delay(predelay);
    }    
    for(int i = 0; i < g_iHistoryWavePartCount; i++){
        ClearDot(i);
    }
    //m_UI.DrawGrid();
    for(int i = 0; i < g_iWavePartCount; i++){
        DrawDot(i);
    }

    g_iHistoryWavePartCount = g_iWavePartCount;
    g_iWavePartCount = 0;
    for(int i = 0; i < SAMPLES; i++){
        g_HistoryWaveTable[i] = g_WaveTable[i];
    }
}
*/

void Oscilloscope::Process(){
    m_UI.ProcessButtons(); 

    switch(m_Rate){
        case RATE_F11: // only first channel
        case RATE_F12: // dual channel
        case RATE_F2: // second channel
        {
            for(int i = 0; i < SAMPLES; i++){ //just do samples
                g_WaveTable[i] = getTransformedVoltage(ad_ch1);
            }
            break;
        }
        case RATE_5MS:
        case RATE_10MS:
        case RATE_20MS:
        {
            const unsigned long r_[] = { 5000 / m_iDotsDiv, 10000 / m_iDotsDiv, 20000 / m_iDotsDiv };
            unsigned long st0 = millis();
            unsigned long st = micros();
            unsigned long r = r_[(int)m_Rate - 3];
            for (int i = 0; i < SAMPLES; i++) {
                while((st - micros()) < r) ;
                st += r;
                g_WaveTable[i] = getTransformedVoltage(ad_ch1);
            }
            break;
        }
        case RATE_50MS: // 6 
        case RATE_01S:
        case RATE_02S:
        case RATE_05S:
        case RATE_1S:
        case RATE_2S:
        case RATE_5S:
        case RATE_10S:
        {
            m_UI.DrawGrid();
            const unsigned long r_[] = {50000/m_iDotsDiv, 100000/m_iDotsDiv, 200000/m_iDotsDiv,
                      500000/m_iDotsDiv, 1000000/m_iDotsDiv, 2000000/m_iDotsDiv,
                      5000000/m_iDotsDiv, 10000000/m_iDotsDiv};
            unsigned long st0 = millis();
            unsigned long st = micros();
            for (int i = 0; i < SAMPLES; i++) {
                while((st - micros()) < r_[(int)m_Rate - 6]) {
                    m_UI.ProcessButtons(); 
                    if ((int)m_Rate < 6)
                        break;
                }
                if ((int)m_Rate < 6) { // sampling rate has been changed
                    m_UI.DrawGrid();
                    break;
                }
                st += r_[(int)m_Rate - 6];
                if (st - micros() > r_[(int)m_Rate - 6])
                    st = micros(); // sampling rate has been changed to shorter interval
                g_WaveTable[i] = getTransformedVoltage(ad_ch1);
                ClearAndDrawDot(i);     
            }
            for(int i = 0; i < SAMPLES; i++){
                g_HistoryWaveTable[i] = g_WaveTable[i];
            }
            return;
        }
    }   
    m_UI.DrawGrid();
    for(int i = 0; i < SAMPLES; i++){
        ClearAndDrawDot(i);
    }
    for(int i = 0; i < SAMPLES; i++){
        g_HistoryWaveTable[i] = g_WaveTable[i];
    }
}