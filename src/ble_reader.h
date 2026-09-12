#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BleReader {
public:
    BleReader(size_t buffer_size = 128);

    void init(const char *device_name,
               const char *service_uuid,
               const char *write_char_uuid,
               const char *notify_char_uuid);

    void update();

    // chamado internamente pelo callback de escrita BLE
    void on_data_received(const uint8_t *data, size_t len);
    void send_line(const char *msg);

    private:
    size_t _buffer_size;
    uint8_t *_line_buf;
    size_t _line_len;

    BLEServer *_server;
    BLECharacteristic *_write_char;
    BLECharacteristic *_notify_char;
    bool _device_connected;

    void process_byte(uint8_t b);
    void handle_line();
    bool parse_stats_line(const char *line);
    void send_response(const char *msg);

    friend class BleReaderWriteCallback;
    friend class BleReaderServerCallback;
};

void task_ble_reader(void *params);
void ble_send_line(const char *msg);
extern TaskHandle_t ble_reader_handle;