#include "ble_reader.h"
#include "state.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include "driver/uart.h"

TaskHandle_t ble_reader_handle = NULL;

static BleReader *g_reader_instance = nullptr;

// ---- Callback de conexão/desconexão ----
class BleReaderServerCallback : public BLEServerCallbacks {
public:
    BleReader *reader;
    BleReaderServerCallback(BleReader *r) : reader(r) {}

    void onConnect(BLEServer *server) override {
        reader->_device_connected = true;
    }

    void onDisconnect(BLEServer *server) override {
        reader->_device_connected = false;
        // volta a anunciar pra aceitar nova conexão
        server->getAdvertising()->start();
    }
};

// ---- Callback de escrita na characteristic ----
class BleReaderWriteCallback : public BLECharacteristicCallbacks {
public:
    BleReader *reader;
    BleReaderWriteCallback(BleReader *r) : reader(r) {}

    void onWrite(BLECharacteristic *characteristic) override {
        std::string value = characteristic->getValue();
        if (value.length() > 0) {
            reader->on_data_received((const uint8_t *)value.data(), value.length());
        }
    }
};

BleReader::BleReader(size_t buffer_size)
    : _buffer_size(buffer_size), _line_len(0), _server(nullptr),
      _write_char(nullptr), _notify_char(nullptr), _device_connected(false)
{
    _line_buf = new uint8_t[_buffer_size];
    g_reader_instance = this;
}

void BleReader::init(const char *device_name,
                       const char *service_uuid,
                       const char *write_char_uuid,
                       const char *notify_char_uuid)
{
    _line_len = 0;
    memset(_line_buf, 0, _buffer_size);

    BLEDevice::init(device_name);
    BLEDevice::setMTU(185);

    _server = BLEDevice::createServer();
    _server->setCallbacks(new BleReaderServerCallback(this));

    BLEService *service = _server->createService(service_uuid);

    _write_char = service->createCharacteristic(
        write_char_uuid,
        BLECharacteristic::PROPERTY_WRITE
    );
    _write_char->setCallbacks(new BleReaderWriteCallback(this));

    _notify_char = service->createCharacteristic(
        notify_char_uuid,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    _notify_char->addDescriptor(new BLE2902());

    service->start();

    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->setMinPreferred(0x06);  // 6 * 1.25ms = 7.5ms
    advertising->setMaxPreferred(0x0C);  // 12 * 1.25ms = 15ms
    advertising->addServiceUUID(service_uuid);
    advertising->setScanResponse(true);
    BLEDevice::startAdvertising();
}

void BleReader::on_data_received(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        process_byte(data[i]);
    }
}

void BleReader::process_byte(uint8_t b)
{
    if (b == '\n' || b == '\r') {
        if (_line_len > 0) {
            handle_line();
            _line_len = 0;
        }
        return;
    }

    if (_line_len < _buffer_size - 1) {
        _line_buf[_line_len++] = b;
    } else {
        _line_len = 0; // linha grande demais, descarta
    }
}

// Espera algo como: STATS,CPU:12.3,MEM:64.7,DISK:78.1
bool BleReader::parse_stats_line(const char *line)
{
    if (strncmp(line, "STATS,", 6) != 0) {
        return false;
    }

    float cpu, mem, disk;
    int matched = sscanf(line, "STATS,CPU:%f,MEM:%f,DISK:%f", &cpu, &mem, &disk);

    if (matched != 3) {
        return false;
    }

    stats_cpu_pct = cpu;
    stats_mem_pct = mem;
    stats_disk_pct = disk;
    stats_last_update_ms = millis();

    return true;
}

void BleReader::send_line(const char *msg)
{
    if (_notify_char != nullptr && _device_connected) {
        _notify_char->setValue((uint8_t *)msg, strlen(msg));
        _notify_char->notify();
    }
}

void ble_send_line(const char *msg)
{
    if (g_reader_instance != nullptr) {
        g_reader_instance->send_line(msg);
    }
}

void BleReader::handle_line()
{
    _line_buf[_line_len] = '\0';
    const char *line = (const char *)_line_buf;

    // eco para fins de teste
    char out[160];
    snprintf(out, sizeof(out), "RECEBIDO: %s", line);
    send_line(out);

    // tenta interpretar como pacote de stats
    if (parse_stats_line(line)) {
        char confirm[96];
        snprintf(confirm, sizeof(confirm),
                 "STATS OK -> CPU:%.1f MEM:%.1f DISK:%.1f",
                 stats_cpu_pct, stats_mem_pct, stats_disk_pct);
        send_line(confirm);
    }
}

void BleReader::update()
{
    // BLE é orientado a evento (onWrite), então não há polling de bytes aqui.
}

// ---- task opcional ----
static BleReader reader(128);

void task_ble_reader(void *params)
{
    reader.init(
        "KBALX",
        "4fafc201-1fb5-459e-8fcc-c5c9c331914b",
        "beb5483e-36e1-4688-b7f5-ea07361b26a8",
        "beb5483e-36e1-4688-b7f5-ea07361b26a9"
    );

    while (true) {
        reader.update();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}