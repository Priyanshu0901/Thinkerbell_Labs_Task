# ThinkerBell Labs Task

[![STM32](https://img.shields.io/badge/STM32-F411CE-blue)](https://www.st.com/en/microcontrollers-microprocessors/stm32f411ce.html)
[![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-orange)](https://www.st.com/en/development-tools/stm32cubeide.html)
[![Language](https://img.shields.io/badge/Language-C-blue)](https://en.wikipedia.org/wiki/C_(programming_language))

A FreeRTOS-based firmware implementation for STM32F411 featuring multi-pattern button detection, hierarchical menu navigation, and 16-LED output control via shift register.

## Overview

This project implements a complete embedded user interface system controlled entirely through three physical buttons. The system features debounced multi-press detection (single, double, triple, and long press), a hierarchical menu structure, and real-time LED pattern display with adjustable brightness control.

**Target Platform:** STM32F411CEU6  
**RTOS:** FreeRTOS (CMSIS-RTOS v2) 

## Hardware Requirements

### Microcontroller
- STM32F411CEU6 (Black Pill board)
- System Clock: 100 MHz
- HSE: 25 MHz crystal

### Peripherals
- 3x Push buttons (active-low with external pull-ups)
- 2x SN74HC595 shift registers (cascaded for 16-bit output)
- 16x LEDs with current limiting resistors
- 1x User status LED
- UART2 for debug logging (115200 baud)

### Pin Assignments

| Pin   | Function        | Description                    |
|-------|-----------------|--------------------------------|
| PB13  | BTN_1          | Button 1 input                 |
| PB14  | BTN_2          | Button 2 input                 |
| PB15  | BTN_3          | Button 3 input                 |
| PA6   | SR_DATA        | Shift register serial data     |
| PA7   | SR_CLK         | Shift register serial clock    |
| PB0   | RCLK           | Shift register latch clock     |
| PB1   | SR_CLR         | Shift register clear (active-low)|
| PA5   | OE_PIN         | Output enable (PWM/TIM2_CH1)   |
| PA2   | USART2_TX      | Debug UART transmit            |

## System Architecture

### Thread Structure

**ButtonInput Thread** (Priority: Normal, Stack: 4KB)
- Polls button GPIO every 5ms
- Implements 30ms debounce filter
- Detects press patterns using timestamp analysis
- Queues button events for menu processing

**MenuLogic Thread** (Priority: Normal, Stack: 4KB)
- Receives button events from queue
- Implements hierarchical state machine
- Manages menu navigation and mode switching
- Generates display update commands
- Handles auto-mode pattern cycling (2-second interval)

**DisplayManager Thread** (Priority: Normal, Stack: 4KB)
- Receives display patterns from queue
- Drives cascaded SN74HC595 shift registers
- Controls brightness via PWM (0-10 levels, 480 Hz)
- Protects hardware access with mutex

### Inter-Task Communication

```
ButtonInput --[button_event_queue]--> MenuLogic --[display_pattern_queue]--> DisplayManager
                                                                                    |
                                                                              [shiftreg_mutex]
                                                                                    |
                                                                              SN74HC595 Hardware
```

**Queues:**
- button_event_queue: 16 elements of 12 bytes (BTN_event_t)
- display_pattern_queue: 16 elements of 4 bytes (Display_update_data_t)

**Mutexes:**
- shiftreg_mutex: Protects shift register hardware access

## Button Detection

### Press Types

| Type          | Timing Requirement           | Detection Method                    |
|---------------|------------------------------|-------------------------------------|
| Single Press  | Press and release < 300ms    | One click, 300ms idle timeout       |
| Double Press  | Two presses within 500ms     | Two clicks, 500ms window            |
| Triple Press  | Three presses within 700ms   | Three clicks, immediate trigger     |
| Long Press    | Hold > 1500ms                | Continuous press duration           |

### Debouncing
- Software debounce: 30ms stability window
- State change detection: Rising and falling edge tracking
- Jitter rejection: Pin must remain stable for debounce period

## Menu Structure

```
Main Menu
├── Brightness              (LED: 0x1000)
│   └── Brightness Setting  (LED: 0x03FF, adjustable 0-10)
│
├── Mode                    (LED: 0x2000)
│   ├── Manual Selection    (LED: 0x200F)
│   │   └── Manual Mode     (LED: Pattern, user cycles)
│   └── Auto Selection      (LED: 0x20F0)
│       └── Auto Mode       (LED: Pattern, auto-cycles 2s)
│
├── Info                    (LED: 0x4000)
│   └── Firmware Version    (LED: 0x0A05, v10.5)
│
└── Reset                   (LED: 0x8000)
    └── Reset Confirmation  (LED: 0x80FF, double-press to confirm)
```

### Menu Navigation Controls

**Main Menu:**
- BTN1 Single: Move selection forward (circular)
- BTN2 Single: Enter selected menu item
- BTN3 Single: Move selection backward (circular)
- BTN3 Long: Power off system

**Brightness Setting:**
- BTN2 Single: Increase brightness (max: 10)
- BTN3 Single: Decrease brightness (min: 0)
- BTN1 Single: Return to main menu

**Mode Selection:**
- BTN1 Single: Toggle between Manual and Auto
- BTN2 Single: Enter selected mode
- BTN3 Single: Cancel, return to main menu

**Manual Mode:**
- BTN1 Single: Cycle through 16 LED patterns
- BTN2 Single: Save pattern, return to mode selection
- BTN3 Single: Cancel without saving

**Auto Mode:**
- Automatic pattern cycling every 2 seconds
- BTN1 Single: Exit to mode selection

**Reset Confirmation:**
- BTN2 Double: Confirm reset to defaults
- BTN3 Single: Cancel, return to main menu

**Power Off State:**
- All inputs ignored except BTN1 Long to restart

### Default Settings

| Parameter      | Default Value |
|----------------|---------------|
| Brightness     | 5 (medium)    |
| Mode           | Manual        |
| Pattern Index  | 0 (0x0001)    |

## LED Patterns

The system uses 16 predefined patterns for visual display:

```c
Pattern 0:  0x0001  (1 LED)
Pattern 1:  0x0003  (2 LEDs)
Pattern 2:  0x0007  (3 LEDs)
Pattern 3:  0x000F  (4 LEDs)
Pattern 4:  0x001F  (5 LEDs)
Pattern 5:  0x003F  (6 LEDs)
Pattern 6:  0x007F  (7 LEDs)
Pattern 7:  0x00FF  (8 LEDs)
Pattern 8:  0x01FF  (9 LEDs)
Pattern 9:  0x03FF  (10 LEDs)
Pattern 10: 0x07FF  (11 LEDs)
Pattern 11: 0x0FFF  (12 LEDs)
Pattern 12: 0x1FFF  (13 LEDs)
Pattern 13: 0x3FFF  (14 LEDs)
Pattern 14: 0x7FFF  (15 LEDs)
Pattern 15: 0xFFFF  (16 LEDs)
```

## Brightness Control

PWM-based brightness control using TIM2_CH1 on OE pin (active-low):

```
PWM Frequency: 500 Hz (flicker-free)
Timer Configuration:
  - Prescaler: 999 (50 MHz / 1000 = 50 kHz)
  - ARR: 99 (50 kHz / 100 = 500 Hz)
  - Duty Cycle: Inverted (0% = max brightness, 100% = off)

Brightness Levels:
  Level 0:  100% duty (LEDs off)
  Level 5:   50% duty (medium)
  Level 10:   0% duty (maximum brightness)
```

## Project Structure (Important)

```
Core/
├── Inc/
│   ├── Button.h              Button driver interface
│   ├── Display.h             Display manager interface
│   ├── Menu.h                Menu state machine interface
│   ├── SN74HC595.h           Shift register driver interface
│   ├── debug_logger.h        UART logging utilities
│   └── main.h                Pin definitions and includes
│
└── Src/
    ├── Button.c              Button state machine and detection
    ├── Display.c             Display manager implementation
    ├── Menu.c                Menu logic and state transitions
    ├── SN74HC595.c           Shift register bit-banging driver
    ├── debug_logger.c        Colored UART logging with timestamps
    ├── freertos.c            Task initialization and scheduling
    └──  main.c                System initialization and main loop

```
