#include "display.h"
#include "config.h"
#include "driver/uart.h"
#include "eyes.h"
#include "monitor.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <string.h>

TaskHandle_t display_handle = NULL;

// Dois barramentos I2C de hardware distintos do ESP32
static TwoWire WireEsq = TwoWire(0);
static TwoWire WireDir = TwoWire(1);

static Adafruit_SSD1306 display_esq(SCREEN_WIDTH, SCREEN_HEIGHT, &WireEsq, OLED_RESET);
static Adafruit_SSD1306 display_dir(SCREEN_WIDTH, SCREEN_HEIGHT, &WireDir, OLED_RESET);

static bool display_ok_esq = false;
static bool display_ok_dir = false;

static void mostrar_boot_screen(Adafruit_SSD1306 &disp)
{
    disp.clearDisplay();
    disp.setTextColor(SSD1306_WHITE);
    disp.setTextSize(1);
    disp.setCursor(0, 0);
    disp.println("Sout0");
    disp.println("v" VERSION);
    disp.display();
}

void inicio_display_esquerdo()
{
    WireEsq.begin(OLED_SDA, OLED_SCL);

    display_ok_esq = display_esq.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    if (!display_ok_esq)
    {
        const char *msg = "Notice: OLED esquerdo not found\n";
        uart_write_bytes(UART, msg, strlen(msg));
        return;
    }

    mostrar_boot_screen(display_esq);
    monitor_init();
    delay(800);
}

void inicio_display_direito()
{
    WireDir.begin(OLED2_SDA, OLED2_SCL);

    display_ok_dir = display_dir.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    if (!display_ok_dir) {
        const char *msg = "Notice: OLED direito not found\n";
        uart_write_bytes(UART, msg, strlen(msg));
        return;
    }
    mostrar_boot_screen(display_dir);
    delay(800);
}

void ui_request_redraw()
{
    if (display_handle != NULL)
    {
        xTaskNotifyGive(display_handle);
    }
}

static const int look_sequence[][2] = {
    {0, 0},
    {1, 0},
    {0, 0},
    {-1, 0},
    {0, 0},
    {0, -1},
    {0, 0},
    {0, 1},
};
static const int look_sequence_len = sizeof(look_sequence) / sizeof(look_sequence[0]);

void task_display(void *params)
{
    unsigned long last_blink = millis();
    unsigned long next_blink_interval = 3000;

    unsigned long last_look_change = millis();
    unsigned long look_hold_ms = 1500;
    int look_index = 0;

    unsigned long last_monitor_sample = millis();
    const unsigned long monitor_interval_ms = 1000;

    while (true)
    {
        ulTaskNotifyTake(pdTRUE, DISPLAY_REFRESH_MS / portTICK_PERIOD_MS);

        unsigned long now = millis();

        if (now - last_blink > next_blink_interval) {
            eyes_blink();
            last_blink = now;
            next_blink_interval = 2500 + (rand() % 3000);
        }

        if (now - last_look_change > look_hold_ms) {
            look_index = (look_index + 1) % look_sequence_len;
            eyes_look(look_sequence[look_index][0], look_sequence[look_index][1]);
            last_look_change = now;
        }

        if (display_ok_dir) {
            eyes_draw(display_dir);
        }

        if (display_ok_esq && (now - last_monitor_sample > monitor_interval_ms)) {
            monitor_update();
            monitor_draw(display_esq);
            last_monitor_sample = now;
        }
    }
}