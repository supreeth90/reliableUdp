/*
 * UdpServer.cpp
 *
 *  Created on: Sep 30, 2015
 *      Author: supreeth
 */

#include "UdpServer.h"
#include "Logger.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <bitset>
#include <vector>
#include <fstream>
#include <time.h>
#include <cmath>
#include <strings.h>
#include <sstream>

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

using namespace std;

UdpServer::UdpServer() {
	slidingWindow = new SlidingWindow();
	packetStats = new PacketStats();
	logger = new Logger();
	sockfd = 0;
	estimatedRtt = 20000;
	estimatedTimeout = 0;
	devRTT = 0;
}
int UdpServer::startServer(int portNum) {
	int sfd;
	struct sockaddr_in server_addr;
	logger->logDebug(SSTR("Starting the webserver... port: " <<portNum));
	cout << "Starting the Reliable UDP based File Server at port:" << portNum
			<< endl;
	sfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sfd < 0) {
		cout << "Socket open failure" << endl;
		exit(0);
	}
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(portNum);

	/**binding socket to the required port**/
	if (bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		cout << "binding error";
		exit(0);
	}
	cout << "**Server Binding set to :" << server_addr.sin_addr.s_addr << endl;
	cout << "**Server Binding set to port:" << server_addr.sin_port << endl;
	cout << "**Server Binding set to family:" << server_addr.sin_family << endl;
	logger->logDebug(SSTR("Reliable UDP FileServer started successfully"));
	cout << "Reliable UDP FileServer started successfully" << endl;
	sockfd = sfd;
	return sfd;
}

bool UdpServer::openFile(string fileName) {
	cout << "Opening the file " << fileName << endl;
	file.open(fileName.c_str(), ios::in);
	if (!this->file.is_open()) {
		cout << "File: " << fileName << " opening failed" << endl;
		return false;
	} else {
		cout << "File: " << fileName << " opening success" << endl;
		return true;
	}
}

void UdpServer::startFileTransfer() {
	cout << "Starting the file transfer " << endl;
	initialSeqNo = 127;

	/* Calulate the fileLength */
	file.seekg(0, ios::end);
	fileLength = file.tellg();
	file.seekg(0, ios::beg);

	/*Start sending the file */
	send();
}

void UdpServer::sendError() {
	string fileErrorMessage("FILE NOT FOUND");
	sendto(sockfd, fileErrorMessage.c_str(), fileErrorMessage.size(), 0,
			(struct sockaddr *) &cli_address, sizeof(cli_address));
}

void UdpServer::send() {

	logger->logDebug("Entering Send()");
	estimatedTimeout = 40000;				//initializing estimatedTimeout
	ssthresh = 64;
	devRTT = 0;
	cwnd = 1;
	int thisrun = 1;
	int thisrunLimit = 1;
	startByte = 0;
	isSlowStart = true;
	isCongAvd = false;
	isFastRecovery = false;
	struct timeval startTime;
	struct timeval endTime;
	gettimeofday(&startTime, NULL);

	if (slidingWindow->lastPacketSent == -1) {
		startByte = 0;
	}

	logger->logDebug("Before transfer starting in Send()");
	while (startByte <= fileLength) {

		fd_set rfds;
		struct timeval tv;
		int retval;

		thisrun = 1;
		thisrunLimit = min(rwnd, cwnd);

		logger->logDebug(SSTR("WINDOW START::"));
		logger->logDebug(
				SSTR(
						"Before the window in Send() rwnd:" << rwnd << " cwnd:" << cwnd << " window capacity:" << slidingWindow->lastPacketSent - slidingWindow->lastAckedPacket));
		while (slidingWindow->lastPacketSent - slidingWindow->lastAckedPacket
				<= min(rwnd, cwnd) && thisrun <= thisrunLimit) {

			sendPacket(startByte + initialSeqNo, startByte);

			if (isSlowStart) {
				logger->logDebug(SSTR("Window In SLOW START Window"));
				packetStats->slowStartPacketSentCount++;
			} else if (isCongAvd) {
				logger->logDebug(SSTR("Window In CONG AVD Window"));
				packetStats->congAvdPacketSentCount++;
			}

			startByte = startByte + MAX_DATA_SIZE;
			if (startByte > fileLength) {
				cout << "No more data left to be sent" << endl;
				break;
			}
			thisrun++;
		}
		logger->logDebug(SSTR("WINDOW END::"));
		//Start the timer and wait for anydata in sockfd

		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);
		/* Wait up to estimatedTimeout seconds */
		tv.tv_sec = (long) estimatedTimeout / 1000000;
		tv.tv_usec = estimatedTimeout - tv.tv_sec;

		logger->logDebug(SSTR("ENTERING SELECT::" << estimatedTimeout));
		while (1) {
			retval = select(sockfd + 1, &rfds, NULL, NULL, &tv);
			if (retval == -1) {
				cout << "Error in select" << endl;
			} else if (retval > 0) { // ACK available event
				waitForAck();

				if (cwnd >= ssthresh) {
					logger->logDebug(SSTR("CHANGE TO CONG AVD"));
					isCongAvd = true;
					isSlowStart = false;

					cwnd = 1;
					ssthresh = 64;
				}

				if (slidingWindow->lastAckedPacket
						== slidingWindow->lastPacketSent) {
					if (isSlowStart) {
						cwnd = cwnd * 2;
					} else {
						cwnd = cwnd + 1;
					}
					break;
				}
			} else { // Timeout event
				logger->logDebug(
						SSTR("Timeout occurred SELECT::" << estimatedTimeout));
				logger->logDebug(SSTR("CHANGE TO SLOW START"));
				ssthresh = cwnd / 2;
				if (ssthresh < 1) {
					ssthresh = 1;
				}
				cwnd = 1;

				if (isFastRecovery) {
					logger->logDebug(
							SSTR(
									"CHANGE TO SLOW START from FastRecovery startByte:" << startByte));
					isFastRecovery = false;
				}
				isSlowStart = true;
				isCongAvd = false;

				//Timeout exit event when time is 0
				if (tv.tv_sec == 0 && tv.tv_usec == 0) {
					//Retransmit All unacked segments
					if (slidingWindow->lastAckedPacket
							!= slidingWindow->lastPacketSent) {
						int retransmitStartByte = 0;
						if (slidingWindow->lastAckedPacket != -1) {
							retransmitStartByte =
									slidingWindow->slidingWindowBufferVector[slidingWindow->lastAckedPacket].firstByte
											+ MAX_DATA_SIZE;
						}
						logger->logDebug(
								SSTR(
										"Timeout Retransmit seqNo" << retransmitStartByte+initialSeqNo));
						retransmitSegmentAt(retransmitStartByte);
						packetStats->retransmitCount++;
						cout << "Timeout: retransmission at "
								<< retransmitStartByte << endl;

					}
				}
				break;
			}
		}
		logger->logDebug(SSTR("EXIT SELECT::"));
		logger->logDebug(
				SSTR(
						"current byte ::" << startByte << " fileLength " << fileLength));
	}

	gettimeofday(&endTime, NULL);

	long totalTime = (endTime.tv_sec * 1000000 + endTime.tv_usec)
			- (startTime.tv_sec * 1000000 + startTime.tv_usec);

	int totalPacketSent = packetStats->slowStartPacketSentCount
			+ packetStats->congAvdPacketSentCount;
	cout << "Total Time taken " << (float) totalTime / pow(10, 6) << " secs"
			<< endl;
	cout << "Final stats:: Tx Slow start "
			<< packetStats->slowStartPacketSentCount << " CongAvd "
			<< packetStats->congAvdPacketSentCount << endl;
	cout << "Final stats:: Slow start %"
			<< ((float) packetStats->slowStartPacketSentCount / totalPacketSent)
					* 100 << " CongAvd %"
			<< ((float) packetStats->congAvdPacketSentCount / totalPacketSent)
					* 100 << endl;
	cout << "Final stats:: Retransmissions " << packetStats->retransmitCount
			<< endl;

}

void UdpServer::sendPacket(int seqNo, int startByte) {
	bool lastPacket = false;
	int dataLength = 0;
	if (fileLength <= startByte + MAX_DATA_SIZE) {
		cout << "Last packet to be sent" << endl;
		dataLength = fileLength - startByte;
		lastPacket = true;
	} else {
		dataLength = MAX_DATA_SIZE;
	}

	struct timeval time;

	gettimeofday(&time, NULL);

	if (slidingWindow->lastPacketSent != -1
			&& startByte
					< slidingWindow->slidingWindowBufferVector[slidingWindow->lastPacketSent].firstByte) {

		for (int i = slidingWindow->lastAckedPacket + 1;
				i < slidingWindow->lastPacketSent; i++) {
			if (slidingWindow->slidingWindowBufferVector[i].firstByte
					== startByte) {
				slidingWindow->slidingWindowBufferVector[i].timeSent = time;
				break;
			}
		}

	} else {

		SlidingWindowBuffer *slidingWindowBuffer = new SlidingWindowBuffer();
		slidingWindowBuffer->firstByte = startByte;
		slidingWindowBuffer->dataLength = dataLength;
		slidingWindowBuffer->seqNo = initialSeqNo + startByte;
		struct timeval time;
		gettimeofday(&time, NULL);
		slidingWindowBuffer->timeSent = time;
		slidingWindow->lastPacketSent = slidingWindow->addToBuffer(
				*slidingWindowBuffer);
	}
	readFileAndSend(lastPacket, startByte, startByte + dataLength);
}

void UdpServer::waitForAck() {
	unsigned char buffer[MAX_PACKET_SIZE];
	bzero(buffer, MAX_PACKET_SIZE);
	socklen_t addr_size;
	struct sockaddr_in client_address;
	addr_size = sizeof(client_address);
	int n = 0;
	int ackNo;
	while ((n = recvfrom(sockfd, buffer, MAX_PACKET_SIZE, 0,
			(struct sockaddr *) &client_address, &addr_size)) <= 0)
		;
	Datagram *ackDatagram = new Datagram();
	ackDatagram->deserializeToDatagram(buffer, n);

	logger->logDebug(SSTR("ACK Received: ackNo "<< ackDatagram->ackNumber));

	vector<SlidingWindowBuffer> slidingWindowBufferVector =
			slidingWindow->slidingWindowBufferVector;
	SlidingWindowBuffer lastPacketAckedBuffer =
			slidingWindowBufferVector[slidingWindow->lastAckedPacket];
	if (ackDatagram->ackFlag) {
		if (ackDatagram->ackNumber == slidingWindow->sendBase) { // DUP ACK case

			logger->logDebug(
					SSTR("DUP ACK Received: ackNo "<< ackDatagram->ackNumber));
			slidingWindow->dupAck++;
			if (slidingWindow->dupAck == 3) {
				packetStats->retransmitCount++;
				logger->logDebug(
						SSTR(
								"Fast Retransmit seqNo" << ackDatagram->ackNumber));
				retransmitSegmentAt(ackDatagram->ackNumber - initialSeqNo);
				slidingWindow->dupAck = 0;
				if (cwnd > 1) {
					cwnd = cwnd / 2;
				}
				ssthresh = cwnd;
				isFastRecovery = true;
				logger->logDebug(
						SSTR(
								"Change to fast Recovery ackDatagram->ackNumber:" << ackDatagram->ackNumber));
			}

		} else if (ackDatagram->ackNumber > slidingWindow->sendBase) { // true and new ack

			if (isFastRecovery) { // Fast Recovery to Cong Avoidance mode as we have a new ack
				logger->logDebug(
						SSTR(
								"Change to Cong Avoidance from fast recovery recv ack:" << ackDatagram->ackNumber));
				cwnd++;
				isFastRecovery = false;
				isCongAvd = true;
				isSlowStart = false;
			}

			slidingWindow->dupAck = 0;
			slidingWindow->sendBase = ackDatagram->ackNumber;
			if (slidingWindow->lastAckedPacket == -1) { //No Acks yet
				slidingWindow->lastAckedPacket = 0;
				lastPacketAckedBuffer =
						slidingWindowBufferVector[slidingWindow->lastAckedPacket];
			}
			ackNo = lastPacketAckedBuffer.seqNo
					+ lastPacketAckedBuffer.dataLength;
			while (ackNo < ackDatagram->ackNumber) {
				slidingWindow->lastAckedPacket++;
				lastPacketAckedBuffer =
						slidingWindowBufferVector[slidingWindow->lastAckedPacket];
				ackNo = lastPacketAckedBuffer.seqNo
						+ lastPacketAckedBuffer.dataLength;
			}

			struct timeval startTime = lastPacketAckedBuffer.timeSent;
			struct timeval endTime;
			gettimeofday(&endTime, NULL);

			logger->logDebug(
					SSTR(
							"SEQ No of lastACked " << lastPacketAckedBuffer.seqNo));
			calculateRttAndTime(startTime, endTime);

		}
		cout << "slidingWindow->lastAckedPacket"
				<< slidingWindow->lastAckedPacket << endl;
		estimatedTimeout = rand() % 40000;
	}
}

void UdpServer::calculateRttAndTime(struct timeval startTime,
		struct timeval endTime) {

	if (startTime.tv_sec == 0 && startTime.tv_usec == 0) {
		return;
	}
	long sampleRtt = (endTime.tv_sec * 1000000 + endTime.tv_usec)
			- (startTime.tv_sec * 1000000 + startTime.tv_usec);

	estimatedRtt = 0.875 * estimatedRtt + 0.125 * sampleRtt;
	devRTT = 0.75 * devRTT + 0.25 * (abs(estimatedRtt - sampleRtt));
	estimatedTimeout = estimatedTimeout + 4 * devRTT;
}

void UdpServer::retransmitSegmentAt(int firstByte) {

	//Needed to make retransmitted packets not to be considered for timeout calculation
	for (int i = slidingWindow->lastAckedPacket + 1;
			i < slidingWindow->lastPacketSent; i++) {
		if (slidingWindow->slidingWindowBufferVector[i].firstByte
				== firstByte) {
			struct timeval timeSent =
					slidingWindow->slidingWindowBufferVector[i].timeSent;
			timeSent.tv_sec = 0;
			timeSent.tv_usec = 0;
			slidingWindow->slidingWindowBufferVector[i].timeSent = timeSent;
			break;
		}
	}

	readFileAndSend(false, firstByte, firstByte + MAX_DATA_SIZE);
}

void UdpServer::readFileAndSend(bool finFlag, int startByte, int endByte) {
	int datalength = endByte - startByte;
	if (fileLength - startByte < datalength) {
		datalength = fileLength - startByte;
		finFlag = true;
	}
	char *fileData = (char *) calloc(datalength, sizeof(char));
	if (!file.is_open()) {
		cout << "File not opened" << endl;
		return;
	}
	file.seekg(startByte);
	file.read(fileData, datalength);

	Datagram *datagram = new Datagram();
	datagram->seqNumber = startByte + initialSeqNo;
	datagram->ackNumber = 0;
	datagram->ackFlag = false;
	datagram->finFlag = finFlag;
	datagram->length = datalength;
	datagram->data = fileData;

	sendDatagram(datagram);
	logger->logDebug(SSTR("Packet Sent:SeqNo: " << datagram->seqNumber));

}

char *UdpServer::getRequest(int clientSockFd) {
	cout << "***processing file requests for " << clientSockFd << endl;
	char *buffer = (char *) calloc(MAX_PACKET_SIZE, sizeof(char));
	struct sockaddr_in client_address;
	socklen_t addr_size;
	bzero(buffer, MAX_PACKET_SIZE);
	addr_size = sizeof(client_address);
	/* Read from the socket for new HTTP Requests */
	while (recvfrom(clientSockFd, buffer, MAX_PACKET_SIZE, 0,
			(struct sockaddr *) &client_address, &addr_size) <= 0)
		;
	cout << "**Request received is::" << buffer << endl;
	cli_address = client_address;
	return buffer;
}

void UdpServer::sendDatagram(Datagram *datagram) {
	char *datagramChars = datagram->serializeToCharArray();
	sendto(sockfd, datagramChars, MAX_PACKET_SIZE, 0,
			(struct sockaddr *) &cli_address, sizeof(cli_address));
}

