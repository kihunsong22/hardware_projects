import serial, time
import numpy as np
# import matplotlib.pyplot as plt
from colourize import colourise, highlight
from drawnow import *

lstX, lstY, lstT = [], [], []
cnt, data = 0, 0
ard = serial.Serial('COM10', 115200)
startTime = float('%0.3f'% time.time())
baseTime = float('%0.3f'% time.time())


while cnt < 250:
    try:
        while ard.inWaiting() == 0:
            pass
        data = ard.readline()
        data = str(data.decode())
        data = data.split()
        lstX.append(int(data[0]))
        lstY.append(int(data[1]))

        interval = (time.time() - baseTime) * 1000
        lstT.append(interval)
        cnt += 1
        print("CNT: " + str(cnt) + "   DATA: [" + str(data[0]) + "], [" + str(data[1]) + "]" + "  T: " + str(lstT[-1]))

    except UnicodeDecodeError:
        error = str("ERROR_ UnicodeDecodeError - restarting")
        print(highlight("grey", colourise("black", error)))
        pass

    except TypeError:
        error = str("ERROR_ TypeError - restarting")
        print(highlight("grey", colourise("black", error)))
        pass

    except ValueError:
        error = str("ERROR_ ValueError")
        print(highlight("grey", colourise("black", error)))
        msg = str("DATA: " + str(data))
        print(highlight("grey", colourise("black", msg)))
        ard.close()
        break

    except Exception as e:
        error = str("ERROR_ " + str(e))
        print(highlight("grey", colourise("black", error)))
        # global data
        msg = str("DATA: " + str(data))
        print(highlight("grey", colourise("black", msg)))
        ard.close()
        break

    baseTime = float('%0.3f'% time.time())

ard.close()

lstT.pop(0)
interval = np.mean(lstT) # 데이터수집 간격 (ms)
totalElap = baseTime-startTime
SumAcX = np.sum(lstX)
SumAcY = np.sum(lstY)

print()
print("meanTime: " + str(interval) + "ms")
print("meanTime: " + str(totalElap) + "ms")
print("accX pos: " + str(SumAcX) + "")
print("accY pos: " + str(SumAcY) + "")

fig = plt.figure()
acc = fig.add_subplot(2, 1, 1)
inte = fig.add_subplot(2, 1, 2)

acc.grid(True)
acc.plot(lstX, label='AcX')
acc.plot(lstY, label='AcY')
acc.legend(loc='upper left')

inte.plot(lstT, label='interval')
inte.legend(loc='upper left')
# plt.title('MPU-6050')
plt.show()
