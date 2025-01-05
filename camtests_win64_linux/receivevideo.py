import cv2
import paho.mqtt.client as mqtt
import base64
import numpy as np

# MQTT Broker 地址和主题
MQTT_BROKER = "192.168.121.117"
MQTT_PORT = 1883
MQTT_TOPIC = "video/frames"

# 初始化 MQTT 客户端
client = mqtt.Client()

# MQTT 连接成功回调函数
def on_connect(client, userdata, flags, rc):
    print(f"连接成功，返回码：{rc}")
    client.subscribe(MQTT_TOPIC)

# MQTT 接收到消息回调函数
def on_message(client, userdata, msg):
    # 接收到的消息是 Base64 编码的帧
    try:
        frame_base64 = msg.payload.decode('utf-8')
        # 将 Base64 字符串解码为字节流
        frame_data = base64.b64decode(frame_base64)
        # 将字节流转换为图像
        np_arr = np.frombuffer(frame_data, dtype=np.uint8)
        frame = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

        # 显示接收到的视频帧
        if frame is not None:
            cv2.imshow("Received Video", frame)
        else:
            print("解码失败，接收到的数据不是有效的图像帧")
    except Exception as e:
        print(f"处理消息时发生错误：{e}")

# 连接到 MQTT Broker
def connect_mqtt():
    try:
        client.on_connect = on_connect
        client.on_message = on_message
        client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
        print(f"成功连接到 MQTT Broker: {MQTT_BROKER}:{MQTT_PORT}")
    except Exception as e:
        print(f"连接 MQTT Broker 失败: {e}")
        exit()

def main():
    # 连接到 MQTT Broker
    connect_mqtt()

    # 持续运行 MQTT 客户端，等待接收消息
    print("等待接收视频帧...")
    while True:
        client.loop()  # 处理回调并等待消息

        # 按 'q' 键退出
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # 关闭 OpenCV 窗口
    cv2.destroyAllWindows()
    client.disconnect()

if __name__ == "__main__":
    main()
