import cv2
import numpy as np
import paho.mqtt.client as mqtt

# MQTT configuration
MQTT_BROKER = "192.168.121.101"
MQTT_PORT = 1883
MQTT_TOPIC = "data/video"

# Frame dimensions (update according to your setup)
FRAME_WIDTH = 640
FRAME_HEIGHT = 480

def on_message(client, userdata, message):
    """Callback function to handle incoming MQTT messages."""
    try:
        # Decode the message payload into a numpy array
        yuv_data = np.frombuffer(message.payload, dtype=np.uint8)
        
        # Ensure the data size matches the expected frame size
        expected_size = FRAME_WIDTH * FRAME_HEIGHT * 3 // 2  # YUV420p size
        if len(yuv_data) != expected_size:
            print("Invalid frame size received!")
            return

        # Reshape YUV420 data for OpenCV
        yuv_frame = yuv_data.reshape((FRAME_HEIGHT * 3 // 2, FRAME_WIDTH))

        # Convert YUV420 to BGR for display
        bgr_frame = cv2.cvtColor(yuv_frame, cv2.COLOR_YUV2BGR_I420)

        # Display the frame
        cv2.imshow("YUV420 Frame", bgr_frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            client.disconnect()
            cv2.destroyAllWindows()

    except Exception as e:
        print(f"Error processing frame: {e}")

def main():
    # Set up MQTT client
    client = mqtt.Client()
    client.on_message = on_message

    try:
        # Connect to MQTT broker
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.subscribe(MQTT_TOPIC)

        # Start MQTT loop
        print("Listening for frames...")
        client.loop_forever()
    except KeyboardInterrupt:
        print("Exiting...")
        client.disconnect()
        cv2.destroyAllWindows()
    except Exception as e:
        print(f"MQTT Error: {e}")

if __name__ == "__main__":
    main()
