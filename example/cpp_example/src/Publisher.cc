#include "Packet.h"
#include <iostream>
#include <random>
#include <fstream>
#include <functional>
#include <Eigen/Core>
#include <opencv2/core/core.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/opencv.hpp>
#include "json.hpp"

std::pair<char*, size_t> serializeVectorToCharPtr(const std::vector<unsigned char>& vec) {
    size_t size = vec.size();
    char* data = new char[size];
    std::memcpy(data, vec.data(), size);
    return std::make_pair(data, size);
}

int main(int argc, char **argv) {
    // start packet publisher
    PacketSerializer channel("127.0.0.1", 5001);
    int status = channel.start();
    std::cout << "status is " << status << std::endl;

    // initialize a image, a double, and a json
    std::string imagePath = "../src/sunflower.jpg";
    cv::Mat color_image = cv::imread(imagePath);

    double timestamp = 123.456;
    nlohmann::json json_data;
    json_data["key"] = "value";

    // send image
    std::vector<uchar> data;
    cv::imencode(".jpg", color_image, data);
    std::pair<char*, size_t> result = serializeVectorToCharPtr(data);
    char* serializedData = result.first;
    size_t serializedSize = result.second;
    channel.write(1, serializedData, serializedSize);
    delete[] result.first;
    
    // send double
    channel.write(2, (const char *)&timestamp, 8);

    // send json
    std::string json_str = json_data.dump();
    const char* charPtr = json_str.c_str();
    channel.write(3, charPtr, json_str.size());

    return 0;
}
