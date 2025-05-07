// Geolocation.h

#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
	#include "WProgram.h"
#endif

struct Geolocation
{
public:
	float latitude;
	float longitude;
	String date;
	String utc;
};

