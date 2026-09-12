#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern TaskHandle_t display_handle;

// Inicializa o OLED. Chamar uma vez no setup().
void inicio_display_esquerdo();
void inicio_display_direito();


// Task que redesenha a tela atual periodicamente (ou quando notificada
// via ui_request_redraw()).
void task_display(void *params);

// Acorda a task_display imediatamente, sem esperar o proximo timeout de
// refresh. Pode ser chamada de qualquer task (buttons, pulse, ble).
void ui_request_redraw();
