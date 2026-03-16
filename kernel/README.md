# AQI Kernel Driver

Linux I2C character device driver for the CCS811 Air Quality sensor.

## Overview
- **Device Node**: `/dev/aqi_sensor` (Major 235 by default).
- **Protocol**: I2C (Address `0x5a`).
- **Interfaces**: 
    - `read()`: Returns `struct aqi_reading` (eCO2, TVOC).
    - `ioctl()`: Supports reset and force-measure commands.

## Build
```bash
make
sudo insmod aqi_sensor.ko
```

## Simulation
Tested using `i2c-stub` on Linux Kernel 6.8+ (Ubuntu 24.04).
