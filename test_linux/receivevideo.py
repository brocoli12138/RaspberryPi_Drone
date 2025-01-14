import paho.mqtt.client as mqtt
import numpy as np
import cv2

# MQTT settings
BROKER = "192.168.121.101"
PORT = 1883
TOPIC = "data/video"

def yuyv_to_rgb(yuyv_frame, width, height):
    def clamp(value, min_val=0, max_val=255):
        return max(min_val, min(value, max_val))
    
    # Create an empty array for the RGB frame
    rgb_frame = np.zeros((height, width, 3), dtype=np.uint8)
    
    for y in range(height):
        for x in range(0, width, 2):
            # Get the Y, U, and V values from the YUYV frame
            y1 = yuyv_frame[y * width * 2 + 2 * x]
            u = yuyv_frame[y * width * 2 + 2 * x + 1]
            y2 = yuyv_frame[y * width * 2 + 2 * x + 2]
            v = yuyv_frame[y * width * 2 + 2 * x + 3]
            
            # Convert YUV to RGB for the first pixel
            r1 = clamp(y1 + 1.402 * (v - 128))
            g1 = clamp(y1 - 0.344136 * (u - 128) - 0.714136 * (v - 128))
            b1 = clamp(y1 + 1.772 * (u - 128))
            
            # Convert YUV to RGB for the second pixel
            r2 = clamp(y2 + 1.402 * (v - 128))
            g2 = clamp(y2 - 0.344136 * (u - 128) - 0.714136 * (v - 128))
            b2 = clamp(y2 + 1.772 * (u - 128))
            
            # Assign the RGB values to the corresponding positions in the RGB frame
            rgb_frame[y, x] = [r1, g1, b1]
            rgb_frame[y, x + 1] = [r2, g2, b2]
    
    return rgb_frame

# Callback when message is received
def on_message(client, userdata, msg):
     # Assuming msg.payload is the YUYV frame data
    yuyv_frame = msg.payload
    # Frame dimensions
    width = 800  # Update with your frame width
    height = 600  # Update with your frame height
    rgb_image = yuyv_to_rgb(yuyv_frame, width, height)

    # Display the image
    cv2.imshow("Received Frame", rgb_image)
    cv2.waitKey(1)

# MQTT client setup
client = mqtt.Client()
client.on_message = on_message

# Connect to the MQTT broker
client.connect(BROKER, PORT)

# Subscribe to the topic
client.subscribe(TOPIC)

# Loop to keep receiving messages
client.loop_forever()
