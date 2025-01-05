import cv2
import paho.mqtt.client as mqtt
import base64

# MQTT Broker 地址和主题
MQTT_BROKER = "192.168.121.117"
MQTT_PORT = 1883
MQTT_TOPIC = "video/frames"

# 初始化 MQTT 客户端
client = mqtt.Client()

# 连接到 MQTT Broker
def connect_mqtt():
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
        print(f"成功连接到 MQTT Broker: {MQTT_BROKER}:{MQTT_PORT}")
    except Exception as e:
        print(f"连接 MQTT Broker 失败: {e}")
        exit()

# 发送视频帧数据
def send_frame(frame):
    # 将视频帧编码为 JPEG 格式
    _, buffer = cv2.imencode('.jpg', frame)
    # 转换为 Base64 字符串
    frame_base64 = base64.b64encode(buffer).decode('utf-8')
    # 通过 MQTT 发布
    client.publish(MQTT_TOPIC, frame_base64)

def main():
    # 连接到 MQTT Broker
    connect_mqtt()

    # 打开摄像头
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("无法打开摄像头")
        exit()

    print("开始捕获视频帧，按 'q' 键退出")
    while True:
        ret, frame = cap.read()
        if not ret:
            print("无法捕获视频帧")
            break

        # 显示视频帧
        cv2.imshow("Sending Video", frame)

        # 发送帧数据到 MQTT Broker
        send_frame(frame)

        # 按下 'q' 键退出
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # 释放摄像头和资源
    cap.release()
    cv2.destroyAllWindows()
    client.disconnect()
    print("已断开与 MQTT Broker 的连接")

if __name__ == "__main__":
    main()
