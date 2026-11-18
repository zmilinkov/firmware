#pragma once

#include "configuration.h"

#if defined(ARCH_ESP32) && defined(USE_XL9535)

#include "mesh/RadioLibInterface.h"
#include "platform/esp32/ExtensionIOXL9535.h"
#include <SPI.h>

#ifndef XL9535_VPIN_BASE
#define XL9535_VPIN_BASE 100
#endif

/**
 * Routes RadioLib virtual GPIO XL9535_VPIN_BASE..+15 to an XL9535 I2C expander; all other pins fall
 * through to the regular Arduino GPIO HAL.
 *
 * The expander /INT is not usable as a DIO1 interrupt, so boards define LORA_DIO1_SOFTWARE_POLL and the
 * radio thread polls the radio's IRQ status register instead.
 */
class XL9535LockingArduinoHal : public LockingArduinoHal
{
  public:
    XL9535LockingArduinoHal(SPIClass &spi, SPISettings spiSettings, ExtensionIOXL9535 &expander);

    void pinMode(uint32_t pin, uint32_t mode) override;
    void digitalWrite(uint32_t pin, uint32_t value) override;
    uint32_t digitalRead(uint32_t pin) override;
    void attachInterrupt(uint32_t interruptNum, void (*cb)(void), uint32_t mode) override;
    void detachInterrupt(uint32_t interruptNum) override;
    uint32_t pinToInterrupt(uint32_t pin) override;

  private:
    ExtensionIOXL9535 &xl;
    static bool isXlPin(uint32_t pin);
};

#endif // ARCH_ESP32 && USE_XL9535
