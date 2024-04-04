import serial
import time
from quaternion import Quaternion, normalize
import numpy as np
import math

def closest(av1, av2, odd, even):
    # distances = []
    min_i = 0

    for i in range(len(odd)):
        ev1 = odd[i]
        ev2 = even[i]
        if abs(av1[0] - ev1[0]) <= .1 and abs(av1[1] - ev1[1]) <= .1 and abs(av1[2] - ev1[2]) <= .1 and abs(av2[0] - ev2[0]) <= .1 and abs(av2[1] - ev2[1]) <= .1 and abs(av2[2] - ev2[2]) <= .1:
            min_i = i
        # distance1 = np.sum((av1 - ev1) ** 2)
        # distance2 = np.sum((av2 - ev2) ** 2)
        # total_distance = distance1 + distance2
        # distances.append(total_distance)
    # print("actual: ", av1, av2)
    # print("vectors: ", odd, even)
    # print("index: ", )

    # Find the index of the minimum total distance
    # min_index = np.argmin(distances)

    return min_i

def check_orientation(data, prev_vects, init):
    av1 = []
    av2 = []

    ex1 = []
    ex2 = []

    q_arr = data.split(" ")
    # Set reference frame for calcs 
    x_axis_unit = (1, 0, 0)
    y_axis_unit = (0, 1, 0)
    z_axis_unit = (0, 0, 1)


    r1 = Quaternion.from_axisangle(np.pi / 2, x_axis_unit)
    nr1 = Quaternion.from_axisangle(np.negative(np.pi) / 2, x_axis_unit)
    r2 = Quaternion.from_axisangle(np.pi / 2, y_axis_unit)
    nr2 = Quaternion.from_axisangle(np.negative(np.pi) / 2, y_axis_unit)
    r3 = Quaternion.from_axisangle(np.pi / 2, z_axis_unit)
    nr3 = Quaternion.from_axisangle(np.negative(np.pi) / 2, z_axis_unit)

    z1 = Quaternion.from_axisangle(0, x_axis_unit)
    z2 = Quaternion.from_axisangle(0, y_axis_unit)
    z3 = Quaternion.from_axisangle(0, z_axis_unit)

    # Parse data to get Quaternion
    r = float(q_arr[1])
    i = float(q_arr[3])
    j = float(q_arr[5])
    k = float(q_arr[7])
    vec = normalize(np.array([i, j, k]))
    quat = Quaternion.from_value(normalize(np.array((r, i, j, k))))


    # yaw = float(q_arr[9])
    # pitch = float(q_arr[11])
    # roll = float(q_arr[13])

    # prev_vects[0] = normalize(prev_vects[0])
    # prev_vects[1] = normalize(prev_vects[1])
    av1 = quat * prev_vects[0]
    av2 = quat * prev_vects[1]

    
    # Expected values
    # check if rotated 90 degrees. 0.015 radian margin of error
    if abs(quat.get_axisangle()[0] % (np.pi / 2)) <= 0.02 or abs(quat.get_axisangle()[0] % np.negative(np.pi / 2)) <= 0.02:
        print("Actual vectors: ")
        print("av1: ", av1)
        print("av2: ", av2)
        

        if not init:
            # If rotated on x axis, values should be close to actual
            ev1 = r1 * prev_vects[0]
            ev2 = r1 * prev_vects[1]
            ev1 = list(map(lambda x: round(x), ev1))
            ev2 = list(map(lambda x: round(x), ev2))
            # print("ev1: ", ev1)
            # print("ev2: ", ev2)
            
            # If rotated on y axis, values should be close to actual
            ev3 = r2 * prev_vects[0]
            ev4 = r2 * prev_vects[1]
            ev3 = list(map(lambda x: round(x), ev3))
            ev4 = list(map(lambda x: round(x), ev4))
            # print("ev3: ", ev3)
            # print("ev4: ", ev4)

            # If rotated on z axis, values should be close to actual
            ev5 = r3 * prev_vects[0]
            ev6 = r3 * prev_vects[1]
            ev5 = list(map(lambda x: round(x), ev5))
            ev6 = list(map(lambda x: round(x), ev6))
            # print("ev5: ", ev5)
            # print("ev6: ", ev6)

            ev7 = nr1 * prev_vects[0]
            ev8 = nr1 * prev_vects[1]
            ev7 = list(map(lambda x: round(x), ev7))
            ev8 = list(map(lambda x: round(x), ev8))
            # print("ev7: ", ev7)
            # print("ev8: ", ev8)

            ev9 = nr2 * prev_vects[0]
            ev10 = nr2 * prev_vects[1]
            ev9 = list(map(lambda x: round(x), ev9))
            ev10 = list(map(lambda x: round(x), ev10))
            # print("ev9: ", ev9)
            # print("ev10: ", ev10)

            ev11 = nr3 * prev_vects[0]
            ev12 = nr3 * prev_vects[1]
            ev11 = list(map(lambda x: round(x), ev11))
            ev12 = list(map(lambda x: round(x), ev12))
            # print("ev11: ", ev11)
            # print("ev12: ", ev12)

            ev13 = z1 * prev_vects[0]
            ev14 = z1 * prev_vects[1]
            ev13 = list(map(lambda x: round(x), ev13))
            ev14 = list(map(lambda x: round(x), ev14))
            # print("ev13: ", ev13)
            # print("ev14: ", ev14)

            ev15 = z2 * prev_vects[0]
            ev16 = z2 * prev_vects[1]
            ev15 = list(map(lambda x: round(x), ev15))
            ev16 = list(map(lambda x: round(x), ev16))
            # print("ev13: ", ev15)
            # print("ev14: ", ev16)

            ev17 = z3 * prev_vects[0]
            ev18 = z3 * prev_vects[1]
            ev17 = list(map(lambda x: round(x), ev17))
            ev18 = list(map(lambda x: round(x), ev18))
            # print("ev13: ", ev17)
            # print("ev14: ", ev18)

            ev_odd = [ev1, ev3, ev5, ev7, ev9, ev11, ev13]
            ev_even = [ev2, ev4, ev6, ev8, ev10, ev12, ev14]
            index = closest(av1, av2, ev_odd, ev_even)
            
            ex1 = ev_odd[index]
            ex2 = ev_even[index]

            ex1 = list(map(lambda x: round(x), ex1))
            ex2 = list(map(lambda x: round(x), ex2))

            av1 = list(map(lambda x: round(x), av1))
            av2 = list(map(lambda x: round(x), av2))
        else:
            av1 = list(map(lambda x: round(x), av1))
            av2 = list(map(lambda x: round(x), av2))

            ex1 = av1
            ex2 = av2
        
        prev_vects = [ex1, ex2]
        print("Expected vectors: ")
        print("Point 1: ", ex1)
        print("Point 2: ", ex2)

        

    else:
        # print("radian angle: ", quat.get_axisangle()[0])
        # print("mod: ", quat.get_axisangle()[0] % np.pi / 2)
        print("BAD! rotation difference: ", round(math.degrees(abs(quat.get_axisangle()[0] - np.pi / 2)), 2), " degrees")


        if not init:
            # If rotated on x axis, values should be close to actual
            ev1 = r1 * prev_vects[0]
            ev2 = r1 * prev_vects[1]
            ev1 = list(map(lambda x: round(x), ev1))
            ev2 = list(map(lambda x: round(x), ev2))
            # print("ev1: ", ev1)
            # print("ev2: ", ev2)
            
            # If rotated on y axis, values should be close to actual
            ev3 = r2 * prev_vects[0]
            ev4 = r2 * prev_vects[1]
            ev3 = list(map(lambda x: round(x), ev3))
            ev4 = list(map(lambda x: round(x), ev4))
            # print("ev3: ", ev3)
            # print("ev4: ", ev4)

            # If rotated on z axis, values should be close to actual
            ev5 = r3 * prev_vects[0]
            ev6 = r3 * prev_vects[1]
            ev5 = list(map(lambda x: round(x), ev5))
            ev6 = list(map(lambda x: round(x), ev6))
            # print("ev5: ", ev5)
            # print("ev6: ", ev6)

            ev7 = nr1 * prev_vects[0]
            ev8 = nr1 * prev_vects[1]
            ev7 = list(map(lambda x: round(x), ev7))
            ev8 = list(map(lambda x: round(x), ev8))
            # print("ev7: ", ev7)
            # print("ev8: ", ev8)

            ev9 = nr2 * prev_vects[0]
            ev10 = nr2 * prev_vects[1]
            ev9 = list(map(lambda x: round(x), ev9))
            ev10 = list(map(lambda x: round(x), ev10))
            # print("ev9: ", ev9)
            # print("ev10: ", ev10)

            ev11 = nr3 * prev_vects[0]
            ev12 = nr3 * prev_vects[1]
            ev11 = list(map(lambda x: round(x), ev11))
            ev12 = list(map(lambda x: round(x), ev12))
            # print("ev11: ", ev11)
            # print("ev12: ", ev12)

            ev13 = z1 * prev_vects[0]
            ev14 = z1 * prev_vects[1]
            ev13 = list(map(lambda x: round(x), ev13))
            ev14 = list(map(lambda x: round(x), ev14))
            # print("ev13: ", ev13)
            # print("ev14: ", ev14)

            ev15 = z2 * prev_vects[0]
            ev16 = z2 * prev_vects[1]
            ev15 = list(map(lambda x: round(x), ev15))
            ev16 = list(map(lambda x: round(x), ev16))
            # print("ev13: ", ev15)
            # print("ev14: ", ev16)

            ev17 = z3 * prev_vects[0]
            ev18 = z3 * prev_vects[1]
            ev17 = list(map(lambda x: round(x), ev17))
            ev18 = list(map(lambda x: round(x), ev18))
            # print("ev13: ", ev17)
            # print("ev14: ", ev18)

            ev_odd = [ev1, ev3, ev5, ev7, ev9, ev11, ev13]
            ev_even = [ev2, ev4, ev6, ev8, ev10, ev12, ev14]
            index = closest(av1, av2, ev_odd, ev_even)
            
            ex1 = normalize(ev_odd[index])
            ex2 = normalize(ev_even[index])

            ex1 = list(map(lambda x: round(x), ex1))
            ex2 = list(map(lambda x: round(x), ex2))

            av1 = list(map(lambda x: round(x), av1))
            av2 = list(map(lambda x: round(x), av2))
        else:
            av1 = list(map(lambda x: round(x), av1))
            av2 = list(map(lambda x: round(x), av2))

            ex1 = av1
            ex2 = av2


        prev_vects = [ex1, ex2]
        

    # assuming all went smoothly
    # prev_vects = [av1, av2]
    return prev_vects


# Open serial port
ser = serial.Serial('COM5', 115200)

# intial vectors will be on the x and y axis
prev_vects = [[0,1,0], [0,0,1]]
init = True
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

        if "r:" in data:
            prev_vects = check_orientation(data, prev_vects, init)  # Call Python function
            init = False
            
except KeyboardInterrupt:
    ser.close()  # Close the serial port when program exits
