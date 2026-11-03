# Buttons Library

A lightweight, zero-dependency C++ library for handling buttons, capacitive touch, and discrete sensors on microcontrollers.  
Designed as a clean, fast, and robust replacement for `EncButton` and `GyverButton`. Fully compatible with modern **ESP32 Core 3.x (PlatformIO espressif32 8.x)**, **ESP8266**, **AVR**, **STM32**, and **RP2040 (Raspberry Pi Pico)**.

[English](README_EN.md) | **Русский**

Легковесная библиотека для работы с кнопками и дискретными сенсорами без внешних зависимостей.  
Разработана как простая, быстрая и надёжная замена `EncButton` и `GyverButton`, совместима с современными версиями ядер **ESP32 Core 3.x (PlatformIO espressif32 8.x)**, **ESP8266**, **AVR**, **STM32** и **RP2040 (Raspberry Pi Pico)**.

---

## Основные возможности

1. **Подсчёт быстрых кликов (мультиклик):**
   - Распознавание 1, 2, 3... $N$ нажатий подряд.
   - Настраиваемый таймаут между кликами (`setClickTimeout()`).
   - Проверка через `hasClicks()` и получение количества через `getClicks()`.
2. **Удержание и автоповтор (генерация кликов при долгом нажатии):**
   - Событие удержания `hold()` / `isHolded()` срабатывает один раз при превышении таймаута (`setHoldTimeout()`).
   - Состояние удержания `holding()` / `isHold()` активно всё время, пока кнопка удерживается.
   - Генерация импульсов `step()` периодически с заданным интервалом (`setStepTimeout()`) для быстрой прокрутки меню или инкремента значений.
   - Отпускание кнопки после удержания/степа **не** вызывает паразитный клик.
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

---

## Структура библиотеки

```
Buttons/
├── library.json         # Манифест PlatformIO
├── library.properties   # Манифест Arduino IDE
├── LICENSE              # Лицензия GNU LGPLv3
├── README.md            # Документация (Русский)
├── README_EN.md         # Documentation (English)
├── examples/            # Примеры скетчей для Arduino и PlatformIO
│   ├── 01.PhysicalButton/
│   ├── 02.TouchButton/
│   ├── 03.VirtButton/
│   └── 04.CustomButton/
└── src/
    ├── ButtonBase.h     # Абстрактный базовый класс со всей логикой автомата
    ├── Button.h         # Кнопка на физическом GPIO
    ├── VirtButton.h     # Виртуальная кнопка (ручной tick(state) или лямбда)
    ├── TouchButton.h    # Емкостная сенсорная кнопка ESP32 (touchRead)
    └── Buttons.h        # Главный заголовочный файл
```

---

## Установка

### PlatformIO
Добавьте зависимость в ваш `platformio.ini`:
```ini
lib_deps =
    https://github.com/SerhiiLe/Buttons.git
```
Либо скопируйте папку с библиотекой в локальный каталог `lib/` вашего проекта.

### Arduino IDE
1. Скачайте репозиторий в виде ZIP-архива: [Buttons.zip](https://github.com/SerhiiLe/Buttons/archive/refs/heads/main.zip).
2. В Arduino IDE выберите пункт меню: **Скетч -> Подключить библиотеку -> Добавить .ZIP библиотеку...** и укажите скачанный архив.
3. Либо распакуйте папку библиотеки в каталог `libraries/` вашей папки со скетчами Arduino.

---

## Примеры использования

### 1. Физическая кнопка (Button)

```cpp
#include <Buttons.h>

// Пин 0 (кнопка BOOT), подтяжка INPUT_PULLUP, при нажатии сигнал LOW
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

    // Долгое удержание (срабатывает 1 раз при превышении holdTimeout)
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

**Вариант А:** Ручная передача значения в `tick(bool)`:
```cpp
#include <Buttons.h>

VirtButton btn;

void loop() {
    // Передаём результат внешнего условия напрямую в tick
    btn.tick(touchRead(4) < 40 ? true : false);

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

**Вариант Б:** Передача лямбда-функции при инициализации:
```cpp
VirtButton btn([]() {
    return touchRead(4) < 40;
});

void loop() {
    btn.tick(); // автоматически вызывает лямбду
}
```

---

### 4. Создание своего источника через абстрактный класс (`ButtonBase`)

Если вам требуется подключить нестандартный источник (кнопка на АЦП через делитель, матричная клавиатура, I2C экспандер MCP23017 и т.д.):

```cpp
#include <Buttons.h>

class AnalogButton : public ButtonBase {
public:
    AnalogButton(uint8_t pin, int minVal, int maxVal)
        : _pin(pin), _minVal(minVal), _maxVal(maxVal) {}

protected:
    bool readRaw() override {
        int val = analogRead(_pin);
        return (val >= _minVal && val <= _maxVal);
    }

private:
    uint8_t _pin;
    int _minVal, _maxVal;
};
```

---

## Примеры в комплекте

В каталоге [`examples/`](examples/) представлены готовые скетчи:
*   [`01.PhysicalButton`](examples/01.PhysicalButton/01.PhysicalButton.ino) — работа с тактовой кнопкой на GPIO, серия кликов, удержание и автоповтор.
*   [`02.TouchButton`](examples/02.TouchButton/02.TouchButton.ino) — использование сенсорного емкостного пина ESP32 и вывод калибровки.
*   [`03.VirtButton`](examples/03.VirtButton/03.VirtButton.ino) — виртуальная кнопка с передачей значения в `tick()` или через callback/лямбду.
*   [`04.CustomButton`](examples/04.CustomButton/04.CustomButton.ino) — создание собственной аналоговой кнопки на АЦП через наследование `ButtonBase`.

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
| `step(num)` | событие | Импульс автоповтора с `num` предварительными кликами |
| `getSteps()` | значение | Число шагов `step` в текущей сессии удержания |
| `press()` / `isPress()` | событие | Импульс при нажатии (после антидребезга) |
| `release()` / `isRelease()`| событие | Импульс при отпускании |
| `releaseHold()` | событие | `true`, если кнопка отпущена после удержания (без step) |
| `releaseStep()` | событие | `true`, если кнопка отпущена после шагов автоповтора |
| `state()` / `isPressed()` | состояние | `true` пока кнопка зажата |
| `busy()` | состояние | `true`, если кнопка зажата или ждёт следующего клика серии |
| `pressISR()` | действие | Безопасная фиксация нажатия из прерывания |
| `reset()` | действие | Полный сброс состояния автомата |
| `clear()` | действие | Сброс флагов событий текущего тика |
| `skipEvents()` | действие | Игнорировать события до полного отпускания |
| `attach(handler)` | коллбэк | Подключить функцию-обработчик событий |
| `detach()` | коллбэк | Отключить функцию-обработчик |
| `setDebTimeout(ms)` | настройка | Время антидребезга (по умолчанию 50 мс) |
| `setClickTimeout(ms)` | настройка | Время ожидания между кликами (по умолчанию 400 мс) |
| `setHoldTimeout(ms)` | настройка | Порог удержания (по умолчанию 600 мс) |
| `setStepTimeout(ms)` | настройка | Интервал автоповтора (по умолчанию 200 мс) |

---

## Переносимость на другие платформы (STM32, Raspberry Pi Pico, AVR, ПК)

Библиотека написана на стандартном **C++** без ассемблерных вставок и привязок к низкоуровневым регистрам:

1. **`ButtonBase`** опирается исключительно на стандартные типы (`uint8_t`, `uint16_t`, `uint32_t`, `bool`) и функцию времени `millis()`:
   * В среде Arduino (STM32duino, Pico SDK / Earle Philhower / Mbed, AVR Arduino, ESP) функция `millis()` доступна сразу.
   * Для сборки вне Arduino (чистый STM32 HAL, FreeRTOS или Pico SDK) достаточно определить свою функцию `uint32_t millis()`, возвращающую миллисекунды (например, через `HAL_GetTick()` на STM32 или `to_ms_since_boot(get_absolute_time())` на Pico).
2. **`Button`** использует только стандартный интерфейс Arduino API (`digitalRead()` и `pinMode()`), доступный на всех поддерживаемых платформах.
3. **`VirtButton`** полностью аппаратно-независим. Для AVR отключена поддержка `std::function` и используются указатели на функции `bool (*)()`. На 32-битных МК (STM32, Pico, ESP32) работают как указатели, так и C++ лямбды.
4. **`TouchButton`** на чипах без аппаратного емкостного сенсора (AVR, STM32, RP2040, ESP8266, ESP32-C3) компилируется без ошибок: `readTouch()` возвращает `0`, а `readRaw()` возвращает `false`.

---

## Лицензия

Библиотека распространяется под свободной лицензией **GNU Lesser General Public License v3.0 (LGPLv3)**.  
Подробности см. в файле [LICENSE](LICENSE).
