// Sim7X00Unit.h

// ReSharper disable CppInconsistentNaming
#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <HardwareSerial.h>

#include "Geolocation.h"
#include <stdint.h>
#include <math.h>
#include "new.h"

using std::size_t;

template <typename T, size_t Size>
std::size_t GetArrLength(T (&)[Size])
{
	return Size;
}

template <typename T, size_t N>
void* clear(T (&rg)[N], int value = '\0')
{
	return memset(rg, value, N);
}

class Sim7X00Unit
{
private:
	HardwareSerial* _serial;

public:
	Sim7X00Unit();
	~Sim7X00Unit();
	bool Init(HardwareSerial* serial, uint8_t pwrKey = 2);
	bool SendSms(const char* number, const char* msg);

	uint8_t SendATCommand(const char* command, const char* expectedAnswer, unsigned int timeout);
	char    SendATCommand2(const char* command, const char* expectedAnswer, const char* expectedAnswer2, unsigned int timeout);

	bool GPSPositioning(Geolocation* geo);
};
