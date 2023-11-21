from flask import Flask, render_template, request, redirect
import cv2
import youtube_dl

app = Flask(__name__)

correct_code = '9508'
code = ''

# Initialisatie van de camera's (vervang de '0', '1', '2', '3' door de juiste camera-indices)
camera1 = cv2.VideoCapture(0)
camera2 = cv2.VideoCapture(1)
camera3 = cv2.VideoCapture(2)
camera4 = cv2.VideoCapture(3)


camera1.set(cv2.CAP_PROP_FPS, 10)  # Beperk de framesnelheid tot 10 FPS

# _, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 50])

def download_video(url):
    ydl_opts = {}
    with youtube_dl.YoutubeDL(ydl_opts) as ydl:
        info_dict = ydl.extract_info(url, download=False)
        video_url = info_dict['formats'][0]['url']
    return video_url

# video_url = download_video('https://www.youtube.com/watch?v=_HGQZlK08gQ&ab_channel=SpaceVideos')
cap = cv2.VideoCapture("video_url")


@app.route('/static/<path:filename>')
def serve_static(filename):
    return send_from_directory('static', filename)

@app.route('/', methods=['GET', 'POST'])
def index():
    global code
    result = None
    start_display = True

    if request.method == 'POST':
        button_value = request.form['button_value']
        if button_value == 'C':
            code = ''
        else:
            code += button_value
        input_code = request.form['code']
        if input_code == correct_code:
            result = 'Code juist'
            start_display = None
            return redirect('/dashboard')  # Doorsturen naar het dashboard als de juiste pincode is ingevoerd

    print(result)
    print(start_display)
    return render_template('./index.html', result=result, start_display=start_display, code=code)


@app.route('/dashboard', methods=['GET'])

def dashboard():
    # Hier kun je logica toevoegen om de camerafeeds op te halen en door te sturen naar het template.
    # Bijvoorbeeld, je kunt een lijst van camera-URL's voorbereiden.
    camera_feeds = [
        'url_camera1',
        'url_camera2',
        'url_camera3',
        'url_camera4'
    ]
    return render_template('./dashboard.html', camera_feeds=camera_feeds)

# Functie om frames van de camera's op te halen en te encoderen
def get_frame(url):
    cap = cv2.VideoCapture(url)
    _, frame = cap.read()
    cap.release()
    _, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 50])
    return frame, buffer.tobytes()


@app.route('/camera1')
def camera1_route():
    ret, frame = cap.read()
    _, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 50])
    return Response(buffer.tobytes(), mimetype='image/jpeg')

@app.route('/camera2')
def camera2_route():
    frame, image_buffer = get_frame(camera2)  # Haal het frame en de afbeeldingsbuffer op
    return Response(image_buffer, mimetype='image/jpeg')

@app.route('/camera3')
def camera3_route():
    frame, image_buffer = get_frame(camera3)  # Haal het frame en de afbeeldingsbuffer op
    return Response(image_buffer, mimetype='image/jpeg')

@app.route('/camera4')
def camera4_route():
    frame, image_buffer = get_frame(camera4)  # Haal het frame en de afbeeldingsbuffer op
    return Response(image_buffer, mimetype='image/jpeg')



if __name__ == '__main__':
    app.run(debug=True)
