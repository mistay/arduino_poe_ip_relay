# POE IP 8 Channel Relay Card
Device for switching relais with [power-over-ethernet](http://en.wikipedia.org/wiki/Power_over_Ethernet) (POE, 802.3af) with watchdog included based on Arduino PRO Mini.

## Description
Often a generic load (here: valve actuators) need to be controlled by IP (here: TCP).

The power for the relay card and the Arduino Pro MINI is derived by a POE 802.3af ethernet switch.

A Texas Instruments TPS2375 chip is used to negotiate the power class of this device and provides the needed power to the Arduino. Unfortunatly the ENC28J60 (Ethernet Chip) needs up to 250mA@5V ( = 1.25Watt) and cannot be driven at startup. As workaround the ENC28J60 Ethernet module is switched on after the voltage for the Arduino is released by the TI POE chip. Therefore a BD139 transitor is needed to switch the Ethernet module on after a delay of approx. 2-3 seconds.

3 LEDs show the status of this device:

- GREEN LED 1: blinks if cpu works correctly. Solid light or switched off LED if software hangs.
- GREEN LED 2: indicates an active transmission (via IP). Blinks two times if a command is executed.
- RED LED: off on normal operation, solid on if watchdog triggered.

## Watchdog


## Block Diagram
<img width="300px" src="https://raw.githubusercontent.com/mistay/arduino_poe_ip_relay/master/media/blockdiagram.png" />

## Components
<img width="300px" src="https://raw.githubusercontent.com/mistay/arduino_poe_ip_relay/master/media/components.jpg" />

## Arduino Source Code
look at [arduino_poe_ip_relay/](https://github.com/mistay/arduino_poe_ip_relay/tree/master/arduino_poe_ip_relay) directory

