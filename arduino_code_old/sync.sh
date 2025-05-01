#!/bin/bash -xe

tty=/dev/ttyUSB0
stty -F $tty cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts -hupcl
sleep 3
echo "T$(date +%s | tr -d '\n')" >$tty
read reply <$tty
echo "$reply"
stty -F $tty hupcl

