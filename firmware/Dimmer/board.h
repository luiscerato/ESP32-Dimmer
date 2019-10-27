#ifndef BOARD_AC_DIMMER_10CH
#define BOARD_AC_DIMMER_10CH
#include "arduino.h"

class board {
#define OUT1 26
#define OUT2 25
#define OUT3 22
#define OUT4 21
#define OUT5 19
#define OUT6 18
#define OUT7 5
#define OUT8 17
#define OUT9 16
#define OUT10 4

#define INPUT1 36
#define INPUT2 39
#define INPUT3 34
#define INPUT4 35

#define INPUT_ZCD 33
#define INPUT_PIR 32
#define INPUT_IRC 27

#define LED1 2
#define LED2 0

#define IO5 23
#define IO12 12
#define IO13 13
#define IO14 14
#define IO15 15

public:
	const char *BOARD_NAME;
	const uint8_t *Pins_Outputs;
	const uint8_t *Pins_Inputs;

	void begin();

	void LED1_Toggle();

	void LED1_Off();

	void LED1_On();

	bool LED1_GetState();

	void LED2_Toggle();

	void LED2_Off();

	void LED2_On();

	bool LED2_GetState();

};

extern board Board;
extern const uint8_t Pins_Outs[];
extern const uint8_t Pins_Leds[];
extern const uint8_t Pins_Input[];

#endif
