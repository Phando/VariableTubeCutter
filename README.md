# Variable Tube Cutter

**Variable Tube Cutter** is an adjustable tool designed to precisely cut tubes of varying diameters. Developed by [Phando](https://github.com/Phando), this project combines easy-to-understand code, 3D printable parts, and a robust design to help makers and professionals alike achieve clean, precise cuts.

![cutter2](https://github.com/user-attachments/assets/ceb8bce3-28ff-4529-ac6c-bcf175325cd8)

---
## Note

This is a new project and requires a few more tweaks to the 3D printed parts before it will be ready for autonomous operation.

---

## Overview

The Variable Tube Cutter project provides:
- **Adjustability:** Quickly adapt the cutter to work with different tube lengths.
- **Simplicity:** The core functionality is controlled by a single source file in the `/src` folder.
- **3D Printability:** All custom parts are designed to be printed on filament printer, with the file available in the `/cad` folder.
- Built to help speed the production of [Champagne Skirts](https://www.champagneskirt.com/)
---

## Features

- **Adjustible Cutting Length:** Twist the rotary to adjsut the tube lenth want.
- **Adjustible Direction:** Press the center button on the PD-Stepper to change motor direction.
- **Adjustible Speed:** Press the left and right buttons on the PD-Stepper to change motor feed speed.
- **Visual Feedback:** When a limit (length or speed) is reached, LEDs on the motor and rotary flash red.
- **Persitent Settings:** Automatically loads last state on reboot.
---

## Hardware
  - [PD-Stepper](https://thingsbyjosh.com/products/pd-stepper): This is an all in one Nema17 motor, ESP32 micro controller and development board.
  - [Adafruit I2C Stemma QT Rotary Encoder](https://www.adafruit.com/product/5880): An easy to use rotary encoder via STEMMA.
  - 6 x [FR4-ZZ 1/4" x 5/8" x 0.196" Flanged Bearing](https://www.amazon.com/uxcell-Bearing-6-35x15-875x4-973mm-Shielded-Bearings/dp/B082PSLNJY)
  - 4 x [693ZZ Deep Groove Bearings Z2 3mm X 8mm X 4mm](https://www.amazon.com/uxcell-Groove-Bearings-Double-Shielded/dp/B0819B49JY)
  - TODO - Fastener List

### Software
  - A compatible IDE (Visual Studio Code with the PlatformIO extension works great).
  - [PlatformIO](https://platformio.org/) is recommended for building and uploading the firmware.
