#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// =====================================================================
//                    ENUMS E STRUCTS COMPARTILHADOS
// =====================================================================

typedef enum {
    BTN_NONE = 0,
    BTN_UP,
    BTN_DOWN,
    BTN_SEL,
    BTN_1X1,
    BTN_1X2,
    BTN_1X3,
    BTN_2X1,
    BTN_2X2,
    BTN_2X3,
    BTN_3X1,
    BTN_3X2,
    BTN_3X3
} Button_t;

typedef enum {
    SCR_FLOW_METER = 0,
    SCR_NUM_BICOS,
    SCREEN_3,
    SCREEN_4,
    SCR_BLE_CHK,
    SCR_COUNT      // deve ser sempre o ultimo: usado como "quantidade de telas"
} Screen_t;



// =====================================================================
// IMPORTANTE (single-writer): variaveis marcadas "escrito somente por X"
// so podem ser ESCRITAS dentro do modulo X. Qualquer outro modulo pode
// LER livremente, mas NUNCA deve escrever nelas diretamente - sinalize
// intencao atraves dos flags *_requested quando for o caso. Isso evita
// condicao de corrida sem precisar de mutex para cada campo.
//
// A definicao real (com valor inicial) de cada variavel abaixo vive em
// state.cpp - este header so declara.
// =====================================================================

// --- Sequencia de leitura por bico (calibracao) ---
extern volatile bool           seq_start_requested;    // sinalizado por task_buttons / BLE
extern volatile bool           seq_cancel_requested;    // sinalizado por task_buttons / BLE
extern volatile bool           seq_running;              // escrito somente por task_pulse

// --- UI / navegacao ---
extern volatile Screen_t current_screen;
extern volatile int      last_btn_raw;    // leitura bruta do ADC (diagnostico)

// --- Preferencias (NVS) ---
extern Preferences prefs;


extern volatile float stats_cpu_pct;
extern volatile float stats_mem_pct;
extern volatile float stats_disk_pct;
extern volatile unsigned long stats_last_update_ms; // millis() do último pacote válido recebido