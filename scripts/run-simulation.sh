#!/usr/bin/env bash
set -e

# Phase 5 (US3): Script to inject mock values into the i2c-stub device
# This should run INSIDE the Lima VM.

BUS=0 # Modify if i2c-stub binds to a different bus
ADDR=0x5A # CCS811

# Example: Simulate an eCO2 spike (e.g., to 1200 ppm)
# 1200 in Hex is 0x04B0 (High Byte: 0x04, Low Byte: 0xB0)
# 50 in Hex is 0x0032 (High Byte: 0x00, Low Byte: 0x32)

echo "Simulating normal air quality (eCO2: 450ppm, TVOC: 15ppb)"
sudo i2cset -y $BUS $ADDR 0x02 0x01
sudo i2cset -y $BUS $ADDR 0x03 0xC2
sudo i2cset -y $BUS $ADDR 0x04 0x00
sudo i2cset -y $BUS $ADDR 0x05 0x0F

sleep 5

echo "Simulating dangerous air quality spike (eCO2: 1200ppm, TVOC: 300ppb)"
sudo i2cset -y $BUS $ADDR 0x02 0x04
sudo i2cset -y $BUS $ADDR 0x03 0xB0
sudo i2cset -y $BUS $ADDR 0x04 0x01
sudo i2cset -y $BUS $ADDR 0x05 0x2C

sleep 5

echo "Simulating sensor hardware failure (clearing registers)"
# The kernel driver will read 0 and flag error or timeout if bus drops
# Alternatively, to simulate I2C bus loss, we'd remove i2c-stub:
# sudo rmmod i2c-stub
