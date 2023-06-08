#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Packet.h"

PacketSerializer::PacketSerializer(const char * addr, unsigned short port) {
	this->addr = addr;
	this->port = port;
	this->socket = -1;
}

int PacketSerializer::start() {
	std::cout << "#################################" << std::endl;
	int status;
	int ref = ::socket(AF_INET, SOCK_STREAM, 0);
	if(ref < 0) return -1; // Unable to initialized socket
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->port);
	status = inet_pton(AF_INET, this->addr, &addr.sin_addr);
	if(status <= 0) return -2; // Address not supported
	status = connect(ref, (struct sockaddr *)&addr, sizeof(addr));
	if(status < 0) return -3; // Connection failed
	this->socket = ref;
	return 0; // Success
}

PacketSerializer::~PacketSerializer() {
	if(this->socket < 0) return;
	close(this->socket);
}

int PacketSerializer::write(int type, const char * data, int size) {
	unsigned char buf1[10], buf2[2];
	// TODO: checksum calculation
	unsigned short chksum1 = 0xABCD;
	unsigned short chksum2 = 0xDCBA;
	*((unsigned int   *)&buf1[0]) = htonl((unsigned int)type);
	*((unsigned int   *)&buf1[4]) = htonl((unsigned int)size);
	*((unsigned short *)&buf1[8]) = htons(chksum1);
	*((unsigned short *)&buf2[0]) = htons(chksum2);
	send(this->socket, buf1, sizeof(buf1), 0);
	send(this->socket, data, size,         0);
	send(this->socket, buf2, sizeof(buf2), 0);
	return 0; // Success
}


template <typename T>
void ConcurrentQueue<T>::push(T & obj) {
	std::unique_lock<std::mutex> lockobj(this->mutex);
	this->queue.push(obj);
	this->cond.notify_one();
}

template <typename T>
void ConcurrentQueue<T>::pop(T & obj) {
	std::unique_lock<std::mutex> lockobj(this->mutex);
	this->cond.wait(lockobj, [this]() {
		return !this->queue.empty();
	});
	obj = this->queue.front();
	this->queue.pop();
}

template <typename T>
int ConcurrentQueue<T>::pop_nowait(T & obj) {
	std::unique_lock<std::mutex> lockobj(this->mutex);
	if(this->queue.empty()) return 0;
	obj = this->queue.front();
	this->queue.pop();
	return 1;
}

template <typename T>
size_t ConcurrentQueue<T>::size() {
	std::unique_lock<std::mutex> lockobj(this->mutex);
	return this->queue.size();
}


PacketDeserializerWorker::PacketDeserializerWorker(int socket, ConcurrentQueue<Packet> & pipe) {
	this->socket = socket;
	this->pipe = &pipe;
}

PacketDeserializerWorker::~PacketDeserializerWorker() {
	if(this->socket < 0) return;
	close(this->socket);
}

void PacketDeserializerWorker::run() {
#ifdef DEBUG
	using namespace std;
#endif
	struct {
		int type;
		int size;
		unsigned short chksum;
	}
	// TODO: change to plain array to avoid
	// packed issues on some ARM platforms
	__attribute__((packed)) header;
	while(true) {
		int status;
		status = read(this->socket, &header, sizeof(header));
		if(status <= 0) {
#ifdef DEBUG
			cout << "[B] Header read fail" << endl;
#endif
			break;
		}
		if(status != sizeof(header)) {
#ifdef DEBUG
			cout << "[C] Invalid hader" << endl;
#endif
			continue;
		}
		header.type = ntohl(header.type);
		header.size = ntohl(header.size);
		header.chksum = ntohs(header.chksum);
		if(header.chksum != 0xABCD) {
#ifdef DEBUG
			cout << "[C] Invalid header chksum" << endl;
#endif
			continue;
		}
		if(header.type < 0) {
#ifdef DEBUG
			cout << "[B] Header type signal" << endl;
#endif
			break;
		}
		char * buffer = new char[header.size];
		status = read(this->socket, buffer, header.size);
		if(status <= 0) {
#ifdef DEBUG
			cout << "[B] Payload read fail" << endl;
#endif
			delete[] buffer;
			break;
		}
		if(status != header.size) {
#ifdef DEBUG
			cout << "[C] Invalid payload" << endl;
#endif
			delete[] buffer;
			continue;
		}
		unsigned short chksum2;
		status = read(this->socket, &chksum2, sizeof(chksum2));
		chksum2 = ntohs(chksum2);
		if(chksum2 != 0xDCBA) {
#ifdef DEBUG
			cout << "[C] Invalid payload chksum" << endl;
#endif
			delete[] buffer;
			continue;
		}
		Packet * packet = new Packet;
		packet->type = header.type;
		packet->size = header.size;
		packet->payload = buffer;
		this->pipe->push(*packet);
#ifdef DEBUG
		cout << "[V] Enqueued valid packet type " << packet->type << " sized " << packet->size << endl;
#endif
	}
#ifdef DEBUG
	cout << "[E] Thread death" << endl;
#endif
	close(this->socket);
	this->socket = -1;
}



PacketDeserializer::PacketDeserializer(unsigned short port) {
	this->port = port;
	this->listener = -1;
	this->listenerThread = NULL;
}

PacketDeserializer::~PacketDeserializer() {
	if(this->listenerThread);
		delete this->listenerThread;
	while(this->pipe.size() > 0) {
		Packet * obj;
		this->pipe.pop(*obj);
		if(obj->payload)
			delete[] obj->payload;
		delete obj;
	}
	if(this->listener > 0)
		shutdown(this->listener, SHUT_RDWR);
	std::cout << "Main destruction" << std::endl;
}

int PacketDeserializer::start() {
	int status;
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0) return -1; // Listener creation failed
	struct sockaddr_in addr;
	int opt = 1;
	status = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	if(status) return -2; // Listener settings failed
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(this->port);
	status = bind(listener, (struct sockaddr*)&addr, sizeof(addr));
	if(status < 0) return -3; // Listener binding failed
	status = listen(listener, 5); // Backlog 5 connections
	if(status < 0) return -4; // Listener listening failed
	this->listener = listener;
	this->listenerThread = new std::thread(&PacketDeserializer::run, this);
	return 0;
}

void PacketDeserializer::run() {
#ifdef DEBUG
	using namespace std;
#endif
	while(true) {
		struct sockaddr_in addr;
		int addrlen = sizeof(addr);
		int socket = accept(this->listener, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
		if(socket < 0) {
#ifdef DEBUG
			cout << "[C] Failed to accept connection" << endl;
#endif
			continue;
		}
#ifdef DEBUG
			cout << "[X] Starting worker" << endl;
#endif
		PacketDeserializerWorker * worker = new PacketDeserializerWorker(socket, this->pipe);
		worker->run();
		delete worker;
	}
#ifdef DEBUG
	cout << "[E] Thread death" << endl;
#endif
}

void PacketDeserializer::read(Packet & obj) {
	this->pipe.pop(obj);
}
