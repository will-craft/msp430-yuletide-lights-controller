# MSP430 Yuletide Lights Controller

Firmware in C for the MSP430G2553 microcontroller implementing a timed LED effects controller with Bluetooth remote programming and LCD display.

Developed as a final project for the Microcontroller Systems course (EL83E) at the Federal University of Technology - Paraná (UTFPR), Curitiba, Brazil, 2019.

---

## Features

- 5 programmable LED lighting effects controlled via 3-bit DIP switch
- Real-time clock displayed on 16x2 LCD (HH:MM:SS)
- Scheduled on/off times programmable via Bluetooth
- Remote on/off toggle independent of scheduled times
- Bluetooth communication via UART (9600 bps, 8N1, RFCOMM)
- Android app integration (RoboRemoFree)

---

## Hardware

| Component | Description |
|---|---|
| MCU | Texas Instruments MSP430G2553 |
| Display | LCD 16x2 (LM016L) |
| Bluetooth | HC-06 module |
| Inputs | 3-bit DIP switch |
| Outputs | 5 LEDs with 330Ω resistors |
| Other | 10K potentiometer (LCD contrast) |

**Toolchain:** IAR Embedded Workbench for MSP430

---

## LED Effects

| DIP Switch | Effect |
|---|---|
| 111 | Set current time |
| 110 | Set turn-on time |
| 101 | Set turn-off time |
| 100 | Effect 1 - sequential (one direction) |
| 011 | Effect 2 - sequential (both directions) |
| 010 | Effect 3 - center to edges |
| 001 | Effect 4 - edges to center |
| 000 | Effect 5 - all effects in sequence |

---

## LCD Display Layout

Reference layout provided in the course specification.

![LCD Display Layout](docs/lcd-display.png)

---

## Circuit Schematic

![Circuit Schematic](docs/schematic.png)

---

## Hardware Assembly

![Hardware Assembly](docs/hardware.jpg)

---

## Academic Origin

**University:** Federal University of Technology - Paraná (UTFPR)  
**Course:** Microcontroller Systems (EL83E)  
**Year:** 2019  
**Circuit Schematic & LCD Layout:** Prof. Sergio Moribe (UTFPR)  
**LCD Driver:** lcd_Port1.c by Prof. Sergio Moribe (UTFPR)