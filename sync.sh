#!/bin/bash
stty -F /dev/ttyUSB0 9600
echo "T$(date +%s | tr -d '\n')" > /dev/ttyUSB0
