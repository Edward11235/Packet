# Packet:
Packet is designed for real-time low-overhead data transmission over the Internet. It transmits and receives raw blobs. It's easy to use, and we provide both C++ and Python implementation. Currently, we tested transmission jpg, json, and double number. It is a potential alternative for ROS subscriber and publisher over WIFI and Internet.

# How to use:
In the example folder, we provide examples for both C++ and Python. To run the example in C++, you need to install eigen3 and opencv library. The publisher in the example sends a image, a double, and a JSON object, and the subscriber will store the image, and print the double and json.

# Tested functions:
- C++ publisher with C++ subscriber over WIFI.
- Python publisher with Python subscriber over WIFI.
- C++ publisher with Python subscriber over WIFI.
- Python publisher with C++ subscriber over WIFI.

# Developers:
Joshua Zhang is in charge of C++ implementation.
Chunlin (Edward) Li is in charge of Python implementation.
