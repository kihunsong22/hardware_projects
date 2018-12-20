import requests, json, datetime
from io import StringIO
import serial

# url = "https://dev-api.dimigo.in/dimibobs/today/"
url = "https://dev-api.dimigo.in/dimibobs/2018-12-20"
# print("시리얼 포트 번호: ")
# ser_port = input()
# print()
#
#
# def sendData(data):
#     data += "\r\n"
#     ser.write(data.encode())


while 1:
    response = requests.get(url)
    if(response.status_code == 200):
        break
    print("HTTP RESPONSE ERROR - " + str(response.status_code))

JSON = StringIO(response.text)
JSON = json.load(JSON)

print("JSON DATA: " + str(JSON) + "\n")
# print()


meal1 = JSON['breakfast']
meal2 = JSON['lunch']
meal3 = JSON['dinner']
meal4 = JSON['snack']
meal_date = JSON['date']

print("meal1: " + meal1)
print("meal2: " + meal2)
print("meal3: " + meal3)
print("meal4: " + meal4)
print("meal_date: " + meal_date)

msg = ""
now = datetime.datetime.now()
if now.hour<9:
    msg = meal1
elif now.hour<13:
    msg = meal2
elif now.hour<17:
    msg = meal3
else:
    msg = meal4

print("\n다음 급식: " + msg + "\n")

# ser = serial.Serial(ser_port, 115200)
#
# while 1:
#     sendData(msg)
#     time.sleep(5)
