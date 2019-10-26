// web_server.h

#ifndef _WEB_SERVER_h
#define _WEB_SERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <WiFi.h>
#include <WiFiUdp.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

class Web_serverClass
{
 protected:
	 // Create AsyncWebServer object on port 80


 public:
	void begin(uint16_t Port);
	void OnNewCmd(void(*handler)(String));
	void run();
};

extern Web_serverClass Web_server;

#endif

