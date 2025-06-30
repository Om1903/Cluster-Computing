import json
import threading
import cv2 as cv
import numpy as np
import paho.mqtt.client as mqtt
from nodes import Node
import base64


BROKER_ADDRESS = "192.168.132.195"
BROKER_PORT = 1883
IDENTITY_TOPIC = "/topic/identity"
SIZE = 256

nodes: list[Node] = []
client = mqtt.Client()
output_image = np.zeros((SIZE, SIZE, 3), dtype=np.uint8)

client.on_connect = lambda client, userdata, flags, rc: print(f"Connected with result {rc} to EQMX")
client.on_disconnect = lambda client, userdata, rc: print(f"Disconnected with result {rc} from EQMX")

client.connect(BROKER_ADDRESS, BROKER_PORT, 60)

client.subscribe(IDENTITY_TOPIC)

def on_message(client, userdata, msg):
    if msg.topic == IDENTITY_TOPIC:
        node = Node(msg.payload.decode(), client)
        nodes.append(node)
        client.subscribe(node.private_topic + "/out")
        print(f"Node {node.mac_addr} registered.")
        node.register_status_topic()
    elif msg.topic in [node.private_topic + "/out" for node in nodes]:
        data = msg.payload
        img = np.frombuffer(data, dtype=np.uint8)
        img = img.reshape((SIZE // 2, SIZE // 2, 3))
        global output_image
        node = next((n for n in nodes if n.private_topic in msg.topic), None)
        if not node:
            return
        print(node.mac_addr)
        if len(node.stats) == 0:
            return
        if node.stats[0] == 0:
            output_image[0:128, 0:128, :] = img
        elif node.stats[0] == 1:
            output_image[0:128, 128:256, :] = img
        elif node.stats[0] == 2:   
            output_image[128:256, 0:128, :] = img
        elif node.stats[0] == 3:
            output_image[128:256, 128:256, :] = img
        node.stats.pop(0)

client.on_message = on_message
client.loop_start()

def spawn_window(img, name="Image"):
    def cb():
        while True:
            cv.imshow(name, img)
            if cv.waitKey(1) & 0xff == ord('q'):
                cv.destroyAllWindows()
                break

    thread = threading.Thread(target=cb)
    thread.start()
    return thread

try:
    while True:
        data = input("> ")

        output_image = np.zeros((SIZE, SIZE, 3), dtype=np.uint8)
        for node in nodes:
            node.stats = []

        img = cv.imread(data)
        img = cv.resize(img, (SIZE, SIZE))
        ot = spawn_window(output_image, name="Output Image")
        print(img[0:SIZE//2, 0:SIZE//2, :].shape, img.dtype)
        print(len(base64.b64encode(img[0:SIZE//2, 0:SIZE//2, :].tobytes()).decode("ascii")))

        chops = [
            img[0:SIZE//2, 0:SIZE//2, :].tobytes(),
            img[0:SIZE//2, SIZE//2:SIZE, :].tobytes(),
            img[SIZE//2:SIZE, 0:SIZE//2, :].tobytes(),
            img[SIZE//2:SIZE, SIZE//2:SIZE, :].tobytes()
        ]

        for idx, chop in enumerate(chops):
            print(f"Sending chop {idx} to node {nodes[idx % len(nodes)].mac_addr} on {nodes[idx % len(nodes)].private_topic}")
            node = nodes[idx % len(nodes)]
            node.stats.append(idx)
            node.send_data(chop)
            print(f"Chop {idx} sent to node {node.mac_addr}")

        ot.join()
        
finally:
    cv.destroyAllWindows()
