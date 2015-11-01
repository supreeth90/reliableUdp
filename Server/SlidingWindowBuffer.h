/*
 * SlidingWindowBuffer.h
 *
 *  Created on: Oct 6, 2015
 *      Author: supreeth
 */

#include <time.h>
#include <sys/time.h>

#ifndef SLIDINGWINDOWBUFFER_H_
#define SLIDINGWINDOWBUFFER_H_

class SlidingWindowBuffer {
public:
	SlidingWindowBuffer();
	virtual ~SlidingWindowBuffer();
	int firstByte;
	int dataLength;
	int seqNo;
	struct timeval timeSent;
};

#endif /* SLIDINGWINDOWBUFFER_H_ */
