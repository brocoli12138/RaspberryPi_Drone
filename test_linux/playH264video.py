from ffpyplayer.player import MediaPlayer
import numpy as np
import cv2

def play_video(video_path):
    # Initialize the media player with the video file
    player = MediaPlayer(video_path)

    while True:
        # Get the next frame
        frame, val = player.get_frame()

        if val == 'eof':
            break
        if frame is None:
            continue

        # Get the frame data and display it
        img, t = frame

        # Extract frame data as a numpy array
        img_data = img.to_bytearray()[0]

        # Get frame dimensions and format
        frame_width, frame_height = img.get_size()

        # Convert bytearray to numpy array and reshape
        frame_array = np.frombuffer(img_data, dtype=np.uint8).reshape((frame_height, frame_width, 3))

        # Convert RGB to BGR format for OpenCV
        frame_bgr = cv2.cvtColor(frame_array, cv2.COLOR_RGB2BGR)

        # Display the frame using OpenCV
        cv2.imshow('Video', frame_bgr)

        if cv2.waitKey(25) & 0xFF == ord('q'):
            break

    # Properly close the media player
    player.close_player()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <video_path>")
        sys.exit(1)

    video_path = sys.argv[1]
    play_video(video_path)