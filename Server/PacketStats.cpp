/*
 * PacketStats.cpp
 *
 *  Created on: Oct 11, 2015
 *      Author: supreeth
 */

#include "PacketStats.h"

PacketStats::PacketStats() {
	// TODO Auto-generated constructor stub
	slowStartPacketSentCount=0;
	slowStartPacketRxCount=0;
	congAvdPacketSentCount=0;
	congAvdPacketRxCount=0;
	retransmitCount=0;
}

PacketStats::~PacketStats() {
	// TODO Auto-generated destructor stub
}

