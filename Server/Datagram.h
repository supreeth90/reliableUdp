/*
 * Datagram.h
 *
 *  Created on: Oct 2, 2015
 *      Author: supreeth
 */

#ifndef DATAGRAM_H_
#define DATAGRAM_H_

const int MAX_PACKET_SIZE=1472;
const int MAX_DATA_SIZE=1460;
const int HEADER_LENGTH=12;
using namespace std;

class Datagram {
private:
	int convertToInteger(unsigned char *buffer, int startIndex);
	bool convertToBoolean(unsigned char *buffer, int index);
	short convertToShort(unsigned char *buffer, int startIndex);
	char *getPayload(unsigned char* data, int length);
	char* intToBytes(int paramInt);
public:
		int seqNumber;
		int ackNumber;
		bool ackFlag;
		bool finFlag;
		unsigned short length;
		char *data;
		char* serializeToCharArray();
		void deserializeToDatagram(unsigned char *datagramChars, int length);
		void setDataAndLength(char *data, unsigned short length);

};

#endif /* DATAGRAM_H_ */
