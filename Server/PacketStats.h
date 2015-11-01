/*
 * PacketStats.h
 *
 *  Created on: Oct 11, 2015
 *      Author: supreeth
 */

#ifndef PACKETSTATS_H_
#define PACKETSTATS_H_

class PacketStats {
public:
	PacketStats();
	virtual ~PacketStats();
	int slowStartPacketSentCount;
	int congAvdPacketSentCount;
	int slowStartPacketRxCount;
	int congAvdPacketRxCount;
	int retransmitCount;
};

#endif /* PACKETSTATS_H_ */
