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
    VirtButton() : _state(false), _cb(nullptr) {}

    /**
     * @brief Установить текущее сырое состояние виртуальной кнопки.
     */
    void set(bool state) {
        _state = state;
    }

    bool get() const {
        return _state;
    }

#if !defined(__AVR__)
    typedef std::function<bool()> ReadCallback;

    VirtButton(ReadCallback readCb) : _state(false), _cb(nullptr), _stdCb(readCb) {}

    void setReadCallback(ReadCallback readCb) {
        _stdCb = readCb;
        _cb = nullptr;
    }
#else
    typedef bool (*ReadCallback)();

    VirtButton(ReadCallback readCb) : _state(false), _cb(readCb) {}

    void setReadCallback(ReadCallback readCb) {
        _cb = readCb;
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
    ReadCallback _stdCb = nullptr;
#endif
};
