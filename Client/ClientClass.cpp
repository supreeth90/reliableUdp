/*
 * ClientClass.cpp
 *
 *  Created on: Oct 1, 2015
 *      Author: supreeth
 */

#include <iostream>
#include "ClientClass.h"
#include "Datagram.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <bitset>
#include <fstream>
#include <sstream>

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

using namespace std;

ClientClass::ClientClass() {
	lastInOrderPacket = -1;
	lastPacketReceived = -1;
	logger = new Logger();
}

void ClientClass::sendFileRequest(string fileName) {
	int n;
	int nextSeqExpected;
	int segmentsInBetween = 0;
	initialSeqNo = 127;
	if (receiverWindow == 0) {
		receiverWindow = 100;
	}
	unsigned char *buffer = (unsigned char *) calloc(MAX_PACKET_SIZE,
			sizeof(unsigned char));
	cout << "server_add::" << server_address.sin_addr.s_addr << endl;
	cout << "server_add_port::" << server_address.sin_port << endl;
	cout << "server_add_family::" << server_address.sin_family << endl;
	n = sendto(sockfd, fileName.c_str(), fileName.size(), 0,
			(struct sockaddr *) &(server_address), sizeof(struct sockaddr_in));
	if (n < 0) {
		printf("ERROR writing to socket\n");
	}
	bzero(buffer, MAX_PACKET_SIZE);

	fstream file;
	file.open(fileName.c_str(), ios::out);

	while ((n = recvfrom(sockfd, buffer, MAX_PACKET_SIZE, 0, NULL,
	NULL)) > 0) {

		char buffer2[20];
		memcpy(buffer2, buffer, 20);
		if (strstr("FILE NOT FOUND", buffer2) != NULL) {
			cout << "File not found" << endl;
			return;
		}

		Datagram *datagram = new Datagram();
		datagram->deserializeToDatagram(buffer, n);

		logger->logDebug(
				SSTR("packet received with seqNo:" << datagram->seqNumber));

		//Random drop Implementation
		if (isPacketDropEnabled && rand() % 100 < probValue) {
			logger->logDebug(
					SSTR(
							"Dropping this packet with seq " << datagram->seqNumber));
			continue;
		}

		//Random Delay Implementation
		if (isDelayEnabled && rand() % 100 < probValue) {
			int sleeptime = (rand() % 10) * 1000;
			logger->logDebug(
					SSTR(
							"Delaying this packet with seq " << datagram->seqNumber << " for " << sleeptime));
			usleep(sleeptime);
		}

		if (lastInOrderPacket == -1) { //First packet case
			nextSeqExpected = initialSeqNo;
		} else {
			nextSeqExpected = datagramVector[lastInOrderPacket].seqNumber
					+ datagramVector[lastInOrderPacket].length;
		}

		if (nextSeqExpected > datagram->seqNumber) {   //Old packet condition
			sendAck(nextSeqExpected);
		}

		segmentsInBetween = (datagram->seqNumber - nextSeqExpected)
				/ MAX_DATA_SIZE;
		int thisSegmentIndex = lastInOrderPacket + segmentsInBetween + 1;

		if (thisSegmentIndex - lastInOrderPacket > receiverWindow) {
			cout << "Packet dropped " << thisSegmentIndex << endl;
			// Drop the packet if it exceeds receiver window
			continue;
		}

		//accept the packet inorder or out-of-order
		insertAt(thisSegmentIndex, *datagram);

		//Push inorder data to the application/file
		for (int i = lastInOrderPacket + 1; i <= lastPacketReceived; i++) {
			if (datagramVector[i].seqNumber != -1) {
				if (file.is_open()) {
					if (lastInOrderPacket != -1) {
						file << datagramVector[lastInOrderPacket].data;
					}
					lastInOrderPacket++;
				}
			} else {
				break;
			}
		}
		sendAck(
				datagramVector[lastInOrderPacket].seqNumber
						+ datagramVector[lastInOrderPacket].length);

		if (datagram->finFlag) {
			cout << "Fin Flag received ...Hence writing last packet to a file"
					<< endl;
			if (file.is_open()) {
				if (lastInOrderPacket != -1) {
					file << datagramVector[lastInOrderPacket].data;
				}
			}
			break;
		}
		bzero(buffer, MAX_PACKET_SIZE);   // Reset the buffer to zero
	}
	free(buffer);
	file.close();
}

int ClientClass::addToDatagramVector(Datagram datagram) {
	datagramVector.push_back(datagram);
	return datagramVector.size() - 1;
}

void ClientClass::sendAck(int ackNumber) {
	cout << "Sending an ack :" << ackNumber << endl;
	int n = 0;
	Datagram *ackDatagram = new Datagram();
	ackDatagram->ackFlag = true;
	ackDatagram->ackNumber = ackNumber;
	ackDatagram->finFlag = false;
	ackDatagram->length = 0;
	ackDatagram->seqNumber = 0;

	char *ackDatagramChars = ackDatagram->serializeToCharArray();
	n = sendto(sockfd, ackDatagramChars, MAX_PACKET_SIZE, 0,
			(struct sockaddr *) &(server_address), sizeof(struct sockaddr_in));
	if (n < 0) {
		cout << "Sending ACk failed" << endl;
	}
}

void ClientClass::createSocketAndServerConnection(string serverAddress,
		string portNumString) {
	struct hostent *server;
	struct sockaddr_in server_address;
	int sfd;
	int portNum;
	portNum = atoi(portNumString.c_str());
	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sfd < 0) {
		cout << "ERROR opening socket" << endl;
	}
	server = gethostbyname(serverAddress.c_str());
	if (server == NULL) {
		cout << "ERROR, no such host" << endl;
		exit(0);
	}
	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char *) server->h_addr,
	(char *)&server_address.sin_addr.s_addr,
	server->h_length);
	server_address.sin_port = htons(portNum);

	sockfd = sfd;
	this->server_address = server_address;
}

/**
 * Insert the packets in the appropriate position in the vector based on whether its in-order or out-of-order
 */
void ClientClass::insertAt(int index, Datagram datagram) {
	if (index > lastPacketReceived) {
		for (int i = lastPacketReceived + 1; i <= index; i++) {
			if (datagramVector.size() == index) {
				datagramVector.push_back(datagram);
			} else {
				Datagram datagram;
				datagramVector.push_back(datagram.getNullDatagram());
			}
		}
		lastPacketReceived = index;
	} else {
		datagramVector[index] = datagram;
	}
}

int main(int argc, char *argv[]) {
	cout << "Starting the client" << endl;
	if (argc != 7) {
		cout
				<< "Please provide in this format : <server-ip> <server-port> <file-name> <receiver-window> <extra-param> <drop/delay-%>"
				<< endl;
		exit(1);
	}
	ClientClass *cl1 = new ClientClass();
	string serverIp(argv[1]);
	string portNum(argv[2]);
	string fileName(argv[3]);
	int recvWindow = atoi(argv[4]);
	cl1->receiverWindow = recvWindow;
	int extraParam = atoi(argv[5]);
	if (extraParam == 0) {
		cl1->isDelayEnabled = false;
		cl1->isPacketDropEnabled = false;
	} else if (extraParam == 1) {
		cl1->isPacketDropEnabled = true;
		cl1->isDelayEnabled = false;
	} else if (extraParam == 2) {
		cl1->isPacketDropEnabled = false;
		cl1->isDelayEnabled = true;
	} else if (extraParam == 3) {
		cl1->isPacketDropEnabled = true;
		cl1->isDelayEnabled = true;
	} else {
		cout << "Invalid 4th argument. Should be 0-3" << endl;
		return 0;
	}
	int dropPercentage = atoi(argv[6]);
	cl1->probValue = dropPercentage;

	cl1->createSocketAndServerConnection(serverIp, portNum);
	cl1->sendFileRequest(fileName);

}
