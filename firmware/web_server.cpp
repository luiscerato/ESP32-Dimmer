// 
// 
// 

#include "web_server.h"
#include "ac_dimmer.h"

extern AC_Dimmer Dimmer;

AsyncWebServer server(80);
AsyncEventSource events("/events");
AsyncWebSocket ws("/cmds");

const String cmds[] = { "out0", "out1", "out2", "out3", "out4", "out5", "out6", "out7", "out8", "out9" };

void(*OnNewCmdHandler)(String);


void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *in_data, size_t len) {

	if (type == WS_EVT_CONNECT) {
		Serial.println("Websocket client connection received");
	}
	else if (type == WS_EVT_DISCONNECT) {
		Serial.println("Client disconnected");
		Serial.println("-----------------------");
	}
	else if (type == WS_EVT_DATA) {
		Serial.print("Data received: ");

		String Msg;
		for (int i = 0; i < len; i++) {
			Serial.print((char)in_data[i]);
			Msg += (char)in_data[i];
		}
		Serial.println();

		if (OnNewCmdHandler) {
			OnNewCmdHandler(Msg);
		}

		int Index, Val, Ok = 0;
		uint8_t *data = in_data;

		while (Ok==0) {
			if (*data++ != 'o')
				break;
			if (*data++ != 'u')
				break;
			if (*data++ != 't')
				break;
			Index = (*data++);
			if (!isdigit(Index))
				break;
			if (*data++ != '=')
				break;
			Val = atol((char*)data);
			Index -= '0';
			if ((Index >= 0 && Index <= 9) && (Val >= 0 && Val <= 100)) {
				Dimmer.setOutDuty(Index, Val);
				Serial.printf("-- Set out %d a %d\n", Index, Val);
			}
			Ok++;
		}
		data = in_data;
		while (Ok==0) {
			if (*data++ != 'c')
				break;
			if (*data++ != 'm')
				break;
			if (*data++ != 'd')
				break;
			if (*data++ != '=')
				break;
			Index = atol((char*)data);
			Serial.printf("-- load cmd %d\n", Index);
			Ok++;
		}
		if (Ok==0)
			Serial.printf("-- no se reconoce comando\n");
	}
}

void Web_serverClass::begin(uint16_t Port)
{
	// Route for root / web page
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/index.html", "text/html");
	});
	server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/index.html", "text/html");
	});
	server.on("/estado.html", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/estado.html", "text/html");
	});
	server.on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/config.html", "text/html");
	});
	server.on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/favicon.png", "image/png");
	});

	// Route to load style.css file
	server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/style.css", "text/css");
	});

	// Route to load style.css file
	server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/script.js", "text/css");
	});

	////server.serveStatic("/", SPIFFS, "/");
	//server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

	//Enviar dirección IP del servidor
	server.on("/ip", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", WiFi.localIP().toString());
	});

	// Route to set GPIO to HIGH
	server.on("/outs", HTTP_GET, [](AsyncWebServerRequest *request) {

		int paramsNr = request->params();

		if (paramsNr) {
			String param;
			for (int i = 0; i < paramsNr; i++) {
				AsyncWebParameter* p = request->getParam(i);
				Serial.print("Outs->Param: ");
				Serial.print(p->name());
				Serial.print(", args: ");
				Serial.println(p->value());

				for (int x = 0; x < 8; x++) {
					if (p->name() == cmds[x]) {
						int Val = p->value().toInt();
						if (Val >= 0 && Val <= 100) {
							Dimmer.setOutDuty(x, Val);
							Serial.printf("-- Set out %d a %d", x, Val);
						}
					}
				}
			}
		}

		String resp;
		resp = "{\"out1\":" + String(Dimmer.getOutDuty(0));
		resp += ", \"out2\":" + String(Dimmer.getOutDuty(1));
		resp += ", \"out3\":" + String(Dimmer.getOutDuty(2));
		resp += ", \"out4\":" + String(Dimmer.getOutDuty(3));
		resp += ", \"out5\":" + String(Dimmer.getOutDuty(4));
		resp += ", \"out6\":" + String(Dimmer.getOutDuty(5));
		resp += ", \"out7\":" + String(Dimmer.getOutDuty(6));
		resp += ", \"out8\":" + String(Dimmer.getOutDuty(7));
		resp += ", \"out9\":" + String(Dimmer.getOutDuty(8));
		resp += ", \"out10\":" + String(Dimmer.getOutDuty(9)) += "}";
		request->send(200, "text/plain", resp);
		//Serial.println(resp);
	});

	server.onNotFound([](AsyncWebServerRequest *request) {
		Serial.printf("NOT_FOUND: ");
		if (request->method() == HTTP_GET)
			Serial.printf("GET");
		else if (request->method() == HTTP_POST)
			Serial.printf("POST");
		else if (request->method() == HTTP_DELETE)
			Serial.printf("DELETE");
		else if (request->method() == HTTP_PUT)
			Serial.printf("PUT");
		else if (request->method() == HTTP_PATCH)
			Serial.printf("PATCH");
		else if (request->method() == HTTP_HEAD)
			Serial.printf("HEAD");
		else if (request->method() == HTTP_OPTIONS)
			Serial.printf("OPTIONS");
		else
			Serial.printf("UNKNOWN");
		Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

		if (request->contentLength()) {
			Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
			Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
		}

		int headers = request->headers();
		int i;
		for (i = 0;i < headers;i++) {
			AsyncWebHeader* h = request->getHeader(i);
			Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
		}

		int params = request->params();
		for (i = 0;i < params;i++) {
			AsyncWebParameter* p = request->getParam(i);
			if (p->isFile()) {
				Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
			}
			else if (p->isPost()) {
				Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
			}
			else {
				Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
			}
		}

		request->send(404);
	});

	// Start server

	ws.onEvent(onWsEvent);

	server.addHandler(&events);
	server.addHandler(&ws);
	server.begin();

	OnNewCmdHandler = NULL;
}

void Web_serverClass::OnNewCmd(void(*handler)(String))
{
	OnNewCmdHandler = handler;
}


void Web_serverClass::run()
{

}
