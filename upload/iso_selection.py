import os
from flask import Flask, render_template, request

app = Flask(__name__)

ISO_FOLDER = '../src'

@app.route('/select_iso', methods=['GET', 'POST'])
def select_iso():
    isos = os.listdir(ISO_FOLDER)
    if request.method == 'POST':
        selected_iso = request.form.get('iso')
        # Proceed to VM creation with selected_iso
        return f'ISO {selected_iso} selected for VM creation'
    return render_template('select_iso.html', isos=isos)
