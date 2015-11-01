/*
 * UdpServer.h
 *
 *  Created on: Sep 30, 2015
 *      Author: supreeth
 */

#include "Datagram.h"
#include "SlidingWindow.h"
#include "PacketStats.h"
#include "Logger.h"
#include <netinet/in.h>
#include <iostream>
#include <fstream>

#ifndef UDPSERVER_H_
#define UDPSERVER_H_

using namespace std;

class UdpServer {
	int sockfd;

	fstream file;
	SlidingWindow *slidingWindow;
	PacketStats *packetStats;
	Logger *logger;
	struct sockaddr_in cli_address;
	int initialSeqNo;
	int fileLength;
	double estimatedRtt;
	double devRTT;
	double estimatedTimeout;
public:
	UdpServer();
	int rwnd;
	int cwnd;
	int ssthresh;
	int startByte;
	bool isSlowStart;
	bool isCongAvd;
	bool isFastRecovery;
	int startServer(int portNum);
	void send();
	void sendPacket(int seqNo,int startByte);

	void calculateRttAndTime(struct timeval startTime,struct timeval endTime);

	char *getRequest(int clientSockFd);
	bool openFile(string fileName);
	void startFileTransfer();
	void sendError();
	void retransmitSegmentAt(int indexNumber);
	void readFileAndSend(bool finFlag,int startByte, int endByte);
	void sendDatagram(Datagram *datagram);
	void waitForAck();
};

#endif /* UDPSERVER_H_ */
