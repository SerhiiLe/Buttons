#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include "ButtonBase.h"

#if !defined(__AVR__)
#include <functional>
#endif

/**
 * @brief Виртуальная кнопка.
 * Наследуется от ButtonBase.
 *
 * Позволяет:
 * 1) Передавать состояние вручную в tick(bool state) в loop();
 * 2) Задавать состояние через btn.set(bool state);
 * 3) Подключать пользовательскую функцию/лямбду опроса (например, touchRead, аналоговый пин и т.д.).
 */
class VirtButton : public ButtonBase {
public:
    VirtButton(bool (*readCb)() = nullptr) : _state(false), _cb(readCb) {}

    /**
     * @brief Установить текущее сырое состояние виртуальной кнопки.
     */
    void set(bool state) {
        _state = state;
    }

    bool get() const {
        return _state;
    }

    /**
     * @brief Задать функцию обратного вызова для чтения сырого состояния.
     */
    void setReadCallback(bool (*readCb)()) {
        _cb = readCb;
    }

#if !defined(__AVR__)
    VirtButton(std::function<bool()> readCb) : _state(false), _cb(nullptr), _stdCb(readCb) {}

    void setReadCallback(std::function<bool()> readCb) {
        _stdCb = readCb;
    }
#endif

    /**
     * @brief Реализация виртуального метода базового класса.
     */
    bool readRaw() override {
#if !defined(__AVR__)
        if (_stdCb) return _stdCb();
#endif
        if (_cb) return _cb();
        return _state;
    }

private:
    bool _state;
    bool (*_cb)();
#if !defined(__AVR__)
    std::function<bool()> _stdCb = nullptr;
#endif
};
