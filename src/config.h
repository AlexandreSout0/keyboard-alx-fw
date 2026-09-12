#pragma once

// =====================================================================
//                          VERSAO DO FIRMWARE
// =====================================================================
#define VERSION "0.1.0"

// =====================================================================
//                          UART / SERIAL
// =====================================================================
#define BAUD_RATE   115200
#define TX          GPIO_NUM_1
#define RX          GPIO_NUM_3
#define UART        UART_NUM_0
#define BUF_SIZE    1024


// =====================================================================
//                          DISPLAY OLED (SSD1306 128x64, I2C)
// =====================================================================
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64
#define OLED_RESET          -1
#define SCREEN_ADDRESS      0x3C

#define OLED_SDA            GPIO_NUM_21
#define OLED_SCL            GPIO_NUM_22

#define OLED2_SDA           GPIO_NUM_18
#define OLED2_SCL           GPIO_NUM_19


#define DISPLAY_REFRESH_MS  30
#define CONTENT_TOP         16
#define SCROLLBAR_X         124

#define BTN_CTRL_ADC_PIN         14

#define BTN_KBD1_ADC_PIN         34
#define BTN_KBD2_ADC_PIN         35
#define BTN_KBD3_ADC_PIN         32


#define BTN_SAMPLE_MS       20
#define BTN_STABLE_SAMPLES  3
#define BTN_HOLD_MS         600
#define BTN_REPEAT_MS       180
#define BTN_LONG_MS         1500

// #define TH_UP_SEL           850
// #define TH_SEL_DOWN         2125
// #define TH_DOWN_NONE        3322

#define BTN_TH_NAN         3322
#define BTN_TH_2           2425
#define BTN_TH_1           850


