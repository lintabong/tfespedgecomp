import os
import csv
import pyrebase
from datetime import datetime, timedelta
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

DATE = '2024-11-10'
TIME_START = '00:00:00'
TIME_END = '23:00:00'

firebase_root_path = os.getenv('FIREBASE_BASE_FOLDER')

firebase = pyrebase.initialize_app(config)
db = firebase.database()

list_data = db.child(f'{firebase_root_path}/history/{DATE}').get().val()

required_keys = ['dust', 'date', 'epoch', 'humidity', 'mq135', 'mq7', 'pressure', 'temperature', 'voc']

csv_file = 'sensor_data.csv'

time_start = int((datetime.strptime(f'{DATE} {TIME_START}', '%Y-%m-%d %H:%M:%S')).timestamp())
time_end = int((datetime.strptime(f'{DATE} {TIME_END}', '%Y-%m-%d %H:%M:%S')).timestamp())

with open(csv_file, mode='w', newline='') as file:
    writer = csv.DictWriter(file, fieldnames=required_keys)
    writer.writeheader()

    for row in list_data:
        list_data[row]['date'] = None
        if all(key in list_data[row].keys() for key in required_keys):
            list_data[row]['date'] = datetime.fromtimestamp(int(list_data[row]['epoch']))
            if int(list_data[row]['epoch']) < time_end and int(list_data[row]['epoch']) > time_start:
                print(list_data[row])
                writer.writerow(list_data[row])
