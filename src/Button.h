#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include "ButtonBase.h"

/**
 * @brief Класс кнопки, подключенной к физическому пину микроконтроллера.
 * Наследуется от ButtonBase.
 */
class Button : public ButtonBase {
public:
    /**
     * @param pin Номер пина (255 - не назначен).
     * @param mode Режим пина (INPUT_PULLUP, INPUT, INPUT_PULLDOWN). По умолчанию INPUT_PULLUP.
     * @param btnLevel Активный уровень (сигнал при нажатии): LOW или HIGH. По умолчанию LOW.
     */
    Button(uint8_t pin = 255, uint8_t mode = INPUT_PULLUP, uint8_t btnLevel = LOW) {
        init(pin, mode, btnLevel);
    }

    /**
     * @brief Инициализация пина.
     */
    void init(uint8_t pin = 255, uint8_t mode = INPUT_PULLUP, uint8_t btnLevel = LOW) {
        _pin = pin;
        _btnLevel = btnLevel;
        if (_pin != 255) {
            pinMode(_pin, mode);
        }
    }

    /**
     * @brief Задать активный уровень (LOW/HIGH).
     */
    void setBtnLevel(uint8_t btnLevel) {
        _btnLevel = btnLevel;
    }

    uint8_t getBtnLevel() const {
        return _btnLevel;
    }

    uint8_t getPin() const {
        return _pin;
    }

    /**
     * @brief Прямое чтение состояния пина без антидребезга (true = нажата).
     */
    bool read() const {
        if (_pin == 255) return false;
        return digitalRead(_pin) == _btnLevel;
    }

    /**
     * @brief Реализация виртуального метода базового класса.
     */
    bool readRaw() override {
        return read();
    }

private:
    uint8_t _pin = 255;
    uint8_t _btnLevel = LOW;
};
