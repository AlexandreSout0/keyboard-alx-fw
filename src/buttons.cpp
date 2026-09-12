#include "buttons.h"
#include "state.h"
#include "display.h"
#include "config.h"
#include "ble_reader.h"
#include "driver/uart.h"

#include <stdio.h>
#include <string.h>


static uint8_t classify_line(int raw) {
    if (raw >= BTN_TH_NAN) return 0; // nenhum
    if (raw >= BTN_TH_2)   return 3;
    if (raw >= BTN_TH_1)    return 2;
    return 1;
}

// Mapeia (linha, zona) -> Button_t
static Button_t map_ctrl(uint8_t zone) {
    switch (zone) {
        case 1: return BTN_UP;
        case 2: return BTN_SEL;
        case 3: return BTN_DOWN;
        default: return BTN_NONE;
    }
}

static Button_t map_kbd1(uint8_t zone) {
    switch (zone) {
        case 1: return BTN_1X1;
        case 2: return BTN_2X1;
        case 3: return BTN_3X1;
        default: return BTN_NONE;
    }
}

static Button_t map_kbd2(uint8_t zone) {
    switch (zone) {
        case 1: return BTN_1X2;
        case 2: return BTN_2X2;
        case 3: return BTN_3X2;
        default: return BTN_NONE;
    }
}

static Button_t map_kbd3(uint8_t zone) {
    switch (zone) {
        case 1: return BTN_1X3;
        case 2: return BTN_2X3;
        case 3: return BTN_3X3;
        default: return BTN_NONE;
    }
}

// ---- nome legível de cada botão, para debug via UART ----
static const char *btn_name(Button_t b) {
    switch (b) {
        case BTN_NONE: return "NONE";
        case BTN_UP:   return "BTN_UP";
        case BTN_DOWN: return "BTN_DOWN";
        case BTN_SEL:  return "BTN_SEL";
        case BTN_1X1:  return "BTN_1X1";
        case BTN_1X2:  return "BTN_1X2";
        case BTN_1X3:  return "BTN_1X3";
        case BTN_2X1:  return "BTN_2X1";
        case BTN_2X2:  return "BTN_2X2";
        case BTN_2X3:  return "BTN_2X3";
        case BTN_3X1:  return "BTN_3X1";
        case BTN_3X2:  return "BTN_3X2";
        case BTN_3X3:  return "BTN_3X3";
        default:       return "UNKNOWN";
    }
}

static void uart_print_button(Button_t b) {
    char msg[48];
    int n = snprintf(msg, sizeof(msg), "%s\n", btn_name(b));
    ble_send_line(msg);
    uart_write_bytes(UART, msg, n);

    
}

static void ui_press(Button_t b, bool repeat) {
    int step = repeat ? 1 : 2;
    ble_send_line(("BTN_TS:" + String(millis()) + "\n").c_str());

    uart_print_button(b);

    switch (b) {
        case BTN_UP:
            if (current_screen == 0) {
                current_screen = (Screen_t)(SCR_COUNT - 1);
            } else {
                current_screen = (Screen_t)(current_screen - 1);
            }
            break;

        case BTN_DOWN:
            current_screen = (Screen_t)((current_screen + 1) % SCR_COUNT);
            break;

        case BTN_SEL:
            break;

        default:
            break;
    }

    ui_request_redraw();
}


static void ui_long_press(Button_t b) {
    if (b != BTN_SEL) return;

    const char *msg = "Botao: SEL (long press)\n";
    //uart_write_bytes(UART, msg, strlen(msg));

    // TODO: ajuste os cases abaixo para os nomes reais do seu enum Screen_t
    switch (current_screen) {
        case SCR_NUM_BICOS:
            ui_request_redraw();
            break;

        case SCR_BLE_CHK:
            ui_request_redraw();
            break;

        default:
            ui_request_redraw();
            break;
    }
}

// Estado de debounce/hold/repeat para uma linha analógica
struct ButtonLine {
    int adc_pin;
    Button_t (*map_fn)(uint8_t zone);

    Button_t current   = BTN_NONE;
    Button_t candidate = BTN_NONE;
    uint8_t  samples   = 0;
    TickType_t press_start = 0;
    TickType_t last_repeat = 0;
    bool long_fired = false;

    // construtor explícito: resolve o erro de inicialização por lista
    ButtonLine(int pin, Button_t (*fn)(uint8_t))
        : adc_pin(pin), map_fn(fn) {}
};

static void process_line(ButtonLine &line, TickType_t tick) {

    int raw = analogRead(line.adc_pin);
    // char msg[16];
    // int len = snprintf(msg, sizeof(msg), "%d\r\n", raw);
    // uart_write_bytes(UART, msg, len);

    uint8_t zone = classify_line(raw);
    Button_t now = line.map_fn(zone);

    if (now == line.candidate) {
        if (line.samples < BTN_STABLE_SAMPLES) line.samples++;
    } else {
        line.candidate = now;
        line.samples = 0;
    }

    if (line.samples >= BTN_STABLE_SAMPLES && line.candidate != line.current) {
        line.current = line.candidate;

        if (line.current != BTN_NONE) { // borda de descida (ação imediata)
            line.press_start = tick;
            line.last_repeat = tick;
            line.long_fired  = false;
            ui_press(line.current, false);
        }
    }
    else if (line.current != BTN_NONE && line.candidate == line.current) {
        TickType_t held = tick - line.press_start;

        if (line.current == BTN_SEL) {
            if (!line.long_fired && held >= pdMS_TO_TICKS(BTN_LONG_MS)) {
                line.long_fired = true;
                ui_long_press(BTN_SEL);
            }
        } else {
            if (held >= pdMS_TO_TICKS(BTN_HOLD_MS) &&
                (tick - line.last_repeat) >= pdMS_TO_TICKS(BTN_REPEAT_MS)) {
                line.last_repeat = tick;
                ui_press(line.current, true);
            }
        }
    }
}

void task_buttons(void *params) {

    static ButtonLine lines[] = {
        { BTN_CTRL_ADC_PIN, map_ctrl },
        { BTN_KBD1_ADC_PIN, map_kbd1 },
        { BTN_KBD2_ADC_PIN, map_kbd2 },
        { BTN_KBD3_ADC_PIN, map_kbd3 },
    };

    const int num_lines = sizeof(lines) / sizeof(lines[0]);

    while (true) {

        // char msg[48] = "teste";
        // int len = strlen(msg);
        // uart_write_bytes(UART, msg, len);

        TickType_t tick = xTaskGetTickCount();

        for (int i = 0; i < num_lines; i++) {
            process_line(lines[i], tick);
        }

        vTaskDelay(BTN_SAMPLE_MS / portTICK_PERIOD_MS);
    }
}