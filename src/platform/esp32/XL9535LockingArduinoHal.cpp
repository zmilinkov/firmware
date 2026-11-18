#include "configuration.h"

#if defined(ARCH_ESP32) && defined(USE_XL9535)

#include "XL9535LockingArduinoHal.h"

XL9535LockingArduinoHal::XL9535LockingArduinoHal(SPIClass &spi, SPISettings spiSettings, ExtensionIOXL9535 &expander)
    : LockingArduinoHal(spi, spiSettings), xl(expander)
{
#if defined(LORA_DIO1_SOFTWARE_POLL)
    LOG_INFO("XL9535: LoRa DIO1 IRQ simulated by polling the radio IRQ status register");
#else
    LOG_WARN("XL9535: no LORA_DIO1_SOFTWARE_POLL, LoRa DIO1 interrupts will not work");
#endif
}

bool XL9535LockingArduinoHal::isXlPin(uint32_t pin)
{
    return pin >= XL9535_VPIN_BASE && pin <= XL9535_VPIN_BASE + 15;
}

void XL9535LockingArduinoHal::pinMode(uint32_t pin, uint32_t mode)
{
    if (isXlPin(pin)) {
        xl.pinMode((uint8_t)(pin - XL9535_VPIN_BASE), mode == GpioModeOutput ? OUTPUT : INPUT);
        return;
    }
    ArduinoHal::pinMode(pin, mode);
}

void XL9535LockingArduinoHal::digitalWrite(uint32_t pin, uint32_t value)
{
    if (isXlPin(pin)) {
        xl.digitalWrite((uint8_t)(pin - XL9535_VPIN_BASE), value == GpioLevelHigh ? HIGH : LOW);
        return;
    }
    ArduinoHal::digitalWrite(pin, value);
}

uint32_t XL9535LockingArduinoHal::digitalRead(uint32_t pin)
{
    if (isXlPin(pin))
        return (uint32_t)xl.digitalRead((uint8_t)(pin - XL9535_VPIN_BASE));
    return ArduinoHal::digitalRead(pin);
}

void XL9535LockingArduinoHal::attachInterrupt(uint32_t interruptNum, void (*cb)(void), uint32_t mode)
{
    ArduinoHal::attachInterrupt(interruptNum, cb, mode);
}

void XL9535LockingArduinoHal::detachInterrupt(uint32_t interruptNum)
{
    ArduinoHal::detachInterrupt(interruptNum);
}

uint32_t XL9535LockingArduinoHal::pinToInterrupt(uint32_t pin)
{
    if (isXlPin(pin))
        return RADIOLIB_NC;
    return ArduinoHal::pinToInterrupt(pin);
}

#endif // ARCH_ESP32 && USE_XL9535
