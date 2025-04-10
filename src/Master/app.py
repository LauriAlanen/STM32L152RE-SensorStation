from flask import Flask, render_template, jsonify
from threading import Thread
from master import Master
from sensor_data_collector import SensorDataCollector
from data_store import SensorDataStore

app = Flask(__name__)
store = SensorDataStore()
master = Master("COM5", 9600)
master.connect()

collector = SensorDataCollector(master, store, 10)

# Start collection in a background thread
thread = Thread(target=collector.start, daemon=True)
thread.start()


@app.route('/')
def index():
    return render_template("index.html")


@app.route('/data')
def data():
    return jsonify(store.dump())


@app.route('/stop')
def stop():
    collector.stop()
    return "Stopped"


if __name__ == '__main__':
    app.run(debug=True, use_reloader=False)
