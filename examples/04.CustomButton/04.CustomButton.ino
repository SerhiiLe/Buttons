/**
 * @file 04.CustomButton.ino
 * @brief Пример создания собственного класса кнопки через наследование от ButtonBase.
 * Демонстрирует подключение нестандартного источника (аналоговая кнопка на АЦП, 
 * матричная клавиатура, I2C экспандер, компаратор и т.д.).
 */

#include <Arduino.h>
#include <Buttons.h>

/**
 * @class AnalogRangeButton
 * @brief Кнопка, считывающая напряжение с аналогового входа (АЦП).
 * Срабатывает, когда значение analogRead() попадает в заданный диапазон [minVal..maxVal].
 * Удобно для подключения нескольких кнопок на один аналоговый пин через резистивный делитель.
 */
class AnalogRangeButton : public ButtonBase {
public:
    AnalogRangeButton(uint8_t pin, int minVal, int maxVal)
        : _pin(pin), _minVal(minVal), _maxVal(maxVal) {}

protected:
    /**
     * @brief Чисто виртуальный метод базового класса.
     * Возвращает true, если сигнал активен (кнопка нажата).
     */
    bool readRaw() override {
#if defined(A0)
        int val = analogRead(_pin);
        return (val >= _minVal && val <= _maxVal);
#else
        return false;
#endif
    }

private:
    uint8_t _pin;
    int _minVal;
    int _maxVal;
};

// Пример создания двух кнопок на одном аналоговом пине A0:
// Кнопка 1 замыкает через делитель ~512 (диапазон 400..600)
// Кнопка 2 замыкает накоротко на питание 1023 (диапазон 900..1024)
#if defined(A0)
AnalogRangeButton btn1(A0, 400, 600);
AnalogRangeButton btn2(A0, 900, 1024);
#endif

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println(F("\n=== Custom Button (AnalogRangeButton) Example ==="));
    Serial.println(F("Inherited from ButtonBase - all debounce, clicks, and hold logic works automatically!"));
}

void loop() {
#if defined(A0)
    btn1.tick();
    btn2.tick();

    // Обработка кнопки 1
    if (btn1.hasClicks()) {
        Serial.print(F("Analog Button 1: Clicks = "));
        Serial.println(btn1.getClicks());
    }
    if (btn1.hold()) {
        Serial.println(F("Analog Button 1: Held!"));
    }

    // Обработка кнопки 2
    if (btn2.hasClicks()) {
        Serial.print(F("Analog Button 2: Clicks = "));
        Serial.println(btn2.getClicks());
    }
    if (btn2.hold()) {
        Serial.println(F("Analog Button 2: Held!"));
    }
#else
    Serial.println(F("A0 pin not defined on this board"));
    delay(2000);
#endif
}

