/*
 * DeltaTimer.cpp
 *
 *  Created on: 13.04.2019
 *      Author: pmale
 */

#include "DeltaTimer.hpp"

DeltaTimer::DeltaTimer()
{
	startTime = Timer::getMillis();
	timeStep = 0;
}

void DeltaTimer::setTimeStep(int32_t milliseconds)
{
	reset();
	timeStep = milliseconds;
}

void DeltaTimer::reset()
{
	startTime = Timer::getMillis();
}

bool DeltaTimer::isTimeUp()
{
	if(timeStep == -1)
	{
		return false; //endless
	}
	if(Timer::getMillis()-startTime > (uint32_t)timeStep)
	{
		startTime = Timer::getMillis();
		return true;
	}
	else
	{
		return false;
	}
}
