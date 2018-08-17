import matplotlib, serial, time

ser = serial.Serial("COM13", 115200)

while 1:
    if( ser.inWaiting()>0 ):
        data = ser.readLine()
        data = data.decode()
        print(data)