/*
 * MainClass.cpp
 *
 *  Created on: Sep 30, 2015
 *      Author: supreeth
 */

#include "MainClass.h"
#include "UdpServer.h"
#include<iostream>
#include <cstdlib>
using namespace std;
int main(int argc, char *argv[]) {

	int sfd = 0;
	int portnum = 8080;
	int recvWindow = 0;
	char *messageRecv;
	if (argc < 3) {
		cout << "Please provide a port number and receive window" << endl;
		exit(1);
	}
	if (argv[1] != NULL) {
		portnum = atoi(argv[1]);
	}
	if (argv[2] != NULL) {
		recvWindow = atoi(argv[2]);
	}

	UdpServer *udpServer = new UdpServer();
	udpServer->rwnd = recvWindow;
	sfd = udpServer->startServer(portnum);
	messageRecv = udpServer->getRequest(sfd);

	if (udpServer->openFile(messageRecv)) {
		udpServer->startFileTransfer();
	} else {
		udpServer->sendError();
	}

	return 1;
}
