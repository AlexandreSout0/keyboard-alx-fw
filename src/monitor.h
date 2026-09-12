#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

void monitor_init();
void monitor_update();                 // chame periodicamente (ex.: 1x/seg)
void monitor_draw(Adafruit_SSD1306 &disp);