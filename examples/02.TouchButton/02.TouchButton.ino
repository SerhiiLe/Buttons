/**
 * @file 02.TouchButton.ino
 * @brief Пример работы со встроенным емкостным сенсором ESP32 (TouchButton).
 * Поддерживает калибровку порога, подсчёт быстрых тач-кликов и удержание.
 * На МК без аппаратного модуля тач-сенсора TouchButton::isSupported() вернёт false.
 */

#include <Arduino.h>
#include <Buttons.h>

// Пин с поддержкой аппаратного touch (на ESP32 classic: GPIO 4 = Touch0)
#define PIN_TOUCH 4

// Порог срабатывания:
// На классическом ESP32 показания touchRead() уменьшаются при касании (без касания ~70..80, при касании ~10..30).
// Поэтому порог обычно выбирается в районе 30..45, а activeBelow = true.
#define TOUCH_THRESHOLD 40

TouchButton touchBtn(PIN_TOUCH, TOUCH_THRESHOLD, true);

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println(F("\n=== ESP32 Touch Button Example ==="));

    if (!TouchButton::isSupported()) {
        Serial.println(F("[WARNING] Hardware capacitive touch is not supported on this microcontroller!"));
        Serial.println(F("TouchButton will safely return false for all events."));
    } else {
        Serial.println(F("Hardware touch sensor ready. Touch the wire/pad connected to GPIO!"));
    }
}

void loop() {
    // Опрос состояния тач-кнопки
    touchBtn.tick();

    // 1. Быстрые клики (касания)
    if (touchBtn.hasClicks()) {
        switch (touchBtn.getClicks()) {
            case 1:
                Serial.println(F("Touch: 1 Click"));
                break;
            case 2:
                Serial.println(F("Touch: 2 Clicks (Double Touch)"));
                break;
            default:
                Serial.print(F("Touch: Clicks = "));
                Serial.println(touchBtn.getClicks());
                break;
        }
    }

    // 2. Долгое удержание пальца на сенсоре
    if (touchBtn.hold()) {
        Serial.println(F("Touch: Hold!"));
    }

    // 3. Автоповтор при удержании сенсора
    if (touchBtn.step()) {
        Serial.print(F("Touch: Step #"));
        Serial.println(touchBtn.getSteps());
    }

    // Периодический вывод текущих сырых показаний touchRead() для калибровки порога
    static uint32_t calibTimer = 0;
    if (millis() - calibTimer >= 1000) {
        calibTimer = millis();
        if (TouchButton::isSupported()) {
            Serial.print(F("Raw touch value: "));
            Serial.print(touchBtn.readTouch());
            Serial.print(F(" (Threshold: "));
            Serial.print(touchBtn.getThreshold());
            Serial.println(F(")"));
        }
    }
}

