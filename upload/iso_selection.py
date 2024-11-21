import os
import requests
import threading
from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

# Directory to store downloaded ISOs
ISO_FOLDER = os.path.expanduser('~/virtualdetect/iso')
os.makedirs(ISO_FOLDER, exist_ok=True)

# Predefined ISO options with download URLs
ISO_OPTIONS = {
    "Ubuntu 24.04": "https://releases.ubuntu.com/noble/ubuntu-24.04.1-desktop-amd64.iso",
    "Debian 12": "https://cdimage.debian.org/debian-cd/current/amd64/iso-cd/debian-12.8.0-amd64-netinst.iso"
}

# Dictionary to track progress
progress = {"percentage": 0}

def download_file(url, iso_path, progress_dict):
    try:
        response = requests.get(url, stream=True)
        response.raise_for_status()
        total_length = int(response.headers.get('content-length', 0))
        downloaded = 0

        print(f"Starting download: {url}")
        with open(iso_path, 'wb') as iso_file:
            for chunk in response.iter_content(chunk_size=8192):
                if chunk:
                    iso_file.write(chunk)
                    downloaded += len(chunk)
                    progress_dict["percentage"] = int(100 * downloaded / total_length)

        progress_dict["percentage"] = 100
        print(f"Download complete: {iso_path}")
    except requests.exceptions.RequestException as e:
        print(f"Error downloading ISO: {e}")
        progress_dict["error"] = str(e)

@app.route('/', methods=['GET', 'POST'])
def select_iso():
    return render_template('select_iso.html', iso_options=ISO_OPTIONS)

@app.route('/download', methods=['POST'])
def download_iso():
    selected_iso = request.json.get('iso')
    iso_url = ISO_OPTIONS.get(selected_iso)

    if not iso_url:
        return jsonify({"error": "Invalid ISO selection"}), 400

    # Construct the ISO file path
    iso_filename = f"{selected_iso.replace(' ', '_')}.iso"
    iso_path = os.path.join(ISO_FOLDER, iso_filename)

    #If our iso already exists, lets not redownload
    if os.path.exists(iso_path):
        message = f'ISO {selected_iso} already exists at {iso_path}'
        return jsonify({"message" : message, "already_downloaded" : True})
    

    # Reset progress for new download
    progress["percentage"] = 0
    progress.pop("error", None)  # Remove any previous errors


    # Start download in a new thread
    download_thread = threading.Thread(target=download_file, args=(iso_url, iso_path, progress))
    download_thread.start()

    # Return immediately
    return jsonify({"message": f'Started downloading ISO "{selected_iso}" to {iso_path}'})

@app.route('/progress', methods=['GET'])
def get_progress():
    print(f"Progress reported: {progress['percentage']}%")
    return jsonify(progress)

if __name__ == "__main__":
    app.run(debug=True, threaded=True, host='0.0.0.0', port='1337')