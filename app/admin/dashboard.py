from flask import render_template, request, session, redirect, url_for
import random

from app.admin import Admin
from app import db, firebase_root_path

@Admin.route('/dashboard')
def dashboard():
    if 'username' in session:
        username = session['username']

        required_keys = ['dust', 'epoch', 'humidity', 'mq135', 'mq7', 'pressure', 'temperature']

        humidities = []
        list_mq7 = []
        list_mq135 = []
        list_dust = []
        times = []

        selected_date = request.args.get('date')
        if selected_date:
            list_data = db.child(firebase_root_path + '/history').get().val()

            if selected_date in list_data.keys():
                for time in list_data[selected_date].keys():
                    data = list_data[selected_date][time]

                    if all(key in data for key in required_keys):
                        list_mq7.append(data['mq7'])
                        list_mq135.append(data['mq135'])
                        list_dust.append(data['dust'])
                        humidities.append(int(data['humidity']))

        mq7 = db.child(firebase_root_path).child('mq7').get().val()
        mq135 = db.child(firebase_root_path).child('mq135').get().val()
        dust = db.child(firebase_root_path).child('dust').get().val()
        humidity = db.child(firebase_root_path).child('humidity').get().val()

        return render_template(
            'dashboard.html', 
            username=username,
            mq7=mq7,
            mq135=mq135,
            dust=dust,
            humidity=humidity,
            humidities=humidities,
            list_mq7=list_mq7,
            list_mq135=list_mq135,
            list_dust=list_dust,
            len_data=[i for i in range(len(humidities))]
        )
    else:
        return redirect(url_for('login'))
