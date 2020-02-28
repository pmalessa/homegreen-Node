/*
 * DeltaTimer.h
 *
 *  Created on: 13.04.2019
 *      Author: pmale
 */

#ifndef DRIVER_DELTATIMER_H_
#define DRIVER_DELTATIMER_H_

#include "PLATFORM.h"
#include "Timer.hpp"

class DeltaTimer
{
public:
	DeltaTimer();
	void setTimeStep(int32_t milliseconds);
	bool isTimeUp();
	void reset();
private:
	uint32_t startTime;
	int32_t timeStep;
};


#endif /* DRIVER_DELTATIMER_H_ */
