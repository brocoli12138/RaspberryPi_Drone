import paho.mqtt.client as mqtt
import av
import numpy as np
import cv2
import io

# Callback for when a message is received on the subscribed topic
def on_message(client, userdata, msg):
    # The received message contains the H264 frame data (binary)
    h264_data = msg.payload
    
    # Decode the received H264 frame
    decode_h264_frame(h264_data)

def decode_h264_frame(h264_data):
    # Create an in-memory byte buffer from the H264 frame data
    container = av.open(io.BytesIO(h264_data))
    
    # Iterate over the packets in the container
    for packet in container.demux():
        # Iterate over the frames in the packet (should be one frame per packet for H264)
        for frame in packet.decode():
            # Check if the frame is in NV12 format
            if frame.format.name == 'nv12':
                y_plane = frame.to_ndarray()[0]  # Y plane (height x width)
                uv_plane = frame.to_ndarray()[1]  # UV plane (height // 2 x width // 2)
                
                # Show the Y plane as a grayscale image
                cv2.imshow('Y Plane', y_plane)
                cv2.waitKey(1)

# Set up MQTT client
mqtt_broker = "mqtt.example.com"  # Replace with your broker's address
mqtt_port = 1883  # Replace with your broker's port
mqtt_topic = "video/frames"  # Replace with the topic you're subscribing to

client = mqtt.Client()

# Set up the on_message callback
client.on_message = on_message

# Connect to the MQTT broker
client.connect(mqtt_broker, mqtt_port, 60)

# Subscribe to the topic
client.subscribe(mqtt_topic)

# Start the MQTT client loop
client.loop_forever()

cv2.destroyAllWindows()
