import cv2
import paho.mqtt.client as mqtt
import numpy as np

# MQTT Broker 地址和主题
MQTT_BROKER = "192.168.50.89"
MQTT_PORT = 1883
MQTT_TOPIC = "data/video"

# 初始化 MQTT 客户端
client = mqtt.Client()

# MQTT 连接成功回调函数
def on_connect(client, userdata, flags, rc):
    print(f"连接成功，返回码：{rc}")
    client.subscribe(MQTT_TOPIC)

# YUYV 转换为 RGB
def yuyv_to_rgb(frame, width, height):
    """
    将 YUYV 格式的帧数据转换为 RGB 格式。
    :param frame: YUYV 格式的帧数据
    :param width: 图像的宽度
    :param height: 图像的高度
    :return: RGB 图像
    """
    yuyv = np.frombuffer(frame, dtype=np.uint8).reshape((height, width, 2))
    y = yuyv[:, :, 0]
    u = yuyv[::2, :, 1]  # 每隔两个像素取一个 U
    v = yuyv[1::2, :, 1]  # 每隔两个像素取一个 V

    # 将 U 和 V 插值到整个图像
    u = cv2.resize(u, (width, height), interpolation=cv2.INTER_LINEAR)
    v = cv2.resize(v, (width, height), interpolation=cv2.INTER_LINEAR)

    # 合并 YUV 数据
    yuv = np.stack((y, u, v), axis=-1)

    # 转换为 RGB 格式
    rgb = cv2.cvtColor(yuv, cv2.COLOR_YUV2BGR)
    return rgb

# MQTT 接收到消息回调函数
def on_message(client, userdata, msg):
    # 假设帧数据直接以 YUYV 格式发送
    try:
        frame_data = msg.payload
        print(f"Received data size: {len(msg.payload)}")
        width = 800  # 解析数据前需知道图像宽度
        height = 600  # 解析数据前需知道图像高度

        # 将 YUYV 转换为 RGB 图像
        rgb_frame = yuyv_to_rgb(frame_data, width, height)

        # 显示接收到的 RGB 图像
        cv2.imshow("Received Video", rgb_frame)
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
