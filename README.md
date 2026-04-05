# Gray_mobility

# STM32 Automotive Signal & Hazard Controller

A robust, interrupt-driven C implementation of an automotive lighting controller for the STM32F4 series. This firmware manages Left/Right turn signals and Hazard light functionality using a Finite State Machine (FSM) and PWM-modulated outputs.

## 🚀 Features

* **FSM Architecture**: Logic is managed via a clean State Machine (IDLE, LEFT, RIGHT, HAZARD).
* **Long-Press Logic**: Signals are toggled using a 1000ms hold time to simulate professional industrial controls.
* **PWM Output**: Uses TIM3 (Channels 1 & 2) to drive LEDs, providing a foundation for brightness control and "soft-blink" effects.
* **Non-Blocking Delay**: Utilizes the ARM SysTick timer for debouncing and timing, ensuring the main loop remains responsive.
* **Hazard Priority**: Simultaneous long-press on both inputs triggers Hazard mode, overriding individual turn signals.

## 🛠 Hardware Configuration

* **MCU**: STM32F4 (e.g., STM32F401/411 Nucleo or Discovery)
* **Inputs**:
    * `PA0`: Left Signal Button (Active Low/Pull-up)
    * `PA1`: Right Signal Button (Active Low/Pull-up)
* **Outputs**:
    * `PB4`: Left Indicator LED (TIM3_CH1)
    * `PB5`: Right Indicator LED (TIM3_CH2)

## 🕹 Logic Flow

1. **Initialization**: Configures GPIO for buttons/PWM and initializes TIM3 for a 1kHz carrier frequency.
2. **Polling**: The main loop runs every 100ms to sample button states.
3. **Event Detection**: 
    * Hold Left/Right for >1s to toggle the turn signal.
    * Hold BOTH for >1s to enter Hazard mode.
4. **Blink Control**: A software timer toggles the `led_state` every 300ms to create the flashing effect.

## 📂 File Structure

* `main.c`: Contains the peripheral initialization (RCC, GPIO, TIM3), the SysTick handler, and the core application logic.

## 🔧 How to Build

1. Import this project into your preferred IDE (STM32CubeIDE, Keil uVision, or VS Code with Cortex-Debug).
2. Ensure the standard `stm32f4xx.h` headers are in your include path.
3. Compile and flash to your target board.

## 📝 License
This project is open-source and available under the MIT License.
