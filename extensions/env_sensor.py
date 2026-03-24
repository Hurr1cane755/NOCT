# ── EXTENSION 2: Environmental Sensor Logging (DHT22) ───────────────
# Hardware required: DHT22 temperature/humidity sensor
#   Connect: DATA → GPIO 4 (RPi Zero 2W)  |  VCC → 3.3V  |  GND → GND
# Install library: pip install adafruit-circuitpython-dht
# Add the following to the existing stream.py on the Raspberry Pi Zero 2W
 
import adafruit_dht
import board
import json
 
# Initialise DHT22 sensor object on GPIO pin 4
dht_sensor = adafruit_dht.DHT22(board.D4)
 
# read_environment() — reads temperature (°C) and relative humidity (%) from DHT22
# DHT22 occasionally fails a read cycle; RuntimeError is caught and returns None
# values to prevent the server from crashing on a missed measurement
def read_environment():
    try:
        return {
            'temperature_c': dht_sensor.temperature,  # degrees Celsius
            'humidity_pct':  dht_sensor.humidity,     # relative humidity %
            'status': 'ok'
        }
    except RuntimeError:
        # Sensor missed read — return null values; browser handles gracefully
        return {'temperature_c': None, 'humidity_pct': None,
                'status': 'sensor_error'}
 
# /environment — Flask route returning current sensor readings as JSON
# Browser JavaScript polls this endpoint every 2000 ms to update the display
@app.route('/environment')
def environment_data():
    return app.response_class(
        response=json.dumps(read_environment()),
        status=200,
        mimetype='application/json'
    )
 
# ── Browser-side JavaScript to display sensor readings (add to HTML page) ──
# setInterval(function() {
#   fetch('http://<RPI_IP>:5000/environment')
#     .then(response => response.json())
#     .then(data => {
#       if (data.status === 'ok') {
#         document.getElementById('temp').innerText = data.temperature_c + ' °C';
#         document.getElementById('hum').innerText  = data.humidity_pct + ' %';
#       }
#     });
# }, 2000);  // poll every 2 seconds
