import serial, time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
from colourize import colourise, highlight

ser = serial.Serial("COM10", 115200)
startTime = float('%0.2f' % float(time.time()))
elapTime = 0.0
count = 0
lst = []

style.use('fivethirtyeight')
fig = plt.figure()
gr1 = fig.add_subplot(1, 1, 1)


def ardPlot():
    gr1.xlim([count - 400, count])
    gr1.ylim(0,1000)
    gr1.grid(True)
    # gr1.ylabel("Voltage Value")
    # gr1.legend(loc="upper left")

    gr1.clear()
    gr1.plot(lst, 'b-', label="Voltage V")
    gr1.pause(0.00001)


ani = animation.FuncAnimation(fig, ardPlot, interval=200)
plt.show()

while 1:
    try:
        while(ser.inWaiting() == 0):
            time.sleep(5)

        data = ser.readline()
        data = data.decode()
        value = int(data[7:])
        elapTime = ( float('%0.2f' % float(time.time()))-startTime )
        print("elapTime: " + str(elapTime) + ", Value: " + str(value))

        count += 1
        if count > 400:
            lst.pop(0)

        lst.append(value)

    except UnicodeDecodeError:
        error = str("ERROR_ UnicodeDecodeError - restarting")
        print(highlight("grey", colourise("black", error)))
        pass


    except Exception as e:
        error = str("ERROR_ " + str(e))
        print(highlight("grey", colourise("black", error)))

        ser.close()
        break