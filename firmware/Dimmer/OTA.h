// OTA.h

#ifndef _OTA_h
#define _OTA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

class OTAClass
{
 protected:


 public:
	 void init(const char *HostName, uint16_t Port = 3232);

	 void run();

};

extern OTAClass OTA;

#endif

