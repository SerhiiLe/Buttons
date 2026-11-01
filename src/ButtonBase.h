#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <cstdint>
#include <cstddef>
extern uint32_t millis();
#endif

#if !defined(__AVR__)
#include <functional>
#endif

/**
 * @brief Абстрактный базовый класс кнопки / сенсора / источника дискретных событий.
 * Содержит полную логику антидребезга, подсчёта быстрых кликов, удержания (hold),
 * автоповтора (step) и управления состояниями.
 *
 * Для создания своего источника (тач-сенсор, АЦП, матричная клавиатура, I2C экспандер и т.д.)
 * достаточно унаследовать этот класс и переопределить метод readRaw().
 */
class ButtonBase {
public:
    virtual ~ButtonBase() = default;

    /**
     * @brief Чисто виртуальный метод чтения сырого состояния источника.
     * @return true - активное (нажатое) состояние, false - неактивное (отпущенное).
     */
    virtual bool readRaw() = 0;

    // =========================================================================
    // Опрос состояния (tick)
    // =========================================================================

    /**
     * @brief Основной метод опроса. Вызывать регулярно в loop().
     * Опрашивает источник через readRaw() и обновляет конечный автомат.
     * @return true, если на текущем тике произошло какое-либо событие.
     */
    bool tick() {
        return tick(readRaw());
    }

    /**
     * @brief Опрос с передачей внешнего сырого состояния.
     * Позволяет использовать виртуальные кнопки и передавать состояние вручную.
     * @param raw Текущее сырое состояние (true = нажато).
     * @return true, если на текущем тике произошло какое-либо событие.
     */
    bool tick(bool raw) {
        clear();
        poll(raw);
        if (hasEvent()) {
            callHandler();
        }
        return hasEvent();
    }

    /**
     * @brief Опрос без сброса флагов событий предыдущего тика.
     */
    bool tickRaw() {
        return tickRaw(readRaw());
    }

    bool tickRaw(bool raw) {
        poll(raw);
        return hasEvent();
    }

    // =========================================================================
    // Настройка таймаутов (мс)
    // =========================================================================

    /**
     * @brief Установить время антидребезга (по умолчанию 50 мс).
     */
    void setDebTimeout(uint16_t tout) { _debTimeout = tout; }
    void setDebounce(uint16_t tout) { _debTimeout = tout; }

    /**
     * @brief Установить таймаут между кликами для подсчёта серии (по умолчанию 400 мс).
     */
    void setClickTimeout(uint16_t tout) { _clickTimeout = tout; }
    void setTimeout(uint16_t tout) { _clickTimeout = tout; }

    /**
     * @brief Установить таймаут удержания кнопки (по умолчанию 600 мс).
     */
    void setHoldTimeout(uint16_t tout) { _holdTimeout = tout; }

    /**
     * @brief Установить интервал генерации автоповтора (step) при удержании (по умолчанию 200 мс).
     */
    void setStepTimeout(uint16_t tout) { _stepTimeout = tout; }

    uint16_t getDebTimeout() const { return _debTimeout; }
    uint16_t getClickTimeout() const { return _clickTimeout; }
    uint16_t getHoldTimeout() const { return _holdTimeout; }
    uint16_t getStepTimeout() const { return _stepTimeout; }

    // =========================================================================
    // События (Pulse events - возвращают true ровно на один тик)
    // =========================================================================

    /**
     * @brief Кнопка только что нажата (после прохождения антидребезга).
     */
    bool press() const { return _fPress; }
    bool isPress() const { return _fPress; }

    /**
     * @brief Кнопка только что отпущена (после прохождения антидребезга).
     */
    bool release() const { return _fRelease; }
    bool isRelease() const { return _fRelease; }

    /**
     * @brief Одиночный клик: кнопка отпущена без перехода в режим удержания.
     * Срабатывает сразу при отпускании (не дожидаясь окончания серии кликов).
     */
    bool click() const { return _fClick; }

    /**
     * @brief Зафиксировано долгое удержание (срабатывает 1 раз по истечении holdTimeout).
     */
    bool hold() const { return _fHold; }
    bool hold(uint8_t num) const { return _fHold && (_clicksHold == num); }
    bool isHolded() const { return _fHold; }
    bool isHeld() const { return _fHold; }

    /**
     * @brief Автоповтор (импульсное удержание): срабатывает периодически с шагом stepTimeout,
     * пока кнопка удерживается после holdTimeout.
     */
    bool step() const { return _fStep; }
    bool step(uint8_t num) const { return _fStep && (_clicksHold == num); }

    /**
     * @brief Получить количество сгенерированных шагов step в текущей сессии удержания.
     */
    uint16_t getSteps() const { return _stepCount; }

    /**
     * @brief Зафиксирована серия кликов (таймаут ожидания следующего клика истёк).
     */
    bool hasClicks() const { return _fClicks; }
    bool hasClicks(uint8_t num) const { return _fClicks && (_clicks == num); }

    /**
     * @brief Получить количество зафиксированных кликов (1, 2, 3...).
     * Возвращает число кликов на тике, когда hasClicks() == true, иначе 0.
     */
    uint8_t getClicks() const { return _clicks; }
    uint8_t clicks() const { return _clicks; }

    /**
     * @brief Кнопка отпущена после удержания (без step).
     */
    bool releaseHold() const { return _fReleaseHold; }

    /**
     * @brief Кнопка отпущена после шагов автоповтора (после step).
     */
    bool releaseStep() const { return _fReleaseStep; }

    /**
     * @brief Случилось ли хоть одно импульсное событие на текущем тике.
     */
    bool hasEvent() const {
        return _fPress || _fRelease || _fClick || _fHold || _fStep || _fClicks;
    }
    bool action() const { return hasEvent(); }

    // =========================================================================
    // Состояния (State getters - true всё время, пока условие истинно)
    // =========================================================================

    /**
     * @brief Текущее стабильное состояние кнопки (true = зажата).
     */
    bool state() const { return _debState; }
    bool pressing() const { return _debState; }
    bool isPressed() const { return _debState; }

    /**
     * @brief Кнопка сейчас находится в состоянии удержания (зажата дольше holdTimeout).
     */
    bool holding() const { return _debState && _held; }
    bool isHold() const { return holding(); }

    /**
     * @brief Кнопка занята: либо нажата, либо ждёт возможного следующего клика в серии.
     */
    bool busy() const { return _debState || (_clickCount > 0); }

    // =========================================================================
    // Управление и обработка прерываний
    // =========================================================================

    /**
     * @brief Вызов из аппаратного прерывания (ISR) для быстрой фиксации нажатия.
     */
    void pressISR() {
        _isrFlag = true;
    }

    /**
     * @brief Сбросить флаги событий текущего тика.
     */
    void clear() {
        _fPress = false;
        _fRelease = false;
        _fClick = false;
        _fHold = false;
        _fStep = false;
        _fClicks = false;
        _fReleaseHold = false;
        _fReleaseStep = false;
        _clicks = 0;
    }

    /**
     * @brief Полный сброс всего конечного автомата кнопки.
     */
    void reset() {
        clear();
        _debState = false;
        _lastRaw = false;
        _held = false;
        _stepped = false;
        _skip = false;
        _isrFlag = false;
        _clickCount = 0;
        _clicksHold = 0;
        _stepCount = 0;
        _debTimer = 0;
        _pressTimer = 0;
        _stepTimer = 0;
        _clickTimer = 0;
    }

    /**
     * @brief Игнорировать все последующие события до полного отпускания кнопки.
     */
    void skipEvents() {
        _skip = true;
        _fHold = false;
        _fStep = false;
        _clickCount = 0;
        _clicksHold = 0;
    }

    // =========================================================================
    // Коллбэки
    // =========================================================================
    typedef void (*ActionHandler)();

    void attach(ActionHandler handler) {
        _cb = handler;
    }

    void detach() {
        _cb = nullptr;
#if !defined(__AVR__)
        _stdCb = nullptr;
#endif
    }

#if !defined(__AVR__)
    typedef std::function<void()> StdActionHandler;
    void attach(StdActionHandler handler) {
        _stdCb = handler;
    }
#endif

protected:
    void poll(bool raw) {
        uint32_t now = millis();

        // Учёт флага из ISR
        if (_isrFlag) {
            _isrFlag = false;
            raw = true;
        }

        // Фильтр дребезга контактов
        if (raw != _lastRaw) {
            _lastRaw = raw;
            _debTimer = now;
        }

        if (_debState != _lastRaw) {
            if ((uint32_t)(now - _debTimer) >= _debTimeout) {
                _debState = _lastRaw;
                if (_debState) {
                    _onPress(now);
                } else {
                    _onRelease(now);
                }
            }
        }

        // Временная обработка удержания и автоповтора (когда кнопка стабильно нажата)
        if (_debState) {
            if (!_skip) {
                if (!_held) {
                    if ((uint32_t)(now - _pressTimer) >= _holdTimeout) {
                        _held = true;
                        _fHold = true;
                        _stepTimer = now;
                        _stepCount = 0;
                    }
                } else {
                    if ((uint32_t)(now - _stepTimer) >= _stepTimeout) {
                        _stepTimer = now;
                        _fStep = true;
                        _stepped = true;
                        _stepCount++;
                    }
                }
            }
        } else {
            // Временная обработка таймаута между кликами (когда кнопка отпущена)
            if (_clickCount > 0) {
                if ((uint32_t)(now - _clickTimer) >= _clickTimeout) {
                    _fClicks = true;
                    _clicks = _clickCount;
                    _clickCount = 0;
                    _clicksHold = 0;
                }
            }
        }
    }

    void _onPress(uint32_t now) {
        _fPress = true;
        _pressTimer = now;
        _stepTimer = now;
        _held = false;
        _stepped = false;
        _skip = false;
        _clicksHold = _clickCount;
    }

    void _onRelease(uint32_t now) {
        _fRelease = true;

        if (_skip) {
            _skip = false;
            _held = false;
            _stepped = false;
            _clickCount = 0;
            _clicksHold = 0;
            return;
        }

        if (_stepped) {
            _fReleaseStep = true;
            _clickCount = 0;
            _clicksHold = 0;
            _held = false;
            _stepped = false;
        } else if (_held) {
            _fReleaseHold = true;
            _clickCount = 0;
            _clicksHold = 0;
            _held = false;
        } else {
            _fClick = true;
            _clickCount++;
            _clickTimer = now;
        }
    }

    void callHandler() {
        if (_cb) _cb();
#if !defined(__AVR__)
        else if (_stdCb) _stdCb();
#endif
    }

    // Таймауты
    uint16_t _debTimeout = 50;
    uint16_t _clickTimeout = 400;
    uint16_t _holdTimeout = 600;
    uint16_t _stepTimeout = 200;

    // Таймеры
    uint32_t _debTimer = 0;
    uint32_t _pressTimer = 0;
    uint32_t _stepTimer = 0;
    uint32_t _clickTimer = 0;

    // Счётчики
    uint8_t _clickCount = 0;
    uint8_t _clicksHold = 0;
    uint8_t _clicks = 0;
    uint16_t _stepCount = 0;

    // Состояния
    bool _debState = false;
    bool _lastRaw = false;
    bool _held = false;
    bool _stepped = false;
    bool _skip = false;
    volatile bool _isrFlag = false;

    // Импульсные флаги событий текущего тика
    bool _fPress = false;
    bool _fRelease = false;
    bool _fClick = false;
    bool _fHold = false;
    bool _fStep = false;
    bool _fClicks = false;
    bool _fReleaseHold = false;
    bool _fReleaseStep = false;

    ActionHandler _cb = nullptr;
#if !defined(__AVR__)
    StdActionHandler _stdCb = nullptr;
#endif
};
