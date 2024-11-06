import pyrebase
import os
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

data = {
    'name': 'admin',
    'password': 'admin',
}

db.child(firebase_root_path).child('users').push(data)

print('Data berhasil ditambahkan ke Firebase!')
