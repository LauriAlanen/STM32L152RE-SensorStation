from flask import Flask, render_template, jsonify, request
from threading import Thread
from master import Master
from sensor_data_collector import SensorDataCollector
from data_store import SensorDataStore
import sensors  # Ensure your sensors module is in the PYTHONPATH

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


@app.route('/sensor_management')
def sensor_management():
    return render_template("add_sensor_page.html")


@app.route('/add_sensor', methods=['POST'])
def add_sensor():
    """
    Add a new sensor dynamically.

    Expected JSON payload:
    {
        "sensor_type": "LMT84LP",   // Must exist in sensors module
        "sensor_name": "temp_sensor", // Unique name for the sensor
        "address": "0x01"           // Can be a hex string or integer
    }
    """
    data = request.get_json(force=True)

    sensor_type = data.get("sensor_type")
    sensor_name = data.get("sensor_name")
    address = data.get("address")

    if not sensor_type or not sensor_name or address is None:
        return jsonify({"error": "Missing one or more required parameters: sensor_type, sensor_name, address"}), 400

    # Convert address to integer. Accept hex strings or integer values.
    try:
        if isinstance(address, str):
            if address.lower().startswith("0x"):
                address_int = int(address, 16)
            else:
                address_int = int(address)
        else:
            address_int = int(address)
    except ValueError:
        return jsonify({"error": "Invalid address format. Use a valid integer or hex string (e.g., '0x01')."}), 400

    # Dynamically retrieve the sensor class from the sensors module.
    try:
        sensor_class = getattr(sensors, sensor_type)
    except AttributeError:
        return jsonify({"error": f"Sensor type '{sensor_type}' not found in sensors module."}), 400

    # Instantiate the sensor with the provided address and name.
    try:
        sensor_instance = sensor_class(address_int, sensor_name)
    except Exception as e:
        return jsonify({"error": f"Error instantiating sensor: {e}"}), 400

    # Add the sensor to the Master. This will error out if sensor_name already exists.
    try:
        master.add_sensor(sensor_name, sensor_instance)
    except ValueError as ve:
        return jsonify({"error": str(ve)}), 400

    return jsonify({"status": f"Sensor '{sensor_name}' of type '{sensor_type}' added successfully with address {address_int}."}), 200


if __name__ == '__main__':
    app.run(debug=True, use_reloader=False)
