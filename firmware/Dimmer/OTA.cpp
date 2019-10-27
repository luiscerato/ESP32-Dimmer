// 
// 
// 

#include "OTA.h"
#include "ac_dimmer.h"

void OTAClass::init(const char *HostName, uint16_t Port)
{
	// Port defaults to 3232
	ArduinoOTA.setPort(Port);

	// Hostname defaults to esp3232-[MAC]
	if (HostName)
		ArduinoOTA.setHostname(HostName);
	else
		ArduinoOTA.setHostname("myesp32");

	ArduinoOTA
		.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.println("Start updating " + type);
		//Dimmer.stop();
	})
		.onEnd([]() {
		Serial.println("\nEnd");
	})
		.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
	})
		.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});

	ArduinoOTA.begin();

}

void OTAClass::run()
{
	ArduinoOTA.handle();
}

OTAClass OTA;

