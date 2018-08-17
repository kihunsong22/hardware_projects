# -*- coding: utf-8 -*-

import time, serial, requests
import urllib

# ser = serial.Serial('COM8', 115200)

url = "https://api.dimigo.in/dimibobes/today/"

while 1:
    response = requests.get(url)
    if(response.status_code == 200):
        break
    print("DBG: HTTP response error")
    time.sleep(0.2)

response.encoding = "euc-kr"
data = response.text

print(data)

# while 1:
#     if(ser.inWaiting()>0):
#         data = ser.readline()
#         data = data.decode()
#         print(data)