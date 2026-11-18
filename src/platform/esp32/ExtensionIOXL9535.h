#pragma once

#include "concurrency/Lock.h"
#include <Arduino.h>
#include <Wire.h>

/**
 * XL9535 (PCA9555-compatible) 16-bit I2C GPIO expander (pins 0-7 = P0.0-P0.7, 8-15 = P1.0-P1.7).
 *
 * Used by boards that route radio control lines through the expander (e.g. LilyGo T-Display-P4).
 * RadioLib access goes through XL9535LockingArduinoHal, which maps virtual pins
 * XL9535_VPIN_BASE..+15 onto local pins 0-15.
 */
class ExtensionIOXL9535
{
  public:
    void begin(TwoWire &wire, uint8_t addr, int sda, int scl);
    bool isPresent() const { return _begun && _present; }

    /** Local pin index 0-15, not the virtual RadioLib pin. */
    void pinMode(uint8_t pin, uint8_t mode);
    void digitalWrite(uint8_t pin, uint8_t value);
    int digitalRead(uint8_t pin);

    /** Raw register read (0x00-0x07), for bring-up / debug. */
    uint8_t readRegister(uint8_t reg);

  private:
    bool readReg(uint8_t reg, uint8_t &val);
    bool writeReg(uint8_t reg, uint8_t val);

    TwoWire *_wire = nullptr;
    uint8_t _addr = 0;
    bool _begun = false;
    bool _present = false;
    // The radio HAL (BUSY/RESET) and board code reach this expander from different threads, and the
    // read-modify-write paths are not atomic.
    concurrency::Lock _lock;
};

/** Global instance shared by the early-init hook and the RadioLib HAL. */
extern ExtensionIOXL9535 xl9535IoExpander;

/**
 * Bring up the expander and put the board into a state where the radio is powered, out of reset and
 * wired to the internal antenna. Must run before the I2C scan and radio init.
 */
void xl9535EarlyInit();
