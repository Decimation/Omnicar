// 
// 
// 

// ReSharper disable CppMemberFunctionMayBeConst
#include "Sim7X00Unit.h"


Sim7X00Unit::Sim7X00Unit()
{
}

Sim7X00Unit::~Sim7X00Unit()
{
}

bool Sim7X00Unit::Init(HardwareSerial* serial, uint8_t pwrKey)
{
	_serial = serial;
	_serial->begin(115200, SERIAL_8N1);


	auto answer = SendATCommand("AT", C_OK, 2000);
	if (answer == 0) {
		Serial.print("Starting up...\n");

		pinMode(pwrKey, OUTPUT);
		// power on pulse
		digitalWrite(pwrKey, HIGH);
		delay(500);
		digitalWrite(pwrKey, LOW);

		// waits for an answer from the module
		while (answer == 0) {     // Send AT every two seconds and wait for the answer
			answer = SendATCommand("AT", C_OK, 2000);
			delay(1000);
		}
	}

	delay(5000);

	while ((SendATCommand("AT+CREG?", "+CREG: 0,1", 500) || SendATCommand("AT+CREG?", "+CREG: 0,5", 500)) == 0)
		delay(500);

	return true;
}

bool Sim7X00Unit::SendSms(const char* number, const char* msg)
{
	uint8_t answer = 0;
	char    aux_string[30];

	// Serial.print("Setting SMS mode...\n");
	SendATCommand("AT+CMGF=1", C_OK, 1000);    // sets the SMS mode to text
	// Serial.print("Sending Short Message\n");

	sprintf(aux_string, "AT+CMGS=\"%s\"", number);

	answer = SendATCommand(aux_string, ">", 3000);    // send the SMS number
	if (answer == 1) {
		_serial->println(msg);
		_serial->write(0x1A);
		answer = SendATCommand("", C_OK, 20000);
		if (answer == 1) {
			// Serial.print("Sent successfully \n");
			return true;
		} else {
			// Serial.print("error \n");
			return false;
		}
	} else {
		//     Serial.print(answer);
		// Serial.print(" error.\n");
		return false;
	}
}

char Sim7X00Unit::SendATCommand2(const char* command, const char* expectedAnswer, const char* expectedAnswer2, unsigned int timeout)
{
	uint8_t       x = 0, answer = 0;
	char          response[100];
	unsigned long previous;

	//memset(response, '\0', 100);    // Initialize the string
	clear(response);
	delay(100);

	while (_serial->available() > 0)
		_serial->read();    // Clean the input buffer

	_serial->println(command);    // Send the AT command 

	x        = 0;
	previous = millis();

	// this loop waits for the answer
	do {
		// if there are data in the UART input buffer, reads it and checks for the asnwer
		if (_serial->available() != 0) {
			response[x] = _serial->read();
			_serial->print(response[x]);
			x++;
			// check if the desired answer 1  is in the response of the module
			if (strstr(response, expectedAnswer) != nullptr) {
				_serial->print("\n");
				answer = 1;
			}
			// check if the desired answer 2 is in the response of the module
			else if (strstr(response, expectedAnswer2) != nullptr) {
				_serial->print("\n");
				answer = 2;
			}
		}
	}
	// Waits for the asnwer with time out
	while ((answer == 0) && ((millis() - previous) < timeout));

	return answer;
}

uint8_t Sim7X00Unit::SendATCommand(const char* command, const char* expectedAnswer, unsigned int timeout)
{
	uint8_t x = 0, answer = 0;

	char          response[100];
	unsigned long previous;

	// memset(response, '\0', 100);    // Initialize the string
	clear(response);

	delay(100);

	while (_serial->available() > 0)
		_serial->read();    // Clean the input buffer

	_serial->println(command);    // Send the AT command 

	x        = 0;
	previous = millis();

	// this loop waits for the answer
	do {
		if (_serial->available() != 0) {
			// if there are data in the UART input buffer, reads it and checks for the asnwer
			response[x] = _serial->read();
			//            Serial.print(response[x]);
			x++;
			// check if the desired answer  is in the response of the module
			if (strstr(response, expectedAnswer) != nullptr) {
				answer = 1;
			}
		}
		// Waits for the asnwer with time out
	}
	while ((answer == 0) && ((millis() - previous) < timeout));


	return answer;
}


bool Sim7X00Unit::GPSPositioning(Geolocation* geo)
{
	uint8_t answer  = 0;
	bool    recNull = true;
	int     i       = 0;
	char    recMessage[200];
	char    latDd[3], latMm[10], logDd[4], logMm[10], ddMmYy[7], utcTime[7];
	int     dayMonthYear;
	float   lat = 0;
	float   log = 0;

	clear(recMessage);
	clear(latDd);
	clear(latMm);
	clear(logMm);
	clear(ddMmYy);
	clear(utcTime);

	//memset(recMessage, '\0', 200);    // Initialize the string
	//memset(latDd, '\0', 3);    // Initialize the string
	//memset(latMm, '\0', 10);    // Initialize the string
	//memset(logDd, '\0', 4);    // Initialize the string
	//memset(logMm, '\0', 10);    // Initialize the string
	//memset(ddMmYy, '\0', 7);    // Initialize the string
	//memset(utcTime, '\0', 7);    // Initialize the string

	Serial.print("Start GPS session...\n");
	SendATCommand("AT+CGPS=1,1", C_OK, 1000);    // start GPS session, standalone mode

	delay(2000);

	while (recNull) {
		answer = SendATCommand("AT+CGPSINFO", "+CGPSINFO: ", 1000);    // start GPS session, standalone mode

		if (answer == 1) {
			answer = 0;
			while (_serial->available() == 0);
			// this loop reads the data of the GPS
			do {
				// if there are data in the UART input buffer, reads it and checks for the asnwer
				if (_serial->available() > 0) {
					recMessage[i] = _serial->read();
					i++;
					// check if the desired answer (OK) is in the response of the module
					if (strstr(recMessage, C_OK) != nullptr) {
						answer = 1;
					}
				}
			}
			while (answer == 0);    // Waits for the asnwer with time out

			recMessage[i] = '\0';
			_serial->print(recMessage);
			_serial->print("\n");

			if (strstr(recMessage, ",,,,,,,,") != nullptr) {
				//memset(recMessage, '\0', 200);    // Initialize the string
				clear(recMessage);
				i      = 0;
				answer = 0;
				delay(1000);
			} else {
				recNull = false;
				SendATCommand("AT+CGPS=0", "OK:", 1000);
			}
		} else {
			// _serial->print("error \n");
			return false;
		}
		delay(2000);
	}

	strncpy(latDd, recMessage, 2);
	latDd[2] = '\0';
	//    Serial.print("LatDD:");
	//    Serial.print(LatDD);

	strncpy(latMm, recMessage + 2, 9);
	latMm[9] = '\0';
	//    Serial.print(" LatMM:");
	//    Serial.print(LatMM);

	lat = atoi(latDd) + (atof(latMm) / 60);
	if (recMessage[12] == 'N') {
		geo->latitude = lat;
	} else if (recMessage[12] == 'S') {
		geo->latitude = lat;
	} else {
		// return nullptr;
	}

	strncpy(logDd, recMessage + 14, 3);
	logDd[3] = '\0';
	//    Serial.print("LogDD:");
	//    Serial.print(LogDD);

	strncpy(logMm, recMessage + 17, 9);
	logMm[9] = '\0';
	//    Serial.print("LogMM:");
	//    Serial.print(LogMM);

	log = atoi(logDd) + (atof(logMm) / 60);
	if (recMessage[27] == 'E') {
		// Serial.print("Longitude is ");
		// Serial.print(log);
		// Serial.print(" E\n");
		geo->longitude = log;
	} else if (recMessage[27] == 'W') {
		/*Serial.print("Latitude is ");
		Serial.print(lat);
		Serial.print(" W\n");*/
		geo->latitude = lat;//todo: error?
	} else {
		// return nullptr;
	}

	strncpy(ddMmYy, recMessage + 29, 6);
	ddMmYy[6] = '\0';
	// Serial.print("Day Month Year is ");
	// Serial.print(ddMmYy);
	// Serial.print("\n");
	geo->date = String(ddMmYy);
	strncpy(utcTime, recMessage + 36, 6);
	utcTime[6] = '\0';
	/*Serial.print("UTC time is ");
	Serial.print(utcTime);
	Serial.print("\n");*/
	geo->utc = String(utcTime);

	return true;
}

bool Sim7X00Unit::ReceiveSms()
{
	uint8_t answer = 0;
	int     i      = 0;
	char    recMessage[200];

	Serial.print("Setting SMS mode...\n");
	SendATCommand("AT+CMGF=1", C_OK, 1000);    // sets the SMS mode to text
	SendATCommand(R"(AT+CPMS="SM","SM","SM")", C_OK, 1000);    // selects the memory

	answer = SendATCommand("AT+CMGR=1", "+CMGR:", 2000);    // reads the first SMS

	if (answer == 1) {
		answer = 0;
		while (Serial.available() == 0);
		// this loop reads the data of the SMS
		do {
			// if there are data in the UART input buffer, reads it and checks for the asnwer
			if (Serial.available() > 0) {
				recMessage[i] = Serial.read();
				i++;
				// check if the desired answer (OK) is in the response of the module
				if (strstr(recMessage, C_OK) != nullptr) {
					answer = 1;
				}
			}
		}
		while (answer == 0);    // Waits for the asnwer with time out

		//       RecMessage[i] = '\0';

		Serial.print(recMessage);
		Serial.print("\n");
	} else {
		Serial.print(answer);
		Serial.print(" error.\n");
		return false;
	}

	return true;
}
