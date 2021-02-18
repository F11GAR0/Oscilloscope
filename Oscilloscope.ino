#include "Oscilloscope.h"

Oscilloscope *core;

void setup() {
  // put your setup code here, to run once:
  core = new Oscilloscope();
  core->Init(30, 320, 240);
  core->LoadUI();
  //delete ui;
}

void loop() {
  core->Process();
}
