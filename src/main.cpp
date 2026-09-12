#include <Arduino.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_task_wdt.h"
#include "config.h"
#include "display.h"
#include "buttons.h"
#include "ble_reader.h"
#include <string>
#include <string.h>


void uart_init(int baudRate, int tx_io_num, int rx_io_num, uart_port_t uart_num)
{
    uart_config_t uart_config =
    {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, tx_io_num, rx_io_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);

}

void setup(){

    uart_init(BAUD_RATE, TX, RX, UART);

    //inicio_display_esquerdo();
    inicio_display_esquerdo();
    inicio_display_direito();

    analogReadResolution(12);
    analogSetPinAttenuation(BTN_KBD1_ADC_PIN, ADC_11db);
    analogSetPinAttenuation(BTN_KBD2_ADC_PIN, ADC_11db);
    analogSetPinAttenuation(BTN_KBD3_ADC_PIN, ADC_11db);
    analogSetPinAttenuation(BTN_CTRL_ADC_PIN, ADC_11db);


    std::string pkg = "VER: " + (std::string) VERSION + (std::string) "\r\n";
    uart_write_bytes(UART, (const char*) pkg.c_str(), pkg.length());
    
    xTaskCreatePinnedToCore(&task_display, "Display Task", 4096, NULL, 2, &display_handle, 0);
    xTaskCreatePinnedToCore(&task_buttons, "Buttons Task",   4096, NULL, 3, NULL,          0);
    xTaskCreatePinnedToCore(&task_ble_reader, "task_ble_reader", 4096, NULL, 1, NULL, 1);


    ui_request_redraw();
}

__attribute__((unused)) void loop()
{
    vTaskDelay(10 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
}