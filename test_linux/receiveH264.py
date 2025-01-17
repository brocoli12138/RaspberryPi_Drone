import paho.mqtt.client as mqtt
import av
import cv2
import numpy as np
import io

# MQTT parameters
BROKER_ADDRESS = "192.168.121.101"  # Replace with your MQTT broker address
TOPIC = "data/video"             # Replace with your MQTT topic

# Initialize global variables
frame_buffer = b""  # Buffer to hold incoming H264 frames
decoder = None       # FFmpeg decoder

def on_connect(client, userdata, flags, rc):
    """Callback when connected to the MQTT broker."""
    print(f"Connected with result code {rc}")
    client.subscribe(TOPIC)

def on_message(client, userdata, msg):
    """Callback when a message is received on the subscribed topic."""
    global frame_buffer
    frame_buffer = msg.payload

def decode_and_display(frame_data):
    """Decode YUV420 frame data using PyAV and display it using OpenCV."""
    global decoder

    # Initialize decoder if not already set
    if decoder is None:
        decoder = av.CodecContext.create("h264", "r")

    # Feed the frame data to the decoder
    packets = decoder.parse(frame_data)

    # Decode packets into frames
    for packet in packets:
        for frame in decoder.decode(packet):
            # Ensure the pixel format is YUV420
            if frame.format.name != "yuv420p":
                print(f"Unexpected pixel format: {frame.format.name}")
                continue

            # Convert YUV420 frame to BGR for OpenCV display
            img = cv2.cvtColor(frame.to_ndarray(), cv2.COLOR_YUV2BGR_I420)

            # Display the frame
            cv2.imshow("Video Stream", img)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                return False
    return True

def main():
    global frame_buffer

    # Initialize the MQTT client
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # Connect to the MQTT broker
    client.connect(BROKER_ADDRESS, 1883, 60)

    # Start the MQTT client loop in a separate thread
    client.loop_start()

    print("Waiting for frames...")

    try:
        while True:
            if frame_buffer:
                # Decode and display the frame
                if not decode_and_display(frame_buffer):
                    break
                frame_buffer = b""  # Clear the buffer after processing
    except KeyboardInterrupt:
        print("Stopping...")
    finally:
        client.loop_stop()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
