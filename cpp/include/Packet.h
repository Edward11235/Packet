#ifndef PACKET_H__
#define PACKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

class PacketSerializer {

	private:
		const char *	addr;
		unsigned short	port;
		int				socket;

	public:
				PacketSerializer(const char * addr, unsigned short port);
			   ~PacketSerializer();
		int		start();
		int		write(int type, const char * data, int size);
};

class Packet {

	public:
		int type;
		int size;
		char * payload;

};

template <typename T>
class ConcurrentQueue {

	private:
		std::mutex mutex;
		std::condition_variable cond;
		std::queue<T> queue;

	public:
		void 	push(T & obj);
		void	pop(T & obj);
		int 	pop_nowait(T & obj);
		size_t 	size();

};

class PacketDeserializerWorker {

	private:
		int socket;
		ConcurrentQueue<Packet> * pipe;

	public:
				PacketDeserializerWorker(int socket, ConcurrentQueue<Packet> & pipe);
			   ~PacketDeserializerWorker();
		void 	run();

};

class PacketDeserializer {

	private:
		unsigned short port;
		int listener;
		ConcurrentQueue<Packet> pipe;
		std::thread * listenerThread;
		void	run();

	public:
				PacketDeserializer(unsigned short port);
			   ~PacketDeserializer();
		int 	start();
		void 	read(Packet & packet);

};

#endif
