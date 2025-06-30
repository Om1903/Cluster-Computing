import json

class Node:
    def __init__(self, mac_addr: str, client):
        self.mac_addr = mac_addr
        self.client = client
        self.private_topic = f'/topic/{self.mac_addr}'
        self.task_pending = False
        self.task_accepted = False
        self.stats = []

    def register_status_topic(self):
        self.client.subscribe(f'{self.private_topic}/task_status')

    def send_data(self, data):
        self.task_pending = True
        self.client.publish(self.private_topic, data)

    def send_stats(self, pos):
        self.client.publish(f'{self.private_topic}/stats', json.dumps({"pos": pos}))
