/*
 * SlidingWindow.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: supreeth
 */

#include "SlidingWindow.h"

using namespace std;

SlidingWindow::SlidingWindow() {
	lastPacketSent=-1;
	lastAckedPacket=-1;
	advertisedWindow=0;
	sendBase=-1;
	dupAck=0;
}

SlidingWindow::~SlidingWindow() {
	// TODO Auto-generated destructor stub
}

int SlidingWindow::addToBuffer(SlidingWindowBuffer slidingWindowBuffer) {
	slidingWindowBufferVector.push_back(slidingWindowBuffer);
	return (slidingWindowBufferVector.size()-1);
}

