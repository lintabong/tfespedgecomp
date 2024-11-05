from flask import Flask, render_template, request, redirect, url_for, session, flash
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
    'messagingSenderId': os.getenv('FIREBASE_MESSAGING_SENDER_ID'),
    'appId': os.getenv('FIREBASE_APP_ID')
}

firebase_root_path = os.getenv('FIREBASE_BASE_FOLDER')
firebase = pyrebase.initialize_app(config)
db = firebase.database()

def create_app():
    app = Flask(__name__)
    app.secret_key = os.getenv('APP_SECRET_KEY')

    app.config['SESSION_PERMANENT'] = False
    app.config['SESSION_TYPE'] = 'filesystem'

    from app.index import Index
    app.register_blueprint(Index)

    from app.admin import Admin
    app.register_blueprint(Admin)

    return app
