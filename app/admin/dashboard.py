from flask import render_template, request, session, redirect, url_for
import random

from app.admin import Admin
from app import db, firebase_root_path

@Admin.route('/dashboard')
def dashboard():
    if 'username' in session:
        username = session['username']

        co2 = db.child(firebase_root_path).child('co2').get().val()
        co = db.child(firebase_root_path).child('co').get().val()
        dust = db.child(firebase_root_path).child('dust').get().val()
        air_quality = db.child(firebase_root_path).child('airQuality').get().val()

        return render_template(
            'dashboard.html', 
            username=username,
            co2=co2,
            co=co,
            dust=dust,
            air_quality=air_quality,
            data=[1,2,3,2,2],
            date=[3,4,5,6,7]
        )
    else:
        return redirect(url_for('login'))
