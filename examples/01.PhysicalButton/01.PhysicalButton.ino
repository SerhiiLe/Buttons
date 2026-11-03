/**
 * @file 01.PhysicalButton.ino
 * @brief Пример работы с физической тактовой кнопкой (GPIO).
 * Демонстрация антидребезга, подсчёта серии быстрых кликов, удержания и автоповтора (step).
 */

#include <Arduino.h>
#include <Buttons.h>

// Пин кнопки. Например, GPIO 0 (кнопка BOOT на платах ESP32/ESP8266)
#define PIN_BUTTON 0

// Инициализация кнопки: пин, режим подтяжки (INPUT_PULLUP), активный уровень при нажатии (LOW)
Button btn(PIN_BUTTON, INPUT_PULLUP, LOW);

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println(F("\n=== Physical Button Example ==="));
    Serial.println(F("Press, multi-click, or hold the button!"));

    // Опционально: настройка таймаутов (мс)
    // btn.setDebTimeout(50);   // Антидребезг (по умолчанию 50 мс)
    // btn.setClickTimeout(400); // Таймаут серии кликов (по умолчанию 400 мс)
    // btn.setHoldTimeout(600);  // Порог удержания (по умолчанию 600 мс)
    // btn.setStepTimeout(200);  // Интервал автоповтора при удержании (по умолчанию 200 мс)
}

void loop() {
    // Регулярный опрос в loop()
    btn.tick();

    // 1. Одиночное нажатие (импульс в момент замыкания после антидребезга)
    if (btn.press()) {
        Serial.println(F("Event: Button pressed"));
    }

    // 2. Отпускание кнопки (импульс в момент размыкания)
    if (btn.release()) {
        Serial.println(F("Event: Button released"));
    }

    // 3. Подсчёт серии быстрых кликов (1, 2, 3...)
    if (btn.hasClicks()) {
        switch (btn.getClicks()) {
            case 1:
                Serial.println(F("Clicks: 1 Click (Single)"));
                break;
            case 2:
                Serial.println(F("Clicks: 2 Clicks (Double)"));
                break;
            case 3:
                Serial.println(F("Clicks: 3 Clicks (Triple)"));
                break;
            default:
                Serial.print(F("Clicks: Multiple clicks count = "));
                Serial.println(btn.getClicks());
                break;
        }
    }

    // 4. Долгое удержание (срабатывает 1 раз при превышении holdTimeout)
    if (btn.hold()) {
        Serial.println(F("Event: Button Hold!"));
    }

    // 5. Автоповтор при удержании (периодические импульсы с шагом stepTimeout)
    if (btn.step()) {
        Serial.print(F("Event: Step #"));
        Serial.println(btn.getSteps());
    }

    // 6. Отпускание после серии шагов автоповтора
    if (btn.releaseStep()) {
        Serial.println(F("Event: Released after step"));
    }
}

