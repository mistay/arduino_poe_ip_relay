# POE IP 8 Channel Relay Card
Device for switching relais with [power-over-ethernet](http://en.wikipedia.org/wiki/Power_over_Ethernet) (POE, 802.3af) with watchdog included based on Arduino PRO Mini.

## Description
This device enables generic loads (here: valve actuators) to be controlled by TCP/IP.

The power for the relay card and the Arduino Pro MINI is derived by a POE 802.3af ethernet switch.

A Texas Instruments TPS2375 chip is used to negotiate the power class (there are a couple of classes (4?) and the value of a resistor is used to tell the ethernet switch the requested class) and provides the needed power to the Arduino. Unfortunatly the ENC28J60 (Ethernet Chip) needs up to 250mA@5V ( = 1.25Watt) and cannot be driven at startup. As a workaround the power of the Ethernet module (ENC28J60) is delayed by a transistor (BD139) used as a switch.

3 LEDs indicate the status of this device:

- GREEN LED 1: blinks if the CPU (ATMega328 on Arduino PRO Mini) runs in normal state. Solid light or switched off LED if CPU stalled (something went wrong).
- GREEN LED 2: Blinks 2 times if an active transmission (via IP) is executed.
- RED LED: off for normal operation, solid on after watchdog triggered.

A LED test (where each LED is switched on and off again) is performed each time the Arduino boots.

## Watchdog
A peculiar feature of this device is the integrated software watchdog. The commands of the host controlling this device are tracked. If a timeout of 10 seconds is detected it causes the watchdog to fire and the RED LED (see above) switches on. All relay states switch off and the system changes it's state to 'emergency'.

In emergency state (RED LED on) the Arduino waits for another 100 seconds for a valid command to leave emergency state. If no command is decoded the ATMega328 CPU (on the Arduino PRO Mini) is beeing reset. This behaviour is desired to guarantee a new connection can be established (even if a stalled connection blocks a new one). Thus, a potential open tcp connection will be reset and the device will be ready to accept new tcp connections after the device resets successfully.

This mechanism is perfect for heating systems that try to keep track of the temperature flowing though the pipes by a non realtime operating system and/or non microcontroller ecosystem.

So in normal operation the controlling host is asked to establish at least one tcp connection (=command) in the space of 10 seconds.

## Block Diagram
<img width="600px" src="https://raw.githubusercontent.com/mistay/arduino_poe_ip_relay/master/media/blockdiagram.png" />

## Components
<img width="300px" src="https://raw.githubusercontent.com/mistay/arduino_poe_ip_relay/master/media/components.jpg" />

## POE

Have a look at the POE circuit (taken from [datasheet](http://www.ti.com/lit/ds/symlink/tps2375.pdf))

<img width="300px" src="https://raw.githubusercontent.com/mistay/arduino_poe_ip_relay/master/media/tsp_circuit.png" />

The Hanrun Pinout on the Ethernet module should look like this:

<img width="300px" src="https://raw.githubusercontent.com/mistay/arduino_poe_ip_relay/master/media/hanrun.jpg" />

A special Ethernet Jack is needed to get this up and running, I used a [Wurth 7499210124A](http://www.digikey.com/product-detail/en/7499210124A/732-4975-ND/4429048)

The connection to the ENC28J60 Ethernet module (RJ45 Hanrun HR91105a modular jacket) is done with this mapping:

Wurth  | Hanrun
------------- | -------------
PIN 1 | J1
PIN 2 | J2
PIN 3 | J3
PIN 6 | J6

## Device Foto

<img width="300px" src="https://raw.githubusercontent.com/mistay/arduino_poe_ip_relay/master/media/IMG_8039.JPG" />


## Communication Protocol

A TCP connection can be established to this device (tcp port 23, telnet). Only one client at the same time is able to connect due to restrictions of the ethernet library.

After the command is sent by the client it will be decoded and executed. The result of this operation is replied either by NOK (in case of failure) or by ACK (in the event of success). The TCP connection is immediately closed just as the reply was sent to the client.

### Command
A command looks like this:

Example: 05O1
This command sets D5 to Output (Arduino: pinMode(D5, OUTPUT); ) and sets pin State to 1 (Arduino: digitalWrite(D5, HIGH); )

In general: <PIN PIN><OPERATION><PARAMETER>
PIN: 0-9, A0, A1, A2, A3, A4, A5

OPERATION: O (sets Pin to Output), I (sets Pin to Input)

PARAMTER: 1 .. HIGH, 0 .. LOW


Please note: In this case a logical HIGH switches the relay OFF and a logical LOW would cause the relay to be switched ON!

Please note: There is no command for toggling the status of a relais but can be implemented very easily.


### Returns
Each command is acknowledged either by "NOK" or by "ACK"


## Usage Examples

xxPyyy, e.g. 05P085 - Port 05 PWM 085%
  
  xxOy,   e.g. 06O1   - Port 06 OUT on
  
  xxIy,   e.g. 07I    - Read Port 07 Digital Status
  




## Arduino Source Code
look at the [root](https://github.com/mistay/arduino_poe_ip_relay) directory

