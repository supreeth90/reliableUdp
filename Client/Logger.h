/*
 * Logger.h
 *
 *  Created on: Oct 12, 2015
 *      Author: supreeth
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <fstream>
#include <iostream>
#include <string>

using namespace std;
class Logger {
public:
	Logger();
	virtual ~Logger();
	fstream logFile;
	void logDebug(string logText);
};

#endif /* LOGGER_H_ */
