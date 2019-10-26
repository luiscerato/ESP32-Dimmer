#pragma once
#ifndef ir_remote_h
#define ir_remote_h

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define BUTTON_CH_MINUS		0xFFA25D
#define BUTTON_CH			0xFF629D
#define BUTTON_CH_PLUS		0xFFE21D
#define BUTTON_PREV			0xFF22DD
#define BUTTON_NEXT			0xFF02FD
#define BUTTON_PLAY			0xFFC23D
#define BUTTON_VOL_MINUS	0xFFE01F
#define BUTTON_VOL_PLUS		0xFFA857
#define BUTTON_EQ			0xFF906F
#define BUTTON_0			0xFF6897
#define BUTTON_100			0xFF9867
#define BUTTON_200			0xFFB04F
#define BUTTON_1			0xFF30CF
#define BUTTON_2			0xFF18E7
#define BUTTON_3			0xFF7A85
#define BUTTON_4			0xFF10EF
#define BUTTON_5			0xFF38C7
#define BUTTON_6			0xFF5AA5
#define BUTTON_7			0xFF42BD
#define BUTTON_8			0xFF4AB5
#define BUTTON_9			0xFF52AD

typedef struct {
	uint32_t Button_Power;
	uint32_t Button_Info;
	uint32_t Button_Source;
	uint32_t Button_0;
	uint32_t Button_1;
	uint32_t Button_2;
	uint32_t Button_3;
	uint32_t Button_4;
	uint32_t Button_5;
	uint32_t Button_6;
	uint32_t Button_7;
	uint32_t Button_8;
	uint32_t Button_9;
	uint32_t Button_100;
	uint32_t Button_200;
	uint32_t Button_Ch_Plus;
	uint32_t Button_Ch_Minus;
	uint32_t Button_Vol_Plus;
	uint32_t Button_Vol_Minus;
	uint32_t Button_Mute;
	uint32_t Button_Up;
	uint32_t Button_Down;
	uint32_t Button_Left;
	uint32_t Button_Right;
	uint32_t Button_Enter;
	uint32_t Button_Menu;
	uint32_t Button_Exit;
}ListCode_t;

class ir_remote
{

private:
	decode_results results;
	void(*handler)(uint32_t);
	uint32_t WaitOnNextKey;

public:
	ir_remote();

	uint8_t begin(uint8_t Pin);

	void run();

	void addOnKeyPressed(void(*func)(uint32_t));

	void removeOnKeyPressed();

};

extern const ListCode_t Remote1;
extern const ListCode_t Remote2;


#endif // !ir_remote_h