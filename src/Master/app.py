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
master.add_sensor("Makuuhuoneen Valo", sensors.LMT84LP(0x01, "Testi"))
master.add_sensor("Makuuhuoneen Ilmanlaatu", sensors.SGP30(0x05, "SGP30"))
master.add_sensor("Makuuhuone",
                  sensors.DHT22(0x06, "DHT22"))

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
    return render_template("add_sensor.html")


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


@app.route('/active_sensors', methods=['GET'])
def active_sensors():
    """
    Returns a list of all currently active sensors.
    Each sensor is represented as:
    {
        "name": "sensor_name",
        "type": "SensorClassName",
        "address": 1
    }
    """
    sensors_list = []
    for name, sensor in master.sensors.items():
        sensors_list.append({
            "name": name,
            "type": sensor.__class__.__name__,
            "address": getattr(sensor, 'address', None)
        })
    return jsonify(sensors_list), 200


@app.route('/sensor_metadata', methods=['GET'])
def sensor_metadata():
    """
    Returns metadata about all sensors including units and channel information.

    Response format:
    {
        "sensor_name": {
            "type": "SensorClassName",
            "address": 1,
            "channels": {
                0: {
                    "name": "Temperature",
                    "unit": "°C"
                },
                1: {
                    "name": "Humidity",
                    "unit": "%"
                }
            }
        }
    }
    """
    metadata = {}
    for name, sensor in master.sensors.items():
        channels = {}
        for channel in range(sensor.channels):
            channels[channel] = {
                "name": sensor.channel_names.get(channel, f"Channel {channel}"),
                "unit": sensor.units.get(channel, "unknown")
            }

        metadata[name] = {
            "type": sensor.__class__.__name__,
            "address": getattr(sensor, 'address', None),
            "channels": channels
        }
    return jsonify(metadata), 200


if __name__ == '__main__':
    app.run(debug=True, use_reloader=False)
