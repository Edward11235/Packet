#include "Packet.h"
#include <iostream>
#include <random>
#include <fstream>
#include <functional>

// Define a CRC32 function for byte arrays
unsigned int crc32(unsigned char* data, size_t size) {
    unsigned int polynomial = 0xEDB88320;
    unsigned int crc = 0xFFFFFFFF;

    for(size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for(int j = 0; j < 8; j++) {
            if(crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xFFFFFFFF;
};

int main(int argc, char **argv) {
    PacketSerializer channel("127.0.0.1", 5001);
    int status = channel.start();
    std::cout << "status is " << status << std::endl;

    unsigned char * message = new unsigned char[500000]; 

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0,255);

    for(int i = 0; i < 500000; i++) {
        char randomChar = static_cast<char>(distribution(generator));
        message[i] = randomChar;
    }

    // Write the message to a file
    std::ofstream outFile("random_message.txt", std::ios::binary);
    if(outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(message), 500000);
        outFile.close();
    } else {
        std::cout << "Unable to open file for writing." << std::endl;
    }

    // Compute CRC32
    unsigned int crc = crc32(message, 500000);
    std::cout << "The CRC32 of the message is: " << std::hex << crc << std::endl;

    // Send the message
    channel.write(4, reinterpret_cast<char*>(message), 500000);

    delete[] message;
    return 0;
}
