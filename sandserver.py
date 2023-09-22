import logging
import asyncio
import threading
import time

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


# Convert this loop into a constant loop that just runs until stopped
async def run(loop):
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
    test = True
    
    await server.start()
    
    # Actual movement loop
    if not test:
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
    else:
        # Testing server functionality and updating characteristic values
        val = input('Type a message to write, or \'exit\' to end\n')
        while(val != 'exit'):
            newval = bytes(val, 'UTF-8')
            c = server.get_characteristic(my_char_uuid)
            c.value = newval
            server.update_value(my_service_uuid, my_char_uuid)
            val = input('Type \'exit\' to end the server\n')
        await server.stop()

loop = asyncio.get_event_loop()
loop.run_until_complete(run(loop))