import serial, time
import matplotlib.pyplot as plt
import numpy as np
from colourize import colourise, highlight


try:
    ser = serial.Serial("COM13", 115200)
    curTime = float('%0.2f' % float(time.time()))
    baseTime = float('%0.2f' % float(time.time()))

    count = 0
    lstX = []
    lstY = []

    plt.title("ArdPLOT")
    plt.xlabel("Count")
    plt.ylabel("Voltage value")
    plt.grid(True)

    while 1:
        elapSec = float(('%0.2f'%float(time.time())))-baseTime
        while(ser.inWaiting() == 0):
            pass

        data = ser.readline()
        data = data.decode()
        value = int(data[7:])
        print("elapSec: " + str(int(elapSec)) + ", Value: " + str(value))

        # save value as list
        count+=1
        lstX.append(count)
        lstY.append(value)

        if(elapSec > 0.5 ):
            plt.plot(lstX, lstY, 'b-')
            plt.xlim([count/2, count])
            plt.ylim([0, 400])

            plt.show()

            baseTime = float('%0.2f' % float(time.time()))
            # ser.close()
            # exit()

except Exception as e:
    error = str("ERROR_ " + str(e))
    print(highlight("grey", colourise("black", error)))

    try:
        ser.close()
    except:
        error = str("ERROR_ NO SERIAL OPENED")
        print(highlight("grey", colourise("black", error)))





# import matplotlib, serial, time
# import matplotlib.pyplot as plt
# from numpy import *
# from colourize import colourise, highlight
#
#
# try:
#     ser = serial.Serial("COM13", 115200)
#     curTime = float('%0.2f' % float(time.time()))
#
#     lstX = []
#     lstY = []
#
#     plt.ion() # enable animation
#     fig = plt.figure()
#     sf = fig.add_subplot(1, 1, 1)
#     plt.xlim([0, 60])
#     plt.ylim([0, 500])
#     line1, = sf.plot(lstX, lstY, 'r-')
#
#     while 1:
#         elapSec = float(('%0.2f'%float(time.time())))-curTime
#
#         if(ser.inWaiting()>0):
#             data = ser.readline()
#             data = data.decode()
#             value = int(data[7:])
#             print("elapSec: " + str(int(elapSec)) + ", Value: " + str(value))
#
#             # save value as list
#             lstX.append(value)
#             lstY.append(elapSec)
#
#             # append to graph
#             line1.set_xdata(lstX)
#             line1.set_ydata(lstY)
#
#             plt.draw()
#
#         if(elapSec > 15 ):
#             ser.close()
#             exit()
#
# except Exception as e:
#     error = str("ERROR_ " + str(e))
#     print(highlight("grey", colourise("black", error)))
#
#     try:
#         ser.close()
#     except:
#         error = str("ERROR_ NO SERIAL OPENED")
#         print(highlight("grey", colourise("black", error)))
