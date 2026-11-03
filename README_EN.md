# Buttons Library

**English** | [Русский](README.md)

A lightweight, zero-dependency C++ library for handling buttons, capacitive touch, and discrete sensors on microcontrollers.  
Designed as a clean, fast, and robust replacement for `EncButton` and `GyverButton`. Fully compatible with modern **ESP32 Core 3.x (PlatformIO espressif32 8.x)**, **ESP8266**, **AVR**, **STM32**, and **RP2040 (Raspberry Pi Pico)**.

---

## Features

1. **Fast Multi-Click Detection:**
   - Detects sequences of 1, 2, 3... $N$ rapid clicks.
   - Configurable timeout between clicks (`setClickTimeout()`).
   - Query via `hasClicks()` and retrieve the exact count with `getClicks()`.
2. **Hold & Auto-Repeat (Step Events on Long Press):**
   - Single-pulse event `hold()` / `isHolded()` triggered once upon reaching the hold threshold (`setHoldTimeout()`).
   - State getter `holding()` / `isHold()` remains `true` as long as the button is held down.
   - Periodic `step()` pulse generation at a customizable rate (`setStepTimeout()`) for rapid menu scrolling or continuous value adjustments.
   - Releasing the button after hold or step **never** generates a false click.
3. **Contact Debounce Filter:**
   - Built-in digital filtering of contact bounce on both press and release (`setDebTimeout()`, default 50 ms).
4. **Abstract Base Class (`ButtonBase`):**
   - Easily plug in any custom input source (GPIO, capacitive touch, analog ADC divider, shift registers, I2C port expanders, MQTT events, etc.) by implementing a single virtual method: `readRaw()`.
5. **Ready-to-Use Classes Out of the Box:**
   - `Button` — physical tactile button on GPIO (`INPUT`, `INPUT_PULLUP`, `INPUT_PULLDOWN`, active `LOW` or `HIGH`).
   - `VirtButton` — virtual button (pass state directly into `tick(bool)` or via a callback/lambda).
   - `TouchButton` — capacitive touch sensor for ESP32 (`touchRead`).
6. **Backward Compatibility:**
   - Method names and behavior match common patterns from `EncButton` and `GyverButton` (`tick()`, `hold()`, `isHolded()`, `hasClicks()`, `getClicks()`, `step()`, `pressISR()`).

---

## Library Structure

```
Buttons/
├── library.json         # PlatformIO library manifest
├── library.properties   # Arduino IDE library manifest
├── LICENSE              # GNU LGPLv3 License
├── README.md            # Documentation (Russian)
├── README_EN.md         # Documentation (English)
├── examples/            # Example sketches for Arduino and PlatformIO
│   ├── 01.PhysicalButton/
│   ├── 02.TouchButton/
│   ├── 03.VirtButton/
│   └── 04.CustomButton/
└── src/
    ├── ButtonBase.h     # Abstract base class containing the state machine
    ├── Button.h         # Physical GPIO button
    ├── VirtButton.h     # Virtual button (manual tick(state) or lambda)
    ├── TouchButton.h    # ESP32 capacitive touch button (touchRead)
    └── Buttons.h        # Main umbrella header

```

---

## Installation

### PlatformIO
Add the repository to your `platformio.ini`:
```ini
lib_deps =
    https://github.com/SerhiiLe/Buttons.git
```
Alternatively, copy the library folder into your project's local `lib/` directory.

### Arduino IDE
1. Download the repository as a ZIP archive: [Buttons.zip](https://github.com/SerhiiLe/Buttons/archive/refs/heads/main.zip).
2. In the Arduino IDE, go to: **Sketch -> Include Library -> Add .ZIP Library...** and select the downloaded file.
3. Alternatively, extract the library folder into the `libraries/` directory inside your Arduino sketchbook folder.

---

## Usage Examples

### 1. Physical GPIO Button (`Button`)

```cpp
#include <Buttons.h>

// Pin 0 (BOOT button), internal pull-up (INPUT_PULLUP), active LOW
Button btn(0, INPUT_PULLUP, LOW);

void setup() {
    Serial.begin(115200);
}

void loop() {
    btn.tick();

    // Fast multi-clicks
    if (btn.hasClicks()) {
        switch (btn.getClicks()) {
            case 1: Serial.println("1 Click"); break;
            case 2: Serial.println("2 Clicks"); break;
            case 3: Serial.println("3 Clicks"); break;
        }
    }

    // Long press event (fires once when hold timeout is reached)
    if (btn.hold()) {
        Serial.println("Button Held!");
    }

    // Auto-repeat step pulses while held
    if (btn.step()) {
        Serial.printf("Auto-repeat Step #%u\n", btn.getSteps());
    }
}
```

---

### 2. ESP32 Capacitive Touch Sensor (`TouchButton`)

```cpp
#include <Buttons.h>

// GPIO 4 (Touch0), threshold 40
TouchButton touchBtn(4, 40);

void loop() {
    touchBtn.tick();

    if (touchBtn.hasClicks()) {
        Serial.printf("Touch clicks: %d\n", touchBtn.getClicks());
    }
    if (touchBtn.hold()) {
        Serial.println("Touch button held!");
    }
}
```

---

### 3. Virtual Button (`VirtButton`)

**Option A:** Pass state manually in `tick(bool)`:
```cpp
#include <Buttons.h>

VirtButton btn;

void loop() {
    // Pass external condition directly into tick()
    btn.tick(touchRead(4) < 40 ? true : false);

    if (btn.hold()) {
        Serial.println("Virtual sensor held!");
    }
    if (btn.hasClicks()) {
        switch (btn.getClicks()) {
            case 1: Serial.println("Click 1"); break;
            case 2: Serial.println("Click 2"); break;
        }
    }
}
```

**Option B:** Pass a lambda function during initialization:
```cpp
VirtButton btn([]() {
    return touchRead(4) < 40;
});

void loop() {
    btn.tick(); // automatically invokes the lambda
}
```

---

### 4. Custom Source via Abstract Base Class (`ButtonBase`)

To interface non-standard hardware (e.g. analog buttons via resistor dividers on ADC, matrix keypads, I2C port expanders):

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

## Bundled Examples

The [`examples/`](examples/) directory includes ready-to-run sketches:
*   [`01.PhysicalButton`](examples/01.PhysicalButton/01.PhysicalButton.ino) — GPIO tactile button, multi-clicks, hold, and step auto-repeat.
*   [`02.TouchButton`](examples/02.TouchButton/02.TouchButton.ino) — ESP32 capacitive touch pin usage with live calibration readout.
*   [`03.VirtButton`](examples/03.VirtButton/03.VirtButton.ino) — Virtual button with manual `tick(state)` or callback/lambda.
*   [`04.CustomButton`](examples/04.CustomButton/04.CustomButton.ino) — Custom ADC resistor ladder button subclassing `ButtonBase`.

---

## Method Reference

| Method | Type | Description |
|---|---|---|
| `tick()` | poll | Updates the state machine (invokes `readRaw()`) |
| `tick(bool raw)` | poll | Updates the state machine with an explicit raw value |
| `hasClicks()` | event | `true` on the tick when a click sequence has finished |
| `hasClicks(num)` | event | `true` if a sequence finished with exactly `num` clicks |
| `getClicks()` / `clicks()` | value | Number of clicks in the sequence (1, 2, 3...) or 0 |
| `click()` | event | Single click (fires immediately on release without hold) |
| `hold()` / `isHolded()` | event | `true` once when long press threshold is reached |
| `hold(num)` | event | `true` on hold following `num` preliminary clicks |
| `holding()` / `isHold()` | state | `true` continuously while held longer than holdTimeout |
| `step()` | event | Periodic pulse every stepTimeout ms while held |
| `step(num)` | event | Step pulse with `num` preliminary clicks |
| `getSteps()` | value | Number of steps generated during the current hold session |
| `press()` / `isPress()` | event | Pulse upon press (after debounce) |
| `release()` / `isRelease()`| event | Pulse upon release |
| `releaseHold()` | event | `true` if released after hold (without step) |
| `releaseStep()` | event | `true` if released after step auto-repeat |
| `state()` / `isPressed()` | state | `true` as long as the button is pressed down |
| `busy()` | state | `true` if button is pressed or waiting for another click |
| `pressISR()` | action | Safe press trigger callable from an interrupt service routine |
| `reset()` | action | Fully resets the state machine |
| `clear()` | action | Clears event flags for the current tick |
| `skipEvents()` | action | Ignores subsequent events until released |
| `attach(handler)` | callback | Attach an action handler callback |
| `detach()` | callback | Detach the action handler callback |
| `setDebTimeout(ms)` | config | Debounce time (default: 50 ms) |
| `setClickTimeout(ms)` | config | Timeout between clicks in a burst (default: 400 ms) |
| `setHoldTimeout(ms)` | config | Hold detection threshold (default: 600 ms) |
| `setStepTimeout(ms)` | config | Auto-repeat step interval (default: 200 ms) |

---

## Cross-Platform Portability (STM32, RP2040, AVR, PC / Bare Metal)

The library is implemented in clean, standard **C++** with no hardware register hacks:

1. **`ButtonBase`** depends solely on standard integer types (`uint8_t`, `uint16_t`, `uint32_t`, `bool`) and a timekeeping function `millis()`:
   * In Arduino ecosystems (STM32duino, Pico SDK / Earle Philhower / Mbed, AVR, ESP), `millis()` is provided out of the box.
   * To use the library outside Arduino (bare-metal STM32 HAL, FreeRTOS, Pico C SDK), simply define a `uint32_t millis()` function (e.g. mapped to `HAL_GetTick()` or `to_ms_since_boot(get_absolute_time())`).
2. **`Button`** uses only standard Arduino API calls (`digitalRead()`, `pinMode()`), supported on all Arduino cores.
3. **`VirtButton`** is completely hardware-agnostic. On AVR, `std::function` is omitted to preserve SRAM, using function pointers `bool (*)()`. On 32-bit platforms (STM32, RP2040, ESP32), both function pointers and modern C++ lambdas are supported.
4. **`TouchButton`** compiles safely on platforms without hardware capacitive touch (AVR, STM32, RP2040, ESP8266, ESP32-C3): `readTouch()` returns `0`, and `readRaw()` returns `false`.

---

## License

This library is licensed under the **GNU Lesser General Public License v3.0 (LGPLv3)**.  
See the [LICENSE](LICENSE) file for details.

