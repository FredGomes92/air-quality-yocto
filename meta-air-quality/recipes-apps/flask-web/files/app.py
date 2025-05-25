
import json
from datetime import datetime
import os

DATA_FILE = 'data.json'

# Ensure data file exists
if not os.path.exists(DATA_FILE):
    with open(DATA_FILE, 'w') as f:
        json.dump({'temperature': [], 'humidity': []}, f)

import eventlet
eventlet.monkey_patch()

from flask import Flask, render_template
from flask_socketio import SocketIO
import threading
import paho.mqtt.client as mqtt
import pytz

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode="eventlet")

temperature_data = []
humidity_data = []

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
TOPIC_TEMPERATURE = "sensors/temperature"
TOPIC_HUMIDITY = "sensors/humidity"
TOPIC_PMS = "sensors/pms"


def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with code {rc}")
    client.subscribe([(TOPIC_TEMPERATURE, 0), (TOPIC_HUMIDITY, 0), (TOPIC_PMS, 0)])

def on_message(client, userdata, msg):
    global temperature_data, humidity_data
    try:
        print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")
        payload_str = msg.payload.decode()
        try:
            payload = json.loads(payload_str)
            if isinstance(payload, dict):
                cest = pytz.timezone('Europe/Berlin')
                timestamp = datetime.now(cest).isoformat()
                new_entry = {'time': timestamp, **payload}
                print(f"New entry: {new_entry}")
            else:
                print(f"Unexpected payload format: {payload}")
                return
        except json.JSONDecodeError:
            print(f"Failed to decode JSON: {payload_str}")

        with open(DATA_FILE, 'r+') as f:
            data = json.load(f)
            if msg.topic == TOPIC_TEMPERATURE:
                key = 'temperature'
            elif msg.topic == TOPIC_HUMIDITY:
                key = 'humidity'
            elif msg.topic == TOPIC_PMS:
                key = 'pms'
            else:
                print(f"Unknown topic: {msg.topic}")
                return
            if key not in data:
                data[key] = []
            data[key].append(new_entry)
            data[key] = data[key][-50:]  # Keep only latest 50

            f.seek(0)
            json.dump(data, f, indent=2)
            f.truncate()

        socketio.emit(f'new_{key}', new_entry)
    except Exception as e:
        print(f"Error: {e}")

def mqtt_thread():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_forever()


@socketio.on('connect')
def send_initial_data():
    print("socketio client connected")
    with open(DATA_FILE) as f:
        data = json.load(f)
    socketio.emit('initial_data', data)

@app.route("/<chart_type>")
def chart_view(chart_type):
    if chart_type not in ["temperature", "humidity", "pms", "pms_aqi"]:
        return "Invalid chart type", 404
    return render_template("chart.html", chart_type=chart_type)

@app.route("/")
def index():
    return render_template("index.html")

if __name__ == "__main__":
    threading.Thread(target=mqtt_thread, daemon=True).start()
    socketio.run(app, host="0.0.0.0", port=8080)


