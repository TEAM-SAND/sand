# Gemstone '24 Team SAND

A repository of all code - bot, server, assembly, and assembly notebook - for Team SAND.

## Contents

This repository contains the total coding work of Team SAND, a group of undergraduate researchers working under the Gemstone Honors College of the University of Maryland, College Park. The team worked from formation in Spring 2021 to graduation in Spring 2024.

We wanted to create a prototype system of self-assembling robots with the primary function of, once massively downscaled, creating different structures capable of adapting to different tasks.

Some of these files are incomplete and non-functional, and many can be optimized beyond this rudimentary research.

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

There are two bot files. SAND_client_2.ino is an unfinished file for all functionality, and sandclient.ino is a finished file with reduced features for testing

The following instructions assume you are using Arduino IDE:

In Boards Manager, download esp32 by Espressif Systems and select ESP32C3 Dev Module as your board

Setup a port with a cable ending in USB-B for flashing

Edit sandclient.ino for your desired functionality. Look to line 33 for pin numbers, line 153 to setup pins, and line 195 for using said pins

Flash the chip. Enable serial monitor for debugging information if desired

NOTE: If you get an error about ringbuffer types during compilation, install the Arduino library ESP32 BLE Arduino by Dariusz Krempa, then go to your Arduino libraries folder and delete the folder that appears there. This is a known issue with the library, though documentation on resolving it is sparse

NOTE: This may work on other ESP chips, but has not been tested

### Assembly

Have assembly.py in the same folder as sandserver.py

For details on the assembly algorithm, see assembly.ipynb. The notebook contains a step-by-step chunking of each part of the code, explanatory comments, and an example run through each step.