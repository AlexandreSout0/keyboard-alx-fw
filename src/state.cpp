#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// state.cpp
#include "state.h"

volatile Screen_t current_screen = SCR_FLOW_METER;
bool bicos_edit_mode = false;


volatile float stats_cpu_pct = 0.0f;
volatile float stats_mem_pct = 0.0f;
volatile float stats_disk_pct = 0.0f;
volatile unsigned long stats_last_update_ms = 0;