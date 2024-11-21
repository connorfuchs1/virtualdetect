import os
import requests
from flask import Flask, render_template, request

app = Flask(__name__)

# Directory to store downloaded ISOs
ISO_FOLDER = os.path.expanduser('~/virtualdetect/iso')
os.makedirs(ISO_FOLDER, exist_ok=True)

# Predefined ISO options with download URLs
ISO_OPTIONS = {
    "Ubuntu 24.04": "https://releases.ubuntu.com/24.04/ubuntu-24.04-desktop-amd64.iso",
    "Debian 12": "https://cdimage.debian.org/debian-cd/current/amd64/iso-cd/debian-12.0.0-amd64-netinst.iso"
}

@app.route('/select_iso', methods=['GET', 'POST'])
def select_iso():
    if request.method == 'POST':
        selected_iso = request.form.get('iso')
        iso_url = ISO_OPTIONS.get(selected_iso)

        if iso_url:
            iso_path = os.path.join(ISO_FOLDER, f"{selected_iso.replace(' ', '_')}.iso")
            try:
                response = requests.get(iso_url, stream=True)
                response.raise_for_status()
                with open(iso_path, 'wb') as iso_file:
                    for chunk in response.iter_content(chunk_size=8192):
                        iso_file.write(chunk)
                return f'ISO "{selected_iso}" has been downloaded to {iso_path}.'
            except requests.exceptions.RequestException as e:
                return f"Error downloading ISO: {e}"
        return "Invalid ISO selection."

    # Debugging: Print ISO_OPTIONS to console
    print("ISO_OPTIONS being passed to the template:", ISO_OPTIONS)

    return render_template('select_iso.html', iso_options=ISO_OPTIONS)

     

if __name__ == "__main__":
    app.run(debug=True)
