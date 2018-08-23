import serial, time
import matplotlib.pyplot as plt
import numpy as np
from colourize import colourise, highlight

curTime = float('%0.2f' % float(time.time()))
baseTime =  float('%0.2f' % float(time.time()))
elapTime = 0.0
count = 0
lstX = []
lstY = []


def makeFig():
    plt.xlim([count - 2000, count])
    plt.ylim(0,600)
    plt.grid(True)
    plt.xlabel("Count")
    plt.ylabel("Voltage Value")

    plt.plot(lstX, lstY, 'b-', label="Voltage V")
    plt.legend(loc="upper left")

try:
    ser = serial.Serial("COM10", 115200)

    while 1:
        while(ser.inWaiting() == 0):
            pass

        data = ser.readline()
        data = data.decode()
        value = int(data[7:])
        print("elapTime: " + str(elapTime) + ", Value: " + str(value))
        count+=1

        if count>2000:
            lstX.pop(0)

        lstX.append(count)
        lstY.append(value)

        if( (float('%0.2f'%float(time.time())))-baseTime > 1):
            plt.plot(lstX, lstY, 'b-')
            plt.xlim([count/2, count])
            plt.ylim([0, 400])
            plt.grid(True)
            plt.xlabel("Count")
            plt.ylabel("Voltage Value")

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





# import serial, time
# import matplotlib.pyplot as plt
# import numpy as np
# from colourize import colourise, highlight
#
#
# try:
#     ser = serial.Serial("COM13", 115200)
#     curTime = float('%0.2f' % float(time.time()))
#     baseTime = float('%0.2f' % float(time.time()))
#
#     count = 0
#     lstX = []
#     lstY = []
#
#     while 1:
#         elapTime = float(('%0.2f'%float(time.time())))-curTime
#         while(ser.inWaiting() == 0):
#             pass
#
#         data = ser.readline()
#         data = data.decode()
#         value = int(data[7:])
#         print("elapTime: " + str(int(elapTime)) + ", Value: " + str(value))
#
#         # save value as list
#         count+=1
#         lstX.append(count)
#         lstY.append(value)
#
#         if(float( (float('%0.2f'%float(time.time())))-baseTime) > 1):
#             plt.plot(lstX, lstY, 'b-')
#             plt.xlim([count/2, count])
#             plt.ylim([0, 400])
#             plt.grid(True)
#             plt.xlabel("Count")
#             plt.ylabel("Voltage Value")
#
#             plt.show()
#
#             baseTime = float('%0.2f' % float(time.time()))
#             # ser.close()
#             # exit()
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
