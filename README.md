# NOCT — Nocturnal Off-road Cave Traverser

A remotely operated mecanum wheel rover designed for cave exploration and confined space rescue scenarios. Built as part of IY463 (Research, Design and Development).

## Hardware
- ESP32-S3 (motor control + WebSocket server)
- Raspberry Pi Zero 2W + Camera Module 3 NoIR (FPV streaming)
- 2× L298N motor drivers
- Mecanum wheels (omnidirectional 6-DOF movement)
- 7.4V 2600mAh LiPo battery

## Structure
- `firmware/` — ESP32-S3 Arduino firmware
- `streaming/` — Raspberry Pi camera streaming server
- `extensions/` — Planned features

## Features
- WebSocket real-time motor control
- 6-DOF movement
- MJPEG FPV video stream
- systemd auto-start on boot