import cv2
import numpy as np
import json
import struct
import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../../python'))
from publisher import PacketSerializer

# Load an image and encode it
image = cv2.imread('sunflower.jpg')
_, image_encoded = cv2.imencode('.jpg', image)
image_data = image_encoded.tobytes()

# A double value to be sent
value = 123.456
value_data = struct.pack('d', value)

# A JSON object to be sent
json_obj = {"key": "value"}
json_str = json.dumps(json_obj)
json_data = json_str.encode()

# Create a PacketSerializer instance connected to localhost on port 5001
serializer = PacketSerializer('localhost', 5001)
serializer.start()

# Send the image
serializer.write(1, image_data, len(image_data))

# Send the double value
serializer.write(2, value_data, len(value_data))

# Send the JSON object
serializer.write(3, json_data, len(json_data))
