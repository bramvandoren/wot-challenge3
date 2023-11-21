from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import cv2
import base64
from io import BytesIO
from PIL import Image

app = Flask(__name__)
socketio = SocketIO(app)

def gen(camera):
    while True:
        frame = camera.get_frame()
        if frame is not None:
            # Encode the frame to base64 for easy transmission over WebSocket
            frame_base64 = base64.b64encode(frame).decode('utf-8')
            socketio.emit('image', frame_base64)
            socketio.sleep(0.01)  # Adjust the sleep time as needed to control the frame rate

class Camera:
    def __init__(self):
        self.cap = cv2.VideoCapture(0)

    def __del__(self):
        self.cap.release()

    def get_frame(self):
        ret, frame = self.cap.read()
        if ret:
            _, jpeg = cv2.imencode('.jpg', frame)
            return jpeg.tobytes()
        return None

@app.route('/')
def index():
    return render_template('test.html')

@socketio.on('connect')
def handle_connect():
    print('Client connected')
    camera = Camera()
    socketio.start_background_task(gen, camera)

if __name__ == '__main__':
    socketio.run(app, debug=True)
