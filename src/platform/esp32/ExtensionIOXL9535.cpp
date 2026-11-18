#include "configuration.h"

#if defined(ARCH_ESP32) && defined(USE_XL9535)

#include "ExtensionIOXL9535.h"
#include "concurrency/LockGuard.h"

// PCA9555 register map: input 0x00/01, output 0x02/03, polarity 0x04/05, direction 0x06/07 (1 = input)
static constexpr uint8_t REG_INPUT = 0x00;
static constexpr uint8_t REG_OUTPUT = 0x02;
static constexpr uint8_t REG_CONFIG = 0x06;

ExtensionIOXL9535 xl9535IoExpander;

void ExtensionIOXL9535::begin(TwoWire &wire, uint8_t addr, int sda, int scl)
{
    if (_begun)
        return;
    _wire = &wire;
    _addr = addr;
    wire.begin(sda, scl);
    _begun = true;
    wire.beginTransmission(addr);
    _present = wire.endTransmission() == 0;
}

bool ExtensionIOXL9535::readReg(uint8_t reg, uint8_t &val)
{
    if (!_wire || !_begun)
        return false;
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0)
        return false;
    if (_wire->requestFrom((int)_addr, 1) != 1 || !_wire->available())
        return false;
    val = (uint8_t)_wire->read();
    return true;
}

bool ExtensionIOXL9535::writeReg(uint8_t reg, uint8_t val)
{
    if (!_wire || !_begun)
        return false;
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->write(val);
    return _wire->endTransmission() == 0;
}

void ExtensionIOXL9535::pinMode(uint8_t pin, uint8_t mode)
{
    if (pin > 15)
        return;
    concurrency::LockGuard guard(&_lock);
    uint8_t reg = REG_CONFIG + (pin >> 3);
    uint8_t v;
    if (!readReg(reg, v)) // skip the write rather than clobber the other 7 pins on a failed read
        return;
    uint8_t mask = 1u << (pin & 7);
    if (mode == OUTPUT)
        v &= ~mask;
    else
        v |= mask; // the XL9535 has no pull-ups, INPUT_PULLUP behaves as INPUT
    writeReg(reg, v);
}

void ExtensionIOXL9535::digitalWrite(uint8_t pin, uint8_t value)
{
    if (pin > 15)
        return;
    concurrency::LockGuard guard(&_lock);
    uint8_t reg = REG_OUTPUT + (pin >> 3);
    uint8_t v;
    if (!readReg(reg, v)) // skip rather than clobber the bank (e.g. drop the radio reset) on a failed read
        return;
    uint8_t mask = 1u << (pin & 7);
    if (value == LOW)
        v &= ~mask;
    else
        v |= mask;
    writeReg(reg, v);
}

int ExtensionIOXL9535::digitalRead(uint8_t pin)
{
    if (pin > 15)
        return LOW;
    concurrency::LockGuard guard(&_lock);
    uint8_t v;
    if (!readReg(REG_INPUT + (pin >> 3), v))
        return HIGH; // fail-safe: a failed read must not look like "BUSY released" and let RadioLib start an SPI
                     // transaction too early. DIO1 is polled via the radio IRQ register, not this pin.
    return (v & (1u << (pin & 7))) ? HIGH : LOW;
}

uint8_t ExtensionIOXL9535::readRegister(uint8_t reg)
{
    concurrency::LockGuard guard(&_lock);
    uint8_t v = 0;
    readReg(reg, v);
    return v;
}

void xl9535EarlyInit()
{
    xl9535IoExpander.begin(Wire, XL9535_ADDR, I2C_SDA, I2C_SCL);
    if (!xl9535IoExpander.isPresent()) {
        LOG_ERROR("XL9535 not found at 0x%02x", XL9535_ADDR);
        return;
    }

    // Levels first, then directions, so a pin never glitches to the wrong level when it becomes an output
    xl9535IoExpander.digitalWrite(EXIO_POWER_EN_3V3, HIGH);
    xl9535IoExpander.pinMode(EXIO_POWER_EN_3V3, OUTPUT);
    // The 3.3 V rail has to settle before the radio is released from reset
    delay(10);

    xl9535IoExpander.digitalWrite(EXIO_RF_SWITCH_VCTL, EXIO_RF_SWITCH_INTERNAL_ANTENNA);
    xl9535IoExpander.pinMode(EXIO_RF_SWITCH_VCTL, OUTPUT);

    xl9535IoExpander.digitalWrite(EXIO_LORA_NRST, HIGH);
    xl9535IoExpander.pinMode(EXIO_LORA_NRST, OUTPUT);
    xl9535IoExpander.pinMode(EXIO_LORA_DIO1, INPUT);

    LOG_INFO("XL9535 0x%02x: CFG0=0x%02x CFG1=0x%02x OUT0=0x%02x OUT1=0x%02x", XL9535_ADDR, xl9535IoExpander.readRegister(0x06),
             xl9535IoExpander.readRegister(0x07), xl9535IoExpander.readRegister(0x02), xl9535IoExpander.readRegister(0x03));
}

#endif // ARCH_ESP32 && USE_XL9535
