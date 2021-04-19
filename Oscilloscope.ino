#include "Oscilloscope.h"

#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

Oscilloscope *core;

void setup() {
  // put your setup code here, to run once:

#if FASTADC
// set prescale to 16
sbi(ADCSRA,ADPS2) ;
cbi(ADCSRA,ADPS1) ;
cbi(ADCSRA,ADPS0) ;
#endif
  TCCR0B = 0b00000010;  // x8
  TCCR0A = 0b00000001;  // phase corre
  core = new Oscilloscope();
  core->Init(30, 320, 240);
  core->LoadUI();
  //delete ui;
}

void loop() {
  core->Process();
}
