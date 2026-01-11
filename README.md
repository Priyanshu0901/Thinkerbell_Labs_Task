# ThinkerBell Labs Task

[![STM32](https://img.shields.io/badge/STM32-F411CE-blue)](https://www.st.com/en/microcontrollers-microprocessors/stm32f411ce.html)
[![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-orange)](https://www.st.com/en/development-tools/stm32cubeide.html)
[![Language](https://img.shields.io/badge/Language-C-blue)](https://en.wikipedia.org/wiki/C_(programming_language))

## Architecture

```
Thread 1: Button Input       → Debounce & detect events
Thread 2: Menu Logic         → Process input & manage state  
Thread 3: Display Manager    → Drive shift register hardware
```

**Communication**:
- `button_event_queue`: Button events → Menu
- `display_pattern_queue`: Display updates → Hardware
- `shiftreg_mutex`: Thread-safe hardware access

## Project Structure

```
├── Button.c/h              # Multi-press button driver with debouncing
├── Menu.c/h                # Menu state machine and navigation logic
├── Display.c/h             # Display manager abstraction layer
├── SN74HC595.c/h           # Shift register driver (16-bit, PWM brightness)
├── debug_logger.c/h        # UART logging with levels and colors
├── freertos.c              # Task initialization and main loops
└── main.c                  # Hardware initialization
```

## Hardware Connections

### Buttons (Active Low)
| Button | GPIO | Function |
|--------|------|----------|
| BTN_1  | -    | Navigate / Cycle |
| BTN_2  | -    | Select / Confirm |
| BTN_3  | -    | Back / Cancel / Power |

### SN74HC595 Shift Register
| Pin | STM32 | Description |
|-----|-------|-------------|
| SER | PA6 | Serial Data |
| SRCLK | PA7 | Serial Clock |
| RCLK | PB0 | Latch Clock |
| SRCLR | PB1 | Clear (Active Low) |
| OE | TIM2_CH1 | PWM Brightness Control |

**Configuration**: Two 74HC595 cascaded for 16 LEDs (B15-B0)

## Menu Structure

```
Main Menu
├── Brightness (B15)
│   └── Settings: 0-10 levels
├── Mode (B14)
│   ├── Manual: 16 selectable patterns
│   └── Auto: Cycle patterns every 2s
├── Info (B13)
│   └── Firmware version display
└── Reset (B12)
    └── Factory reset (double-press confirm)
```

## Button Controls

### Main Menu
- **BTN1 Single**: Next item
- **BTN2 Single**: Enter item
- **BTN3 Single**: Previous item
- **BTN3 Long**: Power off

### Brightness
- **BTN2 Single**: Increase (+1)
- **BTN3 Single**: Decrease (-1)
- **BTN1 Single**: Return to menu

### Mode Selection
- **BTN1 Single**: Toggle Manual/Auto
- **BTN2 Single**: Enter mode
- **BTN3 Single**: Cancel

### Manual Mode
- **BTN1 Single**: Next pattern
- **BTN2 Single**: Save & exit
- **BTN3 Single**: Cancel

### Auto Mode
- **BTN1 Single**: Exit auto mode
- (Auto-cycles every 2 seconds)

### Reset
- **BTN2 Double**: Confirm reset
- **BTN3 Single**: Cancel

### Power Off
- **BTN1 Long**: Power on

## ⚙️ Default Settings

- **Brightness**: 5 (medium)
- **Mode**: Manual
- **Pattern**: Pattern 0 (0x0001)

## Configuration

### Timer (TIM2)
- **Frequency**: 1 kHz PWM
- **Channel 1**: OE pin (brightness control)
- **Prescaler**: 960-1 (@ 96 MHz)
- **ARR**: 100-1

### FreeRTOS
- **Kernel**: CMSIS-RTOS v2
- **Stack Size**: 4KB per task
- **Priority**: Normal for all tasks
- **Queues**: 16 messages each

### UART Logging
- **UART2**: 115200 baud
- **Format**: `(timestamp) [LEVEL] TAG : message`
- **Colors**: ANSI terminal colors

## Getting Started

### 1. Hardware Setup
- Connect shift registers as per pinout
- Configure TIM2 for PWM output
- Connect UART2 for debug logging

### 2. Build & Flash
```bash
# Using STM32CubeIDE
1. Import project
2. Build (Ctrl+B)
3. Flash (F11)
```

### 3. Monitor Output
```bash
# Serial monitor (115200 baud)
screen /dev/ttyUSB0 115200
# or
minicom -D /dev/ttyUSB0 -b 115200
```

## LED Patterns

| Page | Pattern | LEDs On |
|------|---------|---------|
| Brightness | `0x1000` | B12 |
| Mode Select | `0x2000` | B13 |
| Info | `0x4000` | B14 |
| Reset | `0x8000` | B15 |
| Brightness 5 | `0x001F` | B0-B4 |
| Manual Mode | `0x800F` | B15, B0-B3 |
| Auto Mode | `0x80F0` | B15, B4-B7 |
| Firmware 10.5 | `0x0A05` | Version encoded |

## 🧪 Testing

### Without Physical Buttons
Use debugger breakpoints and variable modification:
```c
// In debugger, inject event:
BTN_event_t test_event = {
    .id = BTN_1,
    .type = SINGLE_PRESS,
    .timestamp = HAL_GetTick()
};
osMessageQueuePut(button_event_queueHandle, &test_event, 0, 0);
```

### Display Test
```c
// Direct display update:
Display_update_data_t test = {
    .data = 0xFFFF,        // All LEDs
    .brightness = 10       // Max brightness
};
Display_update(&DisplayManager, &test);
```

## API Documentation

### Button API
- `Button_ctor()`: Initialize button
- `Button_read()`: Poll button state (call every 5ms)

### Menu API
- `Menu_ctor()`: Initialize menu system
- `Menu_process_input()`: Handle button event
- `Menu_is_auto_mode_active()`: Check auto mode
- `Menu_auto_cycle_pattern()`: Trigger pattern cycle

### Display API
- `Display_update()`: Set pattern + brightness
- `Display_set_pattern()`: Change pattern only
- `Display_set_brightness()`: Change brightness only
- `Display_enable/disable()`: Power management
- `Display_clear()`: Turn off all LEDs

### Shift Register API
- `SN74HC595_write()`: Write 16-bit pattern
- `SN74HC595_set_brightness()`: Set PWM duty cycle
- `SN74HC595_update()`: Atomic update
- `SN74HC595_clear()`: Hardware clear

## Debugging

### Enable Verbose Logging
All modules use `debug_logger` with levels:
- `LOG_DEBUG`: Detailed operation info
- `LOG_INFO`: Normal operation events
- `LOG_WARN`: Warnings and clamps
- `LOG_ERROR`: Error conditions

## SNx4HC595 Timing logic 

|Pin|Pin Function|Remarks|
|---|---|---|
| OE | Output Enable | Using for brightness (Active Low)|
| RCLK | Register Clock Input | RCLK Snap on 16th to update display |
| SER | Serial Input| Data IN |
| SRCLK | Serial Clock | Data CLK |
| SRCLR | Not Used | Active Low - HIGH|

## Menu to 16 LED Map

| Func | B15 | B14 | B13 | B12 | B11 | B10 | B9 | B8 | B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 |
| --- | --- | --- | --- | --- | --- | --- | -- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Brightness | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Mode Select| 0 | 0 | 1| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Info | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Reset | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Brightness Behavior (0)| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
| Brightness Behavior (10) | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| Mode Manual| 0 | 0 | 1| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 |
| Mode Auto | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| MANUAL MODE BEHAVIOR (Pattern = x) | x | x | x | x | x | x | x | x | x | x | x | x | x | x | x | x |
| AUTO MODE BEHAVIOR (Pattern = x cycle 2 seconds) | x | x | x | x | x | x | x | x | x | x | x | x | x | x | x | x |
| Firmware version (M.m)| M8 | M7 | M6 | M5 | M4 | M3 | M2 | M1 | M0 | 0 | m5 | m4 | m3 | m2 | m1 | m0 |
| Reset Screen | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |

