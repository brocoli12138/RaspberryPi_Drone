import cv2
import numpy as np

# Define the video stream URL
stream_url = 'tcp://192.168.121.111:1234'

# Open the video stream using OpenCV
cap = cv2.VideoCapture(stream_url, cv2.CAP_FFMPEG)

# Check if the stream is opened successfully
if not cap.isOpened():
    print('Error: Unable to open video stream.')
    exit()

# Set the desired framerate
framerate = 120
frame_delay = int(1000 / framerate)  # milliseconds per frame

while True:
    ret, frame = cap.read()

    if not ret:
        print('Error: Unable to read frame from stream.')
        break

    # Display the frame
    cv2.imshow('TCP Video Stream', frame)

    # Wait for 'q' to exit, or adjust framerate
    if cv2.waitKey(frame_delay) & 0xFF == ord('q'):
        break

# Release the capture object and close any OpenCV windows
cap.release()
cv2.destroyAllWindows()
