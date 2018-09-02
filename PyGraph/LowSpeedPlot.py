import serial, time
from matplotlib import style
from colourize import colourise, highlight
from drawnow import *

ser = serial.Serial("COM10", 115200)

style.use('grayscale')
plt.ion()
count = 0
lst = []


def ardPlot():
    # plt.ylim(0, 1000)
    plt.grid(True)
    plt.plot(lst, 'ro-', label='voltage')


while 1:
    # try:
    while ser.inWaiting() == 0:
        pass
    global value

    value = ser.readline()
    value = int(value.decode())

    count += 1
    lst.append(value)
    print("Count: " + str(count) + ", Value: " + str(value))

    if count > 250:
        lst.pop(0)

    drawnow(ardPlot)
    plt.pause(0.000001)

    # except UnicodeDecodeError:
    #     error = str("ERROR_ UnicodeDecodeError - restarting")
    #     print(highlight("grey", colourise("black", error)))
    #     pass
    #
    # except ValueError:
    #     error = str("ERROR_ ValueError")
    #     print(highlight("grey", colourise("black", error)))
    #     msg = str("DATA: " + value.decode())
    #     print(highlight("grey", colourise("black", msg)))
    #     ser.close()
    #     break
    #
    # except Exception as e:
    #     error = str("ERROR_ " + str(e))
    #     print(highlight("grey", colourise("black", error)))
    #     ser.close()
    #     break
