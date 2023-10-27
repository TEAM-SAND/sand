# Gemstone '24 Team SAND

A repository of all code - bot, server, assembly, and assembly notebook - for Team SAND.

## Installation and Setup

### Server

Ensure the computer you want to run the server on supports Bluetooth Low Energy

sandserver.py relies on the bless Python package, which can be installed with 
```bash
pip install bless
```

Enable Bluetooth in your computer's settings

Run 
```bash 
python sandserver.py <arg>
``` 
in the directory where sandserver.py AND assembly.py are located. If \<arg> is True, then the command line will loop writing user input to the server until given 'exit'. If \<arg> is False, it will use attempt to use assembly.py, but this functionality is not currently implemented

### Bot

There are two bot files. SAND_client_2.ino is an unfinished file for all functionality, and sandclient.ino is a finished file with reduced features for testing.

The following instructions assume you are using Arduino IDE:

In Boards Manager, download esp32 by Espressif Systems and select ESP32C3 Dev Module as your board

Setup a port with a cable ending in USB-B for flashing

Edit sandclient.ino for your desired functionality. Look to line 19 for pin numbers, line 139 to setup pins, and line 181 for using said pins

Flash the chip. Enable serial monitor for debugging information if desired

NOTE: If you get an error about ringbuffer types during compilation, install the Arduino library ESP32 BLE Arduino by Dariusz Krempa, then go to your Arduino libraries folder and delete the folder that appears there.

## TODO: Add description, credits, license, and other fancy stuff