/*
 * Datagram.cpp
 *
 *  Created on: Oct 2, 2015
 *      Author: supreeth
 */

#include "Datagram.h"
#include <stdlib.h>
#include <iostream>
#include <string.h>

char* Datagram::serializeToCharArray() {
	char *finalPacket;
	finalPacket = (char *) calloc(MAX_PACKET_SIZE, sizeof(char));

	memcpy(finalPacket, &seqNumber, sizeof(seqNumber));

	memcpy(finalPacket + 4, &ackNumber, sizeof(ackNumber));

	memcpy((finalPacket + 8), &ackFlag, 1);

	memcpy((finalPacket + 9), &finFlag, 1);

	memcpy((finalPacket + 10), &length, sizeof(length));

	memcpy((finalPacket + 12), data, length);
	return finalPacket;
}

void Datagram::setDataAndLength(char *data, unsigned short length) {
	this->data = data;
	this->length = length;
}

void Datagram::deserializeToDatagram(unsigned char *datagramChars,
		int datagramLength) {
	seqNumber = convertToInteger(datagramChars, 0);
	ackNumber = convertToInteger(datagramChars, 4);
	ackFlag = convertToBoolean(datagramChars, 8);
	finFlag = convertToBoolean(datagramChars, 9);
	length = convertToShort(datagramChars, 10);
	data = getPayload(datagramChars, length);
}

int Datagram::convertToInteger(unsigned char *buffer, int startIndex) {
	unsigned int integerValue = (buffer[startIndex + 3] << 24)
			| (buffer[startIndex + 2] << 16) | (buffer[startIndex + 1] << 8)
			| (buffer[startIndex]);
	return integerValue;
}

short Datagram::convertToShort(unsigned char *buffer, int startIndex) {
	short shortValue = (buffer[startIndex + 1] << 8) | (buffer[startIndex]);
	return shortValue;
}

char *Datagram::getPayload(unsigned char* data, int length) {
	char *payload = (char *) calloc(length + 1, sizeof(char));
	memcpy(payload, data + HEADER_LENGTH, length);
	*(payload + length) = '\0';
	return payload;
}
bool Datagram::convertToBoolean(unsigned char *buffer, int index) {
	bool booleanValue = buffer[index];
	return booleanValue;
}
Datagram Datagram::getNullDatagram() {
	Datagram datagram;
	datagram.ackNumber = -1;
	datagram.seqNumber = -1;
	datagram.length = -1;
	return datagram;
}

