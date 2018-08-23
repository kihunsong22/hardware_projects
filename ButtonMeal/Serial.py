ser = serial.Serial('COM8', 115200)

while 1:
    if(ser.inWaiting()>0):
        data = ser.readline()
        data = data.decode()
        print(data)