# keyboard-alx-fw

Firmware ESP32 para um teclado/interface com suporte a dois displays OLED e conectividade Bluetooth Low Energy (BLE). O dispositivo exibe animações de "olhos" e monitoramento em tempo real de CPU, memória e disco de um host conectado via BLE.

## Funcionalidades

- **Entrada por botões analógicos**: 4 linhas independentes (pad de controle + 3 linhas de teclado) com debounce, detecção de hold e repeat
- **Dois displays OLED SSD1306** (128x64) em barramentos I2C separados
  - Display esquerdo: animação de olhos com rastreamento de pupila e piscar
  - Display direito: gráficos de monitoramento do sistema (CPU, memória, disco)
- **BLE Server**: recebe estatísticas do sistema e envia eventos de botão
- **Multitarefa FreeRTOS**: execução paralela de display, botões e BLE

## Hardware

| Componente | Detalhe |
|---|---|
| MCU | ESP32 (esp32doit-devkit-v1) |
| Displays | 2x Adafruit SSD1306 OLED 128x64 |
| Comunicação | BLE + UART 115200 |

### Pinagem

| Função | GPIO |
|---|---|
| Display esquerdo SDA/SCL | 21 / 22 |
| Display direito SDA/SCL | 18 / 19 |
| Botão controle (ADC) | 14 |
| Teclado linha 1 (ADC) | 34 |
| Teclado linha 2 (ADC) | 35 |
| Teclado linha 3 (ADC) | 32 |

## Stack

- **Framework**: Arduino via PlatformIO
- **RTOS**: FreeRTOS (nativo ESP32)
- **Bibliotecas**: Adafruit GFX, Adafruit SSD1306, ESP32 BLE

## Estrutura do Projeto

```
src/
├── main.cpp        # Ponto de entrada, setup() e loop()
├── config.h        # Pinos, constantes e thresholds
├── state.h/cpp     # Estado global compartilhado
├── display.h/cpp   # Gerenciamento dos displays OLED
├── buttons.h/cpp   # Leitura e debounce dos botões
├── ble_reader.h/cpp# Servidor BLE e parsing do protocolo
├── monitor.h/cpp   # Gráficos de monitoramento (CPU/MEM/DISK)
└── eyes.h/cpp      # Animação dos olhos
```

## Build e Upload

Requer [PlatformIO](https://platformio.org/).

```bash
# Compilar
pio run

# Compilar e enviar ao ESP32
pio run -t upload

# Monitor serial (115200 baud)
pio run -t monitor
```

## Protocolo BLE

**Dispositivo**: `KBALX`

**Formato de dados recebidos:**
```
STATS,CPU:<valor>,MEM:<valor>,DISK:<valor>
```

Exemplo: `STATS,CPU:12.3,MEM:64.7,DISK:78.1`

| UUID | Tipo |
|---|---|
| `4fafc201-1fb5-459e-8fcc-c5c9c331914b` | Service |
| `beb5483e-36e1-4688-b7f5-ea07361b26a8` | Characteristic (Write) |
| `beb5483e-36e1-4688-b7f5-ea07361b26a9` | Characteristic (Notify) |

## Telas Disponíveis

| ID | Tela |
|---|---|
| 0 | SCR_FLOW_METER |
| 1 | SCR_NUM_BICOS |
| 2 | SCREEN_3 |
| 3 | SCREEN_4 |
| 4 | SCR_BLE_CHK (monitoramento BLE) |

Navegação com botões UP/DOWN; SELECT alterna modos (long press suportado).

## Versão

`0.1.0`
