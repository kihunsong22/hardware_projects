import requests, json
from io import StringIO

url = "https://dev-api.dimigo.in/dimibobs/today/"


while 1:
    response = requests.get(url)
    if(response.status_code == 200):
        break
    print("HTTP RESPONSE ERROR - " + str(response.status_code))

JSON = StringIO(response.text)
JSON = json.load(JSON)

print("JSON DATA: " + str(JSON))
print()


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