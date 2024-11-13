from flask import Flask, jsonify, render_template
from pymongo import MongoClient
import paho.mqtt.client as mqtt
from datetime import datetime

app = Flask(__name__)

mongo_client = MongoClient('mongodb://localhost:27017/')
db = mongo_client['hidroponikDB']
collection = db['sensorData']

mqtt_broker = 'broker.hivemq.com'
mqtt_port = 1883
mqtt_topic_suhu = 'hidroponik/suhu'
mqtt_topic_kelembapan = 'hidroponik/kelembapan'

def on_connect(client, userdata, flags, rc):
    print("Terhubung ke MQTT broker dengan kode: " + str(rc))
    client.subscribe([(mqtt_topic_suhu, 0), (mqtt_topic_kelembapan, 0)])

def on_message(client, userdata, msg):
    topic = msg.topic
    value = float(msg.payload.decode())
    current_time = datetime.now()
    sensor_type = 'temperature' if topic == mqtt_topic_suhu else 'humidity'
    document = {
        'sensorType': sensor_type,
        'value': value,
        'timestamp': current_time
    }
    collection.insert_one(document)
    print(f"Data tersimpan: {document}")

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(mqtt_broker, mqtt_port, 60)
mqtt_client.loop_start()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/sensorData', methods=['GET'])
def get_sensor_data():
    data = list(collection.find().sort("timestamp", -1).limit(10))
    for item in data:
        item['_id'] = str(item['_id'])
    return jsonify(data)

@app.route('/data', methods=['GET'])
def get_data():
    data = {
        "suhumax": 30.1,
        "suhumin": 28.2,
        "suhurata": 29.0,
        "nilai_suhu_max_humid_max": {
            "0": {
                "idx": 0,
                "suhu": 30.1,
                "humid": 60.0,
                "Kecerahan": None,
                "timestamp": datetime.now().isoformat()
            },
            "1": {
                "idx": 1,
                "suhu": 30.0,
                "humid": 58.0,
                "Kecerahan": None,
                "timestamp": datetime.now().isoformat()
            }
        },
        "month_year_max": {
            "0": {
                "month_year": "11-2024"
            }
        }
    }
    return jsonify(data)

if __name__ == '__main__':
    app.run(debug=True)
