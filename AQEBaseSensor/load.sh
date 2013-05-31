#!/bin/bash
sudo rm temp.eep.hex
sudo avrdude -p m328 -c avrispmkII -P usb -U eeprom:r:temp.eep:i
sleep 2
sudo avrdude -p m328 -c avrispmkII -e -P usb -U flash:w:eggwiredbase_operational.hex
sleep 2
sudo avrdude -p m328 -c avrispmkII -P usb -U eeprom:w:temp.eep

