# ================================================================
# NOCT — Camera Streaming Server
# Raspberry Pi Zero 2W
# ================================================================
 
# Flask     — lightweight Python web framework; serves the MJPEG stream over HTTP
# picamera2 — official Raspberry Pi camera library for Camera Module 3 NoIR
# io        — BytesIO: in-memory byte buffer for captured JPEG frames
# time      — sleep() used to cap frame rate and limit CPU load on Zero 2W
from flask import Flask, Response
from picamera2 import Picamera2
import io, time
 
app = Flask(__name__)
 
# Initialise camera with video-optimised configuration
# 640x480 resolution: balances image quality with network streaming latency
# create_video_configuration() enables continuous frame capture mode
camera = Picamera2()
camera.configure(camera.create_video_configuration(main={"size": (640, 480)}))
camera.start()
 
# generate() — Python generator function yielding MJPEG frames indefinitely
# Uses multipart/x-mixed-replace MIME type to push frames as HTTP stream
def generate():
    stream = io.BytesIO()          # reusable in-memory buffer
    while True:
        stream.seek(0)             # reset buffer position to start
        camera.capture_file(stream, format='jpeg')  # capture frame as JPEG
        stream.seek(0)             # rewind to read captured bytes
        frame = stream.read()
        # Wrap each frame in MJPEG multipart boundary headers
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n'
               + frame + b'\r\n')
        # ~20 fps cap — prevents 100% CPU utilisation on Raspberry Pi Zero 2W
        time.sleep(0.05)
 
# /stream — HTTP route returning the live MJPEG video stream
# mimetype 'multipart/x-mixed-replace' is the standard MJPEG container format
@app.route('/stream')
def video_feed():
    return Response(generate(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')
 
# / — index route: simple page confirming server is running
@app.route('/')
def index():
    return '<img src="/stream" style="width:100%">'
 
# Start Flask on all network interfaces, port 5000
# host='0.0.0.0' makes the stream accessible from any device on the LAN
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
