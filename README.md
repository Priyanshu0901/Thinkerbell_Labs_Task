# ThinkerBell Labs Task

[![STM32](https://img.shields.io/badge/STM32-F411CE-blue)](https://www.st.com/en/microcontrollers-microprocessors/stm32f411ce.html)
[![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-orange)](https://www.st.com/en/development-tools/stm32cubeide.html)
[![Language](https://img.shields.io/badge/Language-C-blue)](https://en.wikipedia.org/wiki/C_(programming_language))

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
| Mode Manual (Default)| 0 | 0 | 1| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Mode Auto | 0 | 0 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Info | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Reset | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Brightness Behavior (0)| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
| Brightness Behavior (10) | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| MANUAL MODE BEHAVIOR (Pattern = x) | x | x | x | x | x | x | x | x | x | x | x | x | x | x | x | x |
| AUTO MODE BEHAVIOR (Pattern = x cycle 2 seconds) | x | x | x | x | x | x | x | x | x | x | x | x | x | x | x | x |
| Firmware version (M.m)| M8 | M7 | M6 | M5 | M4 | M3 | M2 | M1 | M0 | 0 | m | m4 | m3 | m2 | m1 | m0 |
| Reset Screen | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |

Pattern used