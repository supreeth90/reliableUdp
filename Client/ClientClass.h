/*
 * ClientClass.h
 *
 *  Created on: Oct 1, 2015
 *      Author: supreeth
 */

#include "Datagram.h"
#include "Logger.h"
#include <unistd.h>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef CLIENTCLASS_H_
#define CLIENTCLASS_H_

using namespace std;
class ClientClass {
private:
public:
	int sockfd;
	struct sockaddr_in server_address;
	Logger *logger;
	int seqNo;
	int ackNo;
	short length ;
	int initialSeqNo;
	bool isPacketDropEnabled;
	bool isDelayEnabled;
	int probValue;

	int lastInOrderPacket;
	int lastPacketReceived;
	int receiverWindow;
	vector<Datagram> datagramVector;
	int addToDatagramVector(Datagram datagram);
	void sendFileRequest(string fileName);
	ClientClass();
	void createSocketAndServerConnection(string serverAddress, string portNumString);
	void sendAck(int ackNumber);


	void insertAt(int index, Datagram datagram);
};

#endif /* CLIENTCLASS_H_ */
