import logging
import asyncio
import threading
import time
import sys

from assembly import *

from typing import Any

from bless import (  # type: ignore
        BlessServer,
        BlessGATTCharacteristic,
        GATTCharacteristicProperties,
        GATTAttributePermissions
        )

# logging.basicConfig(level=logging.DEBUG)
# logger = logging.getLogger(name=__name__)
trigger: threading.Event = threading.Event()


def read_request(
        characteristic: BlessGATTCharacteristic,
        **kwargs
        ) -> bytearray:
    # logger.debug(f"Reading {characteristic.value}")
    return characteristic.value


def write_request(
        characteristic: BlessGATTCharacteristic,
        value: Any,
        **kwargs
        ):
    characteristic.value = value
    # logger.debug(f"Char value set to {characteristic.value}")


async def run(loop, test):
    trigger.clear()
    # Instantiate the server
    my_service_name = "Test Service"
    server = BlessServer(name=my_service_name, loop=loop)
    server.read_request_func = read_request
    server.write_request_func = write_request

    # Add Service
    my_service_uuid = "A07498CA-AD5B-474E-940D-16F1FBE7E8CD"
    await server.add_new_service(my_service_uuid)

    # Add a Characteristic to the service
    my_char_uuid = "51FF12BB-3ED8-46E5-B4F9-D64E2FEC021B"
    char_flags = (
            GATTCharacteristicProperties.read |
            GATTCharacteristicProperties.write |
            GATTCharacteristicProperties.indicate |
            GATTCharacteristicProperties.notify
            )
    permissions = (
            GATTAttributePermissions.readable |
            GATTAttributePermissions.writeable
            )
    await server.add_new_characteristic(
            my_service_uuid,
            my_char_uuid,
            char_flags,
            None,
            permissions)
    
    # True to send custom messages; false for algorithm
    # test = True
    
    await server.start()
    # Actual movement loop
    if not test:
        # seq is path of magnets to move as follows:
        # Starting with painted facing south and reference top
        # Move 45 deg north
        # Move 45 deg north
        # Move 45 deg east
        # Move 45 deg east
        # Move 45 deg south
        # Move 45 deg south
        # NOTE: Currently doesn't work; think it's to do with test reading as not False
            # For testing, just do path manually? Best to figure out timings, regardless
        seq = ["S7", "S6", "S11", "S12", "S8", "S0"]
        c = server.get_characteristic(my_char_uuid)
        for val in seq:
            newval = bytes(val, 'UTF-8')
            c.value = newval
            server.update_value(my_service_uuid, my_char_uuid)
            time.sleep(2) # Time a movement takes in seconds (currently estimated)
        c.value = bytes("disconnect", 'UTF-8')
        server.update_value(my_service_uuid, my_char_uuid)
        await server.stop()
            
        """
        # Prep work for movement loop:
        # create starting board
        startboard = [[0,1,0,0,0],[0]*5,[0,0,0,1,0],[0,0,1,0,0],[1,0,0,1,0]]
        endboard = [ [0]*5, [0,0,1,0,0], [0,1,1,1,0], [0,0,1,0,0], [0]*5]
        # startboard = gen_board(5, 5, 5)
        
        # run assembly algorithm
        steplist = blackbox(startboard, endboard)
        # define error behavior?
        
        # Need to sync ids for bots and code somehow
        # Bots have local id, need to arrange where program assumes they'll be?
        # In which case, double check in which order we read bots and assign ids
        # earlier in the program
        # Could also make the boards with IDs in positions, rather than 1s
        
        # await server.start()
        val = input('Type \'start\' to begin the movement loop; \'exit\' to end server')
        while(val != 'exit'):
            if val == 'start':
                # Full movement loop, start to end
                for step in steplist:
                    server.get_characteristic(my_char_uuid).value = step
                    server.update_value(my_service_uuid, my_char_uuid)    
                    # Wait 3 seconds between loops
                    # replace with checking if movement was successful
                    time.sleep(3)
            else:
                print("Unknown command", val)
        await server.stop()
        """
    else:
        # Testing server functionality and updating characteristic values
        val = input('Type a message to write, or \'exit\' to end\n')
        while(val != 'exit'):
            newval = bytes(val, 'UTF-8')
            c = server.get_characteristic(my_char_uuid)
            c.value = newval
            server.update_value(my_service_uuid, my_char_uuid)
            val = input('Type \'exit\' to end the server\n')
        server.get_characteristic(my_char_uuid).value = bytes("disconnect", 'UTF-8')
        server.update_value(my_service_uuid, my_char_uuid)
        time.sleep(2)
        await server.stop()


def main():
    # Get first command line argument,
    test = sys.argv[1]
    # check if boolean, and pass into loop
    if type(test) == type(True):
        print("Bad command line argument")
        return
    
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(loop, test))
    
if __name__ == "__main__":
    main()


"""
This software taken from example code from the bless repository under the following license:


MIT License

Copyright (c) 2020 Kevin Davis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""