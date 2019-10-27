#include "board.h"
#include "esp32-hal.h"

const uint8_t Pins_Outs[] = {OUT1, OUT2, OUT3, OUT4, OUT5, OUT6, OUT7, OUT8, OUT9, OUT10};
const uint8_t Pins_Leds[] = {LED1, LED2};
const uint8_t Pins_Input[] = {INPUT1, INPUT2, INPUT3, INPUT4};

board Board;

void board::begin()
{
	for (uint8_t i = 0; i < sizeof(Pins_Outs); i++) {
		pinMode(Pins_Outs[i], OUTPUT);
		digitalWrite(Pins_Outs[i], 0);
	}
	pinMode(LED1, OUTPUT);
	LED1_Off();
	pinMode(LED2, OUTPUT);
	LED2_Off();

	for (uint8_t i = 0; i < sizeof(Pins_Input); i++) {
		pinMode(Pins_Input[i], INPUT);
	}

	pinMode(INPUT_ZCD, INPUT_PULLUP);
	pinMode(INPUT_PIR, INPUT_PULLUP);
	pinMode(INPUT_IRC, INPUT_PULLUP);

	pinMode(IO5, INPUT_PULLUP);
	pinMode(IO12, INPUT_PULLUP);
	pinMode(IO13, INPUT_PULLUP);
	pinMode(IO14, INPUT_PULLUP);
	pinMode(IO15, INPUT_PULLUP);

	this->BOARD_NAME = "AC DIMMER 10CH";
	this->Pins_Outputs = Pins_Outs;
	this->Pins_Inputs = Pins_Input;
}

void board::LED1_Toggle() {
	digitalWrite(LED1, !digitalRead(LED1));
}

void board::LED1_Off() {
	digitalWrite(LED1, 1); //Está invertido
}

void board::LED1_On() {
	digitalWrite(LED1, 0); //Está invertido
}

bool board::LED1_GetState() {
	digitalRead(LED1);
}

void board::LED2_Toggle() {
	digitalWrite(LED2, !digitalRead(LED2));
}

void board::LED2_Off() {
	digitalWrite(LED2, 0);
}

void board::LED2_On() {
	digitalWrite(LED2, 0);
}

bool board::LED2_GetState() {
	digitalRead(LED2);
}
