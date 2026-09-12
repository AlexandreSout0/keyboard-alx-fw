#include "eyes.h"

using namespace EyeConfig;

static int look_x = 0;
static int look_y = 0;

static bool blinking = false;
static unsigned long blink_start_ms = 0;
static uint16_t blink_duration_ms = 180;

void eyes_look(int dx, int dy) {
    look_x = dx;
    look_y = dy;
}

void eyes_blink(uint16_t duration_ms) {
    blinking = true;
    blink_start_ms = millis();
    blink_duration_ms = duration_ms;
}

static float blink_openness() {
    if (!blinking) return 1.0f;
    unsigned long elapsed = millis() - blink_start_ms;
    float half = blink_duration_ms / 2.0f;
    if (elapsed >= blink_duration_ms) {
        blinking = false;
        return 1.0f;
    }
    float openness;
    if (elapsed < half) {
        openness = 1.0f - (elapsed / half);
    } else {
        openness = (elapsed - half) / half;
    }
    if (openness < 0.05f) openness = 0.05f;
    return openness;
}

static void draw_one_eye(Adafruit_SSD1306 &disp, int center_x, float openness) {
    int h = (int)(EYE_H * openness);
    if (h < 2) h = 2;
    int w = EYE_W;
    int x = center_x - w / 2;
    int y = CY - h / 2;
    int radius = RADIUS;
    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;

    disp.fillRoundRect(x, y, w, h, radius, SSD1306_WHITE);

    if (openness > 0.25f) {
        const int orbit_x = (EYE_W / 2) - PUPIL_R - 2;
        const int orbit_y = (EYE_H / 2) - PUPIL_R - 2;
        int pupil_x = center_x + (look_x * orbit_x);
        int pupil_y = CY + (look_y * orbit_y);
        int pr = (int)(PUPIL_R * openness);
        if (pr < 2) pr = 2;
        disp.fillCircle(pupil_x, pupil_y, pr, SSD1306_BLACK);
    }
}

void eyes_draw(Adafruit_SSD1306 &disp) {
    float openness = blink_openness();
    disp.clearDisplay();
    draw_one_eye(disp, LEFT_CX, openness);
    draw_one_eye(disp, RIGHT_CX, openness);
    disp.display();
}