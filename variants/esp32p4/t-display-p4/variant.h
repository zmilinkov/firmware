#pragma once

// LilyGo T-Display-P4 V1.0: ESP32-P4 (16 MB flash, PSRAM) + ESP32-C6 (ESP-Hosted over SDIO) + SX1262.
// The SX1262 RESET and DIO1 are wired to an XL9535 I2C expander (0x20), so DIO1 uses a software poll.
// Pin map: Xinyuan-LilyGO/lilygo_device_driver, src/device/t_display_p4/v1/config.h

// ─── I2C bus ──────────────────────────────────────────────────────────────────
// Bus 1: XL9535, PCF8563 RTC, BQ27220 fuel gauge, touch. Bus 2 (SDA 20 / SCL 21): ES8311, ICM20948, AW86224.
#define I2C_SDA 7
#define I2C_SCL 8

// ─── I/O expander ─────────────────────────────────────────────────────────────
// XL9535 pins are named IOn for P0.n and IO1n for P1.n; the local index is 0-7 for P0.x and 8-15 for P1.x.
#define USE_XL9535
#define XL9535_ADDR 0x20
#define XL9535_VPIN_BASE 100 // RadioLib virtual pins 100..115 = expander P0.0..P1.7
#define EXIO_POWER_EN_3V3 0  // IO0: 3.3 V peripheral rail
#define EXIO_RF_SWITCH_VCTL 1 // IO1: SKY13453 antenna path
#define EXIO_USB_PHY_POWER_EN 8 // IO10: USB PHY supply (named ESP32P4_VCCA_POWER_EN in Meck-P4)
#define EXIO_GPS_WAKE 9         // IO11: L76K WAKE_UP, driven high
#define EXIO_LORA_NRST 14     // IO16: SX1262 reset
#define EXIO_LORA_DIO1 15     // IO17: SX1262 DIO1

// SKY13453 VCTL: 1 = internal antenna, 0 = external antenna on MMCX1. Never select the external path
// without an antenna connected to MMCX1.
#ifndef EXIO_RF_SWITCH_INTERNAL_ANTENNA
#define EXIO_RF_SWITCH_INTERNAL_ANTENNA HIGH
#endif

// ─── LoRa radio ───────────────────────────────────────────────────────────────
#define USE_SX1262
#define LORA_SCK 2
#define LORA_MOSI 3
#define LORA_MISO 4
#define LORA_CS 24

#define LORA_DIO1 (XL9535_VPIN_BASE + EXIO_LORA_DIO1)  // 115
#define LORA_RESET (XL9535_VPIN_BASE + EXIO_LORA_NRST) // 114
#define LORA_BUSY 6

#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_BUSY
#define SX126X_RESET LORA_RESET

// DIO1 is an expander pin: no hardware interrupt, the SX126x IRQ status register is polled instead
#define LORA_DIO1_SOFTWARE_POLL 1
#define LORA_DIO1_EXTENDED_IO

// The SKY13453 is steered by VCTL only, so DIO2 and TXEN/RXEN are unused. LilyGo's reference code runs
// the SX1262 with the RadioLib default 1.6 V DIO3 TCXO supply.
#define SX126X_TXEN RADIOLIB_NC
#define SX126X_RXEN RADIOLIB_NC
#define SX126X_DIO3_TCXO_VOLTAGE 1.6
#define SX126X_MAX_POWER 22

// ─── GPS ──────────────────────────────────────────────────────────────────────
// Quectel L76K on UART. LilyGo's pin names (TX 22 / RX 23) are the module's, so the module's TX is the MCU's RX.
#define GPS_RX_PIN 22
#define GPS_TX_PIN 23

// ─── Buttons ──────────────────────────────────────────────────────────────────
#define BUTTON_PIN 35 // ESP32-P4 BOOT
