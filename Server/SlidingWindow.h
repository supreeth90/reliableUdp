/*
 * SlidingWindow.h
 *
 *  Created on: Oct 6, 2015
 *      Author: supreeth
 */

#ifndef SLIDINGWINDOW_H_
#define SLIDINGWINDOW_H_

#include <vector>
#include "SlidingWindowBuffer.h"

using namespace std;

class SlidingWindow {
public:
	SlidingWindow();
	virtual ~SlidingWindow();
	int lastPacketSent;
	int lastAckedPacket;
	int advertisedWindow;
	int sendBase;
	int dupAck;

	vector<SlidingWindowBuffer> slidingWindowBufferVector;

	int addToBuffer(SlidingWindowBuffer slidingWindowBuffer);
};


#endif /* SLIDINGWINDOW_H_ */
