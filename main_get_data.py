import os
import csv
import pyrebase
from dotenv import load_dotenv

load_dotenv()

config = {
    'apiKey': os.getenv('FIREBASE_API_KEY'),
    'authDomain': os.getenv('FIREBASE_AUTH_DOMAIN'),
    'databaseURL': os.getenv('FIREBASE_DATABASE_URL'),
    'projectId': os.getenv('FIREBASE_PROJECT_ID'),
    'storageBucket': os.getenv('FIREBASE_STORAGE_BUCKET'),
    'messagingSenderId': os.getenv('FIREBASE_APP_ID'),
    'appId': os.getenv('FIREBASE_APP_ID')
}

firebase_root_path = os.getenv('FIREBASE_BASE_FOLDER')

firebase = pyrebase.initialize_app(config)
db = firebase.database()

list_data = db.child(firebase_root_path + '/history').get().val()

required_keys = ['dust', 'epoch', 'humidity', 'mq135', 'mq7', 'pressure', 'temperature']

csv_file = 'sensor_data.csv'

# for day in list_data:
#     for time in list_data[day].keys():
#         print(list_data[day][time])

with open(csv_file, mode='w', newline='') as file:
    writer = csv.DictWriter(file, fieldnames=required_keys)

    writer.writeheader()

    for day in list_data:
        for time in list_data[day].keys():
            data = list_data[day][time]

            if all(key in data for key in required_keys):
                writer.writerow(data)
