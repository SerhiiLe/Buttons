#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include "ButtonBase.h"

// Определение наличия аппаратного емкостного сенсора (ESP32, ESP32-S2, ESP32-S3)
#if (defined(ESP32) || defined(ARDUINO_ARCH_ESP32)) && defined(SOC_TOUCH_SENSOR_NUM) && (SOC_TOUCH_SENSOR_NUM > 0)
    #define _BUTTONS_HAS_TOUCH 1
#elif (defined(ESP32) || defined(ARDUINO_ARCH_ESP32)) && defined(TOUCH_PAD_MAX) && (TOUCH_PAD_MAX > 0)
    #define _BUTTONS_HAS_TOUCH 1
#else
    #define _BUTTONS_HAS_TOUCH 0
#endif

/**
 * @brief Сенсорная (емкостная) кнопка для ESP32 (touchRead).
 * Наследуется от ButtonBase.
 *
 * Работает как на старых версиях ядра ESP32 (Arduino core 1.x / 2.x),
 * так и на новых (Arduino core 3.x / PlatformIO espressif32 8.x).
 * На платформах без аппаратного тач-сенсора (AVR, STM32, RP2040, ESP8266, ESP32-C3)
 * собирается без ошибок, но метод readRaw() возвращает false.
 */
class TouchButton : public ButtonBase {
public:
    /**
     * @param pin Номер пина с поддержкой touch (например, GPIO 4, 15, 13, 12, 14, 27, 33, 32 на ESP32).
     * @param threshold Порог срабатывания сенсора.
     * @param activeBelow true, если касание УМЕНЬШАЕТ показания (стандартный ESP32 classic: без касания ~70-80, при касании <30).
     *                    false, если касание УВЕЛИЧИВАЕТ показания (ESP32-S2/S3).
     */
    TouchButton(uint8_t pin = 255, uint16_t threshold = 40, bool activeBelow = true) {
        init(pin, threshold, activeBelow);
    }

    void init(uint8_t pin, uint16_t threshold = 40, bool activeBelow = true) {
        _pin = pin;
        _threshold = threshold;
        _activeBelow = activeBelow;
    }

    void setThreshold(uint16_t threshold) { _threshold = threshold; }
    uint16_t getThreshold() const { return _threshold; }

    void setActiveBelow(bool activeBelow) { _activeBelow = activeBelow; }
    bool getActiveBelow() const { return _activeBelow; }

    uint8_t getPin() const { return _pin; }

    /**
     * @brief Проверка, поддерживается ли аппаратный тач на текущем МК.
     */
    static constexpr bool isSupported() {
        return _BUTTONS_HAS_TOUCH == 1;
    }

    /**
     * @brief Считать текущее аналоговое значение touchRead() (удобно для калибровки порога в Serial).
     */
    uint32_t readTouch() const {
#if _BUTTONS_HAS_TOUCH
        if (_pin != 255) {
            return (uint32_t)touchRead(_pin);
        }
#endif
        return 0;
    }

    /**
     * @brief Реализация виртуального метода базового класса.
     */
    bool readRaw() override {
#if _BUTTONS_HAS_TOUCH
        if (_pin == 255) return false;
        uint32_t val = (uint32_t)touchRead(_pin);
        return _activeBelow ? (val < _threshold) : (val > _threshold);
#else
        return false;
#endif
    }

private:
    uint8_t _pin = 255;
    uint16_t _threshold = 40;
    bool _activeBelow = true;
};
