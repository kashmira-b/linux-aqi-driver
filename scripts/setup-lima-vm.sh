#!/usr/bin/env bash
set -e

echo "Setting up Lima VM for AQI Driver Simulation..."

# Start the VM
limactl start --name=aqi-dev template://ubuntu-lts

# Install required kernel headers and I2C dev tools
limactl shell aqi-dev sudo apt update
limactl shell aqi-dev sudo apt install -y build-essential linux-headers-\$(uname -r) i2c-tools

echo "Lima VM is ready!"
echo "To enter the VM: limactl shell aqi-dev"
