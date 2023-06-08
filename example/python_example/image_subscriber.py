import datetime
import cv2
import numpy as np
import struct
import json
import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../../python'))
from subscriber import PacketDeserializer

packet_deserializer = PacketDeserializer(5001)
packet_deserializer.start()

while True:
    packet = packet_deserializer.read()
    if packet['type'] == 1:  # If the packet is an image
        array = np.frombuffer(packet['payload'][0:], dtype=np.uint8)  # try '8' if '0' is not working
        img = cv2.imdecode(array, cv2.IMREAD_COLOR)
        if img is None:
            print("Failed to decode image.")
        elif img.size == 0:
            print("Decoded image is empty.")
        else:
            print("Got an image and save it as jpg")
            timestamp = datetime.datetime.now().strftime("%Y%m%d%H%M%S")
            cv2.imwrite(f'image_{timestamp}.jpg', img)
    elif packet['type'] == 2:  # If the packet is a double
        value = struct.unpack('!d', packet['payload'])[0]
        print(f"Received value: {value}")
    elif packet['type'] == 3:  # If the packet is a JSON
        json_str = packet['payload'].decode()
        json_parsed = json.loads(json_str)
        print(f"Received json: {json_parsed}")