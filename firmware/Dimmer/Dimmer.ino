#include "tokenizer.h"
#include "web_server.h"
#include "ac_dimmer.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <iostream>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "OTA.h"
#include "ArduinoJson.h"
#include "board.h"
#include "Arduino.h"
#include "ir_remote.h"
#include <PubSubClient.h>
#include "esp_system.h"
#include "esp_wifi.h"


// Your WiFi credentials.
char ssid[32] = "Wifi-Luis";
char pass[32] = "";
char HostName[32] = "Dimmer ESP32";
char Use_Static_IP = 1;
const char* mqtt_server = "192.168.1.120"; // MQTT Broker IP address

IPAddress local_IP(192, 168, 1, 180); // Set your Static IP address
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 1, 1);  // Set your Gateway IP address
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

//MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void Load_Conf();

AC_Dimmer Dimmer;
Web_serverClass Web_server;

ir_remote Remote;

void Init_NeoPixel();
void Color_NeoPixel(uint8_t *color);
void onRemote(uint32_t Code);
void MQTTonNewMessage(char* topic, byte* message, unsigned int length);
void MQTTLoop();
void getMacAddress(char *dest);

void Load_Preset(String Preset);

void setup()
{
	// Debug console
	Serial.begin(115200);
	Serial.printf("Iniciando dimmer...\n");

	// Initialize SPIFFS
	if (!SPIFFS.begin(true)) {
		Serial.println("An Error has occurred while mounting SPIFFS");
	}
	Load_Conf();

	if (Use_Static_IP) {
		if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
			Serial.println("STA Failed to configure");
      esp_wifi_set_ps (WIFI_PS_NONE);
		}
	}

	Board.begin();

	// Connect to Wi-Fi network with SSID and password
	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, pass);
      esp_wifi_set_ps (WIFI_PS_NONE);

	Serial.println("Ready");
	Serial.print("IP address: ");

	OTA.init(HostName);

	Dimmer.begin(0, INPUT_ZCD, Pins_Outs);
	Dimmer.setZeroCrossDelay(150);
	Dimmer.setResolution(256);

	Web_server.begin(80);
	Web_server.OnNewCmd(Load_Preset);

	Remote.begin(INPUT_IRC);
	Remote.addOnKeyPressed(onRemote);

	Serial.printf("Ok!\n\n");

	//  ledcSetup(0, 440, 8);
	//  ledcAttachPin(23, 0);
	//    ledcWrite(0, 127);

	Init_NeoPixel();

	//Configurar MQTT
//	client.setServer(mqtt_server, 1883);
//	client.setCallback(MQTTonNewMessage);
}

uint8_t colors[12];

void loop() {
	static uint32_t lastMillis = 0, Count = 0, tCount;
	static int16_t Val = 0, Inc = 10;

	Dimmer.run();
	OTA.run();
	Remote.run();

	//MQTTLoop();

	//  static uint16_t last_out = 0;
	//  if (Dimmer.getOutVal(7) != last_out) {
	//    last_out = Dimmer.getOutVal(7);
	//    uint16_t Freq = 100 + last_out * 40;
	//    ledcWrite(0, Dimmer.getOutVal(1) * 255/100);
	//    ledcWriteTone(0, Freq);
	//    Serial.printf("Freq: %u hz\n", Freq);
	//  }

	  //Color_NeoPixel(Dimmer.getOutVal(2) * 255/100, Dimmer.getOutVal(3) * 255/100, Dimmer.getOutVal(4) * 255/100);

	static uint32_t lastSin;
	if (millis() - lastSin > 19) {
		lastSin = millis();

		static float pos = PI/2.0, inc = PI / 2000.0;
		pos += inc;
		//if (pos > PI) pos -= PI;

		//Dimmer.setOutDuty(2, (float)55.0 + (cosf(pos) * 45.0));
	}

	if (millis() - lastMillis > 999) {
		lastMillis = millis();
		//Serial.printf("AC freq: %.2f hz\n", Dimmer.getACFreq());

		for (int i = 0; i < 12; i++) {
			colors[i] = rand();
		}

		Color_NeoPixel(colors);

		Board.LED1_Toggle();
		Board.LED2_Toggle();


		//Dimmer.setOutDuty(0, Val);
		Val += Inc;
		if (Val >= 100) {
			Val = 100;
			Inc = -2;
		}
		else if (Val <= 0) {
			Val = 0;
			Inc = 2;
		}
	}
}

void Load_Preset(String Preset)
{
	//Dimmer.setOutDuty(0, 0); //Ventilador
	Serial.printf("Mensaje: '");
	Serial.print(Preset);
	Serial.printf("'\n");

	if (Preset == "preset=0") {
		Serial.print("Cargando preset 0\n");
		for (int i = 1; i< 10; i++)
			Dimmer.setOutDuty(i, 0);
	}
	else if (Preset == "preset=1") {
		Serial.print("Cargando preset 1\n");
		for (int i = 1; i < 10; i++)
			Dimmer.setOutDuty(i, 100);
	}
	else if (Preset == "preset=2") {
		Serial.print("Cargando preset 2\n");
		//Dimmer.setOutDuty(6, 100); //Luces techo
		//Dimmer.setOutDuty(7, 100);
		//Dimmer.setOutDuty(8, 100);
		//Dimmer.setOutDuty(9, 100);
		Dimmer.setOutDuty(1, 80);
		Dimmer.setOutDuty(2, 80);
		Dimmer.setOutDuty(3, 80);
		Dimmer.setOutDuty(4, 80);
	}
	else if (Preset == "preset=3") {
		Serial.print("Cargando preset 3\n");
		//Dimmer.setOutDuty(6, 0); //Luces techo
		//Dimmer.setOutDuty(7, 0);
		//Dimmer.setOutDuty(8, 0);
		//Dimmer.setOutDuty(9, 0);
		Dimmer.setOutDuty(1, 60);
		Dimmer.setOutDuty(2, 60);
		Dimmer.setOutDuty(3, 60);
		Dimmer.setOutDuty(4, 60);
	}
	else if (Preset == "preset=4") {
		Serial.print("Cargando preset 4\n");
		//Dimmer.setOutDuty(6, 100); //Luces techo
		//Dimmer.setOutDuty(7, 100);
		//Dimmer.setOutDuty(8, 0);
		//Dimmer.setOutDuty(9, 0);
		Dimmer.setOutDuty(1, 45);
		Dimmer.setOutDuty(2, 45);
		Dimmer.setOutDuty(3, 45);
		Dimmer.setOutDuty(4, 45);
	}
	else if (Preset == "preset=5") {
		Serial.print("Cargando preset 6\n");
		//Dimmer.setOutDuty(6, 0); //Luces techo
		//Dimmer.setOutDuty(7, 0);
		//Dimmer.setOutDuty(8, 100);
		//Dimmer.setOutDuty(9, 100);
		Dimmer.setOutDuty(1, 35);
		Dimmer.setOutDuty(2, 35);
		Dimmer.setOutDuty(3, 35);
		Dimmer.setOutDuty(4, 35);
	}
	else if (Preset == "preset=6") {
		Serial.print("Cargando preset 5\n");
		Dimmer.setOutDuty(1, 25);
		Dimmer.setOutDuty(2, 25);
		Dimmer.setOutDuty(3, 25);
		Dimmer.setOutDuty(4, 25);

		Dimmer.setOutDuty(6, 0); //Luces techo
		Dimmer.setOutDuty(7, 0);
		Dimmer.setOutDuty(8, 0);
		Dimmer.setOutDuty(9, 0);
	}
	else if (Preset == "preset=7") {
		Serial.print("Cargando preset 6\n");
		Dimmer.setOutDuty(1, 15);
		Dimmer.setOutDuty(2, 15);
		Dimmer.setOutDuty(3, 15);
		Dimmer.setOutDuty(4, 15);

		Dimmer.setOutDuty(6, 0); //Luces techo
		Dimmer.setOutDuty(7, 0);
		Dimmer.setOutDuty(8, 0);
		Dimmer.setOutDuty(9, 0);
	}
	else if (Preset == "preset=8") {
		Serial.print("Cargando preset 7\n");
		Dimmer.setOutDuty(1, 10); //Luces pared verde
		Dimmer.setOutDuty(2, 10);
		Dimmer.setOutDuty(3, 10);
		Dimmer.setOutDuty(4, 10); //Luces mesa
	}
	else
		Serial.print("No se recocnoce comando!\n");
}

void MQTTLoop()
{
	static long lastMsg;
	static float Last_Vals[10] = { -1, -1, -1, -1 , -1 , -1 , -1 , -1 , -1 , -1 };

	if (!client.connected()) {
		if (millis() - lastMsg > 2000) {
			lastMsg = millis();
			Serial.printf("Conectando a servidor MQTT....!\n");

			//Publicar
			char topic[32];
			strcpy(topic, "devices/");
			getMacAddress(&topic[8]);

			if (client.connect(HostName, topic, 1, true, "offline")) {
				// Subscribe
				client.subscribe("#");
				Serial.printf("MQTT conectado!\n");

				client.publish(topic, "online");
			}
		}
		else
			return;
	}
	client.loop();

	digitalWrite(IO13, HIGH);
	if ((millis() - lastMsg) > 500) {
		lastMsg = millis();

		char Str1[32], Str2[16];
		int16_t Val, i;
		for (i = 0; i < 10; i++) {
			float v = Dimmer.getOutDuty(i);
			if (Last_Vals[i] != v) {
				Last_Vals[i] = v;

				// Convert the value to a char array
				ltoa(i, Str2, 10);
				strcpy(Str1, "home/luces/luz");
				strcat(Str1, Str2);

				if (v > 0)
					strcpy(Str2, "on");
				else
					strcpy(Str2, "off");

				client.publish(Str1, Str2);

				dtostrf(v, 1, 2, Str2);
				//ftoa(v, Str2, 10);
				strcat(Str1, "/value");
				client.publish(Str1, Str2);

				//Serial.printf("Publicando sub:%s val:%s!\n", Str1, Str2);
			}
		}
	}
	digitalWrite(IO13, LOW);
}

void MQTTonNewMessage(char* topic, byte* message, unsigned int length)
{
	Serial.print("Message arrived on topic: ");
	Serial.print(topic);
	Serial.print(". Message: ");
	String messageTemp;
	char msg[32];

	for (int i = 0; i < length; i++) {
		Serial.print((char)message[i]);
		messageTemp += (char)message[i];
	}
	Serial.println();

	uint16_t pos, posOut, Out;
	pos = strlen(topic);
	posOut = strlen("cmd/home/luces/luz");

	if (pos >= posOut) {
		Out = topic[posOut];
		topic[posOut] = 'x';
	}
	else
		Out = 0;

	// Feel free to add more if statements to control more GPIOs with MQTT

	// If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
	// Changes the output state according to the message
	if (String(topic) == "cmd/home/luces/luzx") {
		Out -= '0';
		if (messageTemp == "on") {
			Dimmer.setOutDuty(Out, 100);
		}
		else if (messageTemp == "off") {
			Dimmer.setOutDuty(Out, 0);
		}
	} 
	else if (String(topic) == "cmd/home/luces/luzx/value") {
		Out -= '0';
		uint16_t Val = messageTemp.toInt();
		if (Val >=0 && Val <= 100) {
			Dimmer.setOutDuty(Out, Val);
		}
	}
	else if (String(topic) == "cmd/preset") {
		Serial.println("New preset recibido");
		Load_Preset(messageTemp);
	}
}

void onRemote(uint32_t Code)
{
	int8_t Out = -1;
	switch (Code) {
	case BUTTON_0:
	case 0xe0e08877:
		Out = 0;
		break;
	case BUTTON_1:
	case 0xe0e020df:
		Out = 1;
		break;
	case BUTTON_2:
	case 0xe0e0a05f:
		Out = 2;
		break;
	case BUTTON_3:
	case 0xe0e0609f:
		Out = 3;
		break;
	case BUTTON_4:
	case 0xe0e010ef:
		Out = 4;
		break;
	case BUTTON_5:
	case 0xe0e0906f:
		Out = 5;
		break;
	case BUTTON_6:
	case 0xe0e050af:
		Out = 6;
		break;
	case BUTTON_7:
	case 0xe0e030cf:
		Out = 7;
		break;
	case BUTTON_8:
	case 0xe0e0b04f:
		Out = 8;
		break;
	case BUTTON_9:
	case 0xe0e0708f:
		Out = 9;
		break;
	}
	Serial.printf("Boton %u\n", Out);

	if (Out >= 0) {
		uint16_t Val = Dimmer.getOutVal(Out);
		if (Val > 0)
			Val = 0;
		else
			Val = 100;
		Dimmer.setOutDuty(Out, Val);
	}
}

void getMacAddress(char *dest)
{
	uint8_t baseMac[6];
	// Get MAC address for WiFi station
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	char baseMacChr[18] = { 0 };
	sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

	strcpy(dest, baseMacChr);
}

void Load_Conf()
{
	DynamicJsonDocument json_file(1024);

	Serial.printf("Cargando configuración desde archivo...\n");

	//Cargar configuración
	File f = SPIFFS.open("/setup.ini");

	//String Data = f.readString();

	//Serial.print(Data);

	//Tokenizer Tk(Data);

	//for (int i = 0; i< 15; i++) {
	//	Token_Val Val = Tk.GetNext();
	//	Serial.print("-> Token: ");
	//	Serial.println(Val.GetString());
	//}

	DeserializationError error = deserializeJson(json_file, f);

	if (error) {
		Serial.printf("deserializeJson() failed: %s\n", error.c_str());
		return;
	}

	strcpy(ssid, json_file["wifi"]["ssid"].as<String>().c_str());
	strcpy(pass, json_file["wifi"]["password"].as<String>().c_str());
	strcpy(HostName, json_file["wifi"]["hostname"].as<String>().c_str());

	Use_Static_IP = json_file["wifi"]["static_ip"].as<bool>();

	local_IP.fromString(json_file["wifi"]["ip"].as<String>());
	subnet.fromString(json_file["wifi"]["mask"].as<String>());
	gateway.fromString(json_file["wifi"]["gateway"].as<String>());
	primaryDNS.fromString(json_file["wifi"]["dns"].as<String>());
	secondaryDNS.fromString(json_file["wifi"]["secdns"].as<String>());

	Serial.println(local_IP);

	Serial.printf("Listo!\n");

}


#define NR_OF_LEDS   4
#define NR_OF_ALL_BITS 24*NR_OF_LEDS

rmt_data_t led_data[NR_OF_ALL_BITS];

rmt_obj_t* rmt_send = NULL;

void Init_NeoPixel()
{
	if ((rmt_send = rmtInit(15, true, RMT_MEM_64)) == NULL)
	{
		Serial.println("init sender failed\n");
	}

	float realTick = rmtSetTick(rmt_send, 100);
	Serial.printf("real tick set to: %fns\n", realTick);
}

void Color_NeoPixel(uint8_t *color)
{
	int led_index = 0;

	int led, col, bit;
	int i = 0;
	for (led = 0; led < NR_OF_LEDS * 3; led++) {
		col = *color++;
		for (bit = 0; bit < 8; bit++) {
			if (col & (1 << (8 - bit))) {
				led_data[i].level0 = 1;
				led_data[i].duration0 = 8;
				led_data[i].level1 = 0;
				led_data[i].duration1 = 4;
			}
			else {
				led_data[i].level0 = 1;
				led_data[i].duration0 = 4;
				led_data[i].level1 = 0;
				led_data[i].duration1 = 8;
			}
			i++;
		}
	}

	// Send the data
	rmtWrite(rmt_send, led_data, NR_OF_ALL_BITS);
}
