#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <sys/wait.h>
#include <unistd.h>
#include "Packet.h"
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

std::vector<unsigned char> deserializeVectorFromCharPtr(const char* data, size_t size) {
    std::vector<unsigned char> vec(size);
    std::memcpy(vec.data(), data, size);
    return vec;
}

void LoadImages(const string &strFile, vector<string> &vstrImageFilenames, vector<double> &vTimestamps);

string GetDatasetName(const string &strSequencePath);

int main(int argc, char **argv){
    PacketDeserializer channel(5001);
    int status = channel.start();
    std::cout << "return value of start is " << status << std::endl;

    while(true){
        try{
            // receive image
            cv::Mat image;
            Packet image_packet;
            channel.read(image_packet);
            if(image_packet.type = 1) std::cout << "Received image" << std::endl;
            if(image_packet.type != 1) std::cout << "warning: wrong type. Should be one for image" << std::endl;
            std::vector<unsigned char> deserializedData = deserializeVectorFromCharPtr(image_packet.payload, image_packet.size);
            image = cv::imdecode(cv::Mat(deserializedData), cv::IMREAD_COLOR);
            cv::imwrite("received.jpg", image);

            // receive double
            double value;
            Packet value_packet;
            channel.read(value_packet);
            value = *((double*)value_packet.payload);
            std::cout << "Received value: " << value << std::endl;

            // receive json
            Packet json_packet;
            channel.read(json_packet);
            std::string json_str(json_packet.payload, json_packet.size);
            nlohmann::json json_parsed = nlohmann::json::parse(json_str);
            std::cout << "Received json: " << json_parsed << std::endl;

            // memory clean up since we used C to write Packet
            delete[] image_packet.payload;
            delete[] value_packet.payload;
            delete[] json_packet.payload;
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }
    return 0;
}

void LoadImages(const string &strFile, vector<string> &vstrImageFilenames, vector<double> &vTimestamps)
{
    ifstream f;
    f.open(strFile.c_str());

    // skip first three lines
    string s0;
    getline(f,s0);
    getline(f,s0);
    getline(f,s0);

    while(!f.eof())
    {
        string s;
        getline(f,s);
        if(!s.empty())
        {
            stringstream ss;
            ss << s;
            double t;
            string sRGB;
            ss >> t;
            vTimestamps.push_back(t);
            ss >> sRGB;
            vstrImageFilenames.push_back(sRGB);
        }
    }
}

string GetDatasetName(const string &strSequencePath) 
{
    string s(strSequencePath);
    std::string delimiter = "/";

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        s.erase(0, pos + delimiter.length());
    }

    if (s.length() == 0)
        return token;
    else
        return s;
}