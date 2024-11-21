import os
from flask import Flask, render_template, request

app = Flask(__name__)

# Path to the directory containing ISO files
ISO_FOLDER = os.path.join(os.path.dirname(__file__), 'isos')

@app.route('/select_iso', methods=['GET', 'POST'])
def select_iso():
    # List ISO files in the directory
    try:
        isos = [f for f in os.listdir(ISO_FOLDER) if f.endswith('.iso')]
    except FileNotFoundError:
        isos = []  # Fallback if the directory doesn't exist
    if request.method == 'POST':
        selected_iso = request.form.get('iso')
        # Respond with the selected ISO or take further actions
        return f'ISO "{selected_iso}" selected for VM creation.'
    return render_template('select_iso.html', isos=isos)

if __name__ == "__main__":
    app.run(debug=True)
