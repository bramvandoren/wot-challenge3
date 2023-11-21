from flask import Flask, render_template, request, redirect, send_from_directory
import paho.mqtt.client as mqtt

app = Flask(__name__)

correct_code = '9508'
code = ''

# Define MQTT broker information
mqtt_broker = "192.168.0.191"
mqtt_port = 1883
mqtt_topic = "camera/status"

# MQTT client setup
client = mqtt.Client()

# def on_connect(client, userdata, flags, rc):
#     print("Connected with result code "+str(rc))
#     client.subscribe(mqtt_topic)

# def on_message(client, userdata, msg):
#     global camera_status
#     camera_status = msg.payload.decode()
#     print("Received Camera Status from MQTT: ", camera_status)

# client.on_connect = on_connect
# client.on_message = on_message

# client.connect(mqtt_broker, mqtt_port, 60)

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
    
    # global camera_status
    # camera_status = request.args.get('camera_status', default="Camera aan")
    # print("Received Camera Status from MQTT: ", camera_status)

    # camera_feeds = [
    # 'http://192.168.0.121:81/stream/camera1',
    # 'http://192.168.0.121:81/stream/camera2',
    # 'http://192.168.0.121:81/stream/camera3',
    # 'http://192.168.0.121:81/stream/camera4'
    # ]

    
    return render_template('./dashboard.html')



if __name__ == '__main__':
    app.run(debug=True, host="0.0.0.0")
