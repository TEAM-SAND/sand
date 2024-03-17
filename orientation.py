import serial
import time
from quaternion import Quaternion
import numpy as np


def check_orientation(data):
    # Set reference frame for calcs 
    x_axis_unit = (1, 0, 0)
    y_axis_unit = (0, 1, 0)
    z_axis_unit = (0, 0, 1)

    # if this is performed as the initial movement, choose which 2 axes you will be looking at initially
    # those will be your initial vectors. otherwise use vector obtained previously
    # set these vectors to be v1 and v2. If this isnt the initial movment, then set v1 and v2 to be the
    # 2 vectors from the vect variable.

    r1 = Quaternion.from_axisangle(np.pi / 2, x_axis_unit)
    r2 = Quaternion.from_axisangle(np.pi / 2, y_axis_unit)
    r3 = Quaternion.from_axisangle(np.pi / 2, z_axis_unit)

    # Parse data to get Quaternion
    # r = parse data for r
    # i = parse data for i
    # j = parse data for j
    # k = parse data for k
    # quat = Quaternion.from_value(np.array([r, i, j, k]))

    # Expected values
    # If rotation will be on x axis and one of the 2 chosen vectors is y:
        # ev1 = r1 * v1
        # ev2 = r1 * v2
    # omitting other if statments, but will do other combinations of possibilities

    # Actual vectors
    # av1 = quat * v1
    # av2 = quat * v2

    # Check if ev1 and ev2 are within an acceptable margin of error of each other
    # if not:
        # amount_over_rotated = 
        # print("bot over rotated!")
        # return or throw error


    # assuming all went smoothly
    # set vect = (av1, av2)
    return


# Open serial port
ser = serial.Serial('COMX', 115200)  # Adjust 'X' to match the Arduino's port
prev_data = ""
vects = ([], [])
try:
    while True:
        # Read data from serial port
        # should be new data
        data = ser.readline().decode().strip()

        # not sure if necessary but in case it needs to do this
        # to get the last printed line. otherwise remove this
        # while ser.in_waiting > 0:
        #     data = ser.readline().decode().strip()
        print("Received:", data)

        # Check if Arduino sent the command to call Python function
        # data = "CALL_PYTHON_FUNCTION" + quaternion data
        if "CALL_PYTHON_FUNCTION" in data and prev_data != data:
            check_orientation(data)  # Call Python function
            prev_data = data
except KeyboardInterrupt:
    ser.close()  # Close the serial port when program exits
