# Buttons Library

Легковесная библиотека для работы с кнопками и дискретными сенсорами без внешних зависимостей.
Разработана как простая, быстрая и надёжная замена `EncButton` и `GyverButton`, совместима с современными версиями ядер **ESP32 Core 3.x (PlatformIO espressif32 8.x)**, **ESP8266**, **AVR**, **STM32** и **RP2040**.

---

## Основные возможности

1. **Подсчёт быстрых кликов (мультиклик):**
   - Распознавание 1, 2, 3... N нажатий подряд.
   - Таймаут между кликами настраивается (`setClickTimeout()`).
   - Проверка через `hasClicks()` и получение количества через `getClicks()`.
2. **Удержание и автоповтор (генерация кликов при долгом нажатии):**
   - Событие удержания `hold()` / `isHolded()` срабатывает один раз при превышении таймаута (`setHoldTimeout()`).
   - Генерация импульсов `step()` периодически с заданным интервалом (`setStepTimeout()`) для быстрой прокрутки меню или изменения значений.
   - Отпускание кнопки после удержания/степа **не** вызывает ложный клик.
3. **Антидребезг (Debounce):**
   - Встроенная цифровая фильтрация дребезга контактов (`setDebTimeout()`, по умолчанию 50 мс).
4. **Абстрактный базовый класс (`ButtonBase`):**
   - Позволяет подключать любые источники событий (физический пин, емкостной тач ESP32, аналоговый вход АЦП, регистр сдвига, MQTT-события и т.д.) через наследование одного метода `readRaw()`.
5. **Готовые классы «из коробки»:**
   - `Button` — физическая кнопка на GPIO пине (`INPUT`, `INPUT_PULLUP`, `INPUT_PULLDOWN`, активный `LOW` или `HIGH`).
   - `VirtButton` — виртуальная кнопка (передача состояния в `tick(bool)` или через функцию/лямбду).
   - `TouchButton` — сенсорная емкостная кнопка для ESP32 (`touchRead`).
6. **Полная обратная совместимость:**
   - Названия методов и логика совпадают с тем, как они использовались в `EncButton` и `GyverButton` (`tick()`, `hold()`, `isHolded()`, `hasClicks()`, `getClicks()`, `step()`, `pressISR()`).
   - Предусмотрен заголовочный файл `EncButton.h`, позволяющий подключить библиотеку без изменения кода в существующих проектах.

---

## Структура библиотеки

```
lib/buttons/src/
├── ButtonBase.h     # Абстрактный базовый класс со всей логикой автомата
├── Button.h         # Кнопка на физическом GPIO
├── VirtButton.h     # Виртуальная кнопка (ручной tick(state) или лямбда)
├── TouchButton.h    # Емкостная сенсорная кнопка ESP32 (touchRead)
├── Buttons.h        # Главный заголовочный файл
└── EncButton.h      # Заголовок для совместимости со старыми проектами
```

---

## Примеры использования

### 1. Физическая кнопка (Button)

```cpp
#include <Buttons.h>

// Пин 0 (BOOT), подтяжка INPUT_PULLUP, при нажатии LOW
Button btn(0, INPUT_PULLUP, LOW);

void setup() {
    Serial.begin(115200);
}

void loop() {
    btn.tick();

    // Серия быстрых кликов
    if (btn.hasClicks()) {
        switch (btn.getClicks()) {
            case 1: Serial.println("1 клик"); break;
            case 2: Serial.println("2 клика"); break;
            case 3: Serial.println("3 клика"); break;
        }
    }

    // Долгое удержание (срабатывает 1 раз)
    if (btn.hold()) {
        Serial.println("Удержание кнопки!");
    }

    // Автоповтор при удержании (шаги)
    if (btn.step()) {
        Serial.printf("Шаг автоповтора #%u\n", btn.getSteps());
    }
}
```

---

### 2. Емкостной сенсор ESP32 (TouchButton)

```cpp
#include <Buttons.h>

// GPIO 4 (Touch0), порог 40
TouchButton touchBtn(4, 40);

void loop() {
    touchBtn.tick();

    if (touchBtn.hasClicks()) {
        Serial.printf("Тач-кликов: %d\n", touchBtn.getClicks());
    }
    if (touchBtn.hold()) {
        Serial.println("Тач удерживается!");
    }
}
```

---

### 3. Виртуальная кнопка (VirtButton)

Как в проекте Watering-esp32:
```cpp
#include <Buttons.h>

VirtButton btn;

void loop() {
    // Передаём результат touchRead или внешнего флага прямо в tick
    btn.tick(touchRead(PIN_BUTTON) < TOUCH_THRESHOLD ? true : false);

    if (btn.hold()) {
        Serial.println("Сенсорная кнопка зажата!");
    }
    if (btn.hasClicks()) {
        switch (btn.getClicks()) {
            case 1: Serial.println("Клик 1"); break;
            case 2: Serial.println("Клик 2"); break;
        }
    }
}
```

Или через лямбду при инициализации:
```cpp
VirtButton btn([]() {
    return touchRead(PIN_BUTTON) < TOUCH_THRESHOLD;
});

void loop() {
    btn.tick(); // автоматически вызывает лямбду
}
```

---

### 4. Создание своего источника через абстрактный класс (`ButtonBase`)

Если у вас нестандартный источник (например, матричная клавиатура, кнопка на АЦП, MCP23017 по I2C):

```cpp
#include <Buttons.h>

class AnalogButton : public ButtonBase {
public:
    AnalogButton(uint8_t pin, int minVal, int maxVal)
        : _pin(pin), _min(minVal), _max(maxVal) {}

protected:
    bool readRaw() override {
        int val = analogRead(_pin);
        return (val >= _min && val <= _max);
    }

private:
    uint8_t _pin;
    int _min, _max;
};
```

---

## Таблица методов

| Метод | Тип | Описание |
|---|---|---|
| `tick()` | опрос | Обновляет состояние автомата (вызывает `readRaw()`) |
| `tick(bool raw)` | опрос | Обновляет состояние автомата с явным сырым значением |
| `hasClicks()` | событие | `true` на тике, когда серия кликов завершена |
| `hasClicks(num)` | событие | `true`, если завершена серия именно из `num` кликов |
| `getClicks()` / `clicks()` | значение | Число кликов в серии (1, 2, 3...) или 0 |
| `click()` | событие | Одиночный клик (срабатывает сразу при отпускании без удержания) |
| `hold()` / `isHolded()` | событие | `true` один раз при фиксации долгого нажатия |
| `hold(num)` | событие | `true` при удержании после `num` предварительных кликов |
| `holding()` / `isHold()` | состояние | `true` всё время, пока кнопка удерживается дольше holdTimeout |
| `step()` | событие | Импульс каждые stepTimeout мс при удержании (для инкремента в меню) |
| `getSteps()` | значение | Число шагов `step` в текущей сессии удержания |
| `press()` / `isPress()` | событие | Импульс при нажатии (после антидребезга) |
| `release()` / `isRelease()`| событие | Импульс при отпускании |
| `state()` / `isPressed()` | состояние | `true` пока кнопка зажата |
| `busy()` | состояние | `true`, если кнопка нажата или ждёт следующего клика |
| `pressISR()` | действие | Безопасная фиксация нажатия из прерывания |
| `reset()` | действие | Полный сброс состояния автомата |
| `clear()` | действие | Сброс флагов событий текущего тика |
| `skipEvents()` | действие | Игнорировать события до полного отпускания |
| `setDebTimeout(ms)` | настройка | Время антидребезга (по умолчанию 50 мс) |
| `setClickTimeout(ms)` | настройка | Время ожидания между кликами (по умолчанию 400 мс) |
| `setHoldTimeout(ms)` | настройка | Порог удержания (по умолчанию 600 мс) |
| `setStepTimeout(ms)` | настройка | Интервал автоповтора (по умолчанию 200 мс) |

