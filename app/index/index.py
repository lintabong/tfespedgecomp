from flask import render_template, request, session, redirect, url_for, flash

from app.index import Index
from app import db, firebase_root_path


@Index.route('/', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']

        users = db.child(firebase_root_path).child('users').get().val()
        if users:
            for user_id, user_data in users.items():
                if user_data.get('name') == username and user_data.get('password') == password:
                    session['username'] = username
                    return redirect(url_for('admin.dashboard'))

        flash('Username atau password salah, coba lagi.')

    return render_template('login.html')
    # if request.method == 'POST':
    #     username = request.form.get('username')
    #     password = request.form.get('password')

    #     cursor = database.cursor()

    #     cursor.execute(f"""SELECT * from `users`
    #         WHERE username='{username}'
    #         AND password='{password}'""")
        
    #     user = cursor.fetchall()

    #     cursor.close()

    #     if len(user) <= 0:
    #         return render_template('index.html')

    #     session['username'] = username
    #     return redirect('/kirka-summary/')
    # return render_template('index.html')

@Index.route('/logout', methods=['GET'])
def logout():
    session['username'] = None
    return redirect('/')