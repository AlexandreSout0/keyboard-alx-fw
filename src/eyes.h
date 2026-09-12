#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// Configuração visual dos olhos (compartilhada pelos dois)
namespace EyeConfig {
    constexpr int EYE_W    = 40;
    constexpr int EYE_H    = 40;
    constexpr int RADIUS   = 10;
    constexpr int PUPIL_R  = 8;

    constexpr int LEFT_CX  = 32;
    constexpr int RIGHT_CX = 96;
    constexpr int CY       = 32;
}

void eyes_look(int dx, int dy);                    // dx, dy: -1, 0, 1
void eyes_blink(uint16_t duration_ms = 180);
void eyes_draw(Adafruit_SSD1306 &disp);