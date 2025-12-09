#!/bin/bash
echo "Installing dependencies for testing..."

# Для Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y g++ make libssl-dev libcrypto++-dev libunittest++-dev

echo "Dependencies installed!"
