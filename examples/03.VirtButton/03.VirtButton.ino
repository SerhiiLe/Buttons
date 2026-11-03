/**
 * @file 03.VirtButton.ino
 * @brief Пример работы с виртуальной кнопкой (VirtButton).
 * Позволяет обрабатывать программные события, внешние датчики или данные из сети:
 * 1) Передачей логического состояния напрямую в tick(bool state);
 * 2) Через подключение функции или лямбды обратного вызова (readCallback).
 */

#include <Arduino.h>
#include <Buttons.h>

// Способ 1: Виртуальная кнопка с ручной передачей состояния в tick(bool)
VirtButton vBtnManual;

// Способ 2: Виртуальная кнопка с лямбда-функцией опроса (например, эмуляция датчика)
bool simulatedSensor = false;
VirtButton vBtnLambda([]() -> bool {
    return simulatedSensor;
});

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println(F("\n=== Virtual Button Example ==="));
}

void loop() {
    // -------------------------------------------------------------------------
    // Вариант 1: Прямая передача логического значения (как в проекте Watering-esp32)
    // Например, опрос внешнего флага, регистра или компаратора:
    // bool isTouchActive = (touchRead(T0) < 40);
    // vBtnManual.tick(isTouchActive);
    // -------------------------------------------------------------------------
    static uint32_t manualTimer = 0;
    static bool manualState = false;

    // Имитируем нажатия виртуальной кнопки каждые несколько секунд для демонстрации:
    if (millis() - manualTimer >= 2000) {
        manualTimer = millis();
        manualState = !manualState;
    }

    vBtnManual.tick(manualState);

    if (vBtnManual.press()) {
        Serial.println(F("vBtnManual: Pressed"));
    }
    if (vBtnManual.hasClicks()) {
        Serial.print(F("vBtnManual: Finished clicks = "));
        Serial.println(vBtnManual.getClicks());
    }
    if (vBtnManual.hold()) {
        Serial.println(F("vBtnManual: Held!"));
    }

    // -------------------------------------------------------------------------
    // Вариант 2: Виртуальная кнопка с лямбдой (vBtnLambda)
    // Опрашивается обычным вызовом vBtnLambda.tick()
    // -------------------------------------------------------------------------
    vBtnLambda.tick();

    if (vBtnLambda.hasClicks()) {
        Serial.print(F("vBtnLambda: Clicks = "));
        Serial.println(vBtnLambda.getClicks());
    }
}

