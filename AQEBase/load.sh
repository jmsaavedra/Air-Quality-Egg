#!/bin/bash
sudo avrdude -p m328 -c avrispmkII -P usb -u -U efuse:w:0xfd:m
sleep 2
sudo avrdude -p m328 -c avrispmkII -P usb -u -U hfuse:w:0xde:m
sleep 2
sudo avrdude -p m328 -c avrispmkII -P usb -u -U lfuse:w:0xff:m
sleep 2
sudo avrdude -p m328 -c avrispmkII -e -P usb -U flash:w:eggbase_atmega328.hex

