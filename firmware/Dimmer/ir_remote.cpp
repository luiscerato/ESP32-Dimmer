#include "ir_remote.h"
#include "board.h"

const ListCode_t Remote1 = {
	0xe0e040bf, // Button_Power
	0xe0e0f807, // Button_Info
	0xe0e0807f, // Button_Source
	0xe0e08877, // Button_0
	0xe0e020df, // Button_1
	0xe0e0a05f, // Button_2
	0xe0e0609f, // Button_3
	0xe0e010ef, // Button_4
	0xe0e0906f, // Button_5
	0xe0e050af, // Button_6
	0xe0e030cf, // Button_7
	0xe0e0b04f, // Button_8
	0xe0e0708f, // Button_9
	0xe0e0c43b, // Button_100
	0xe0e0c837, // Button_200
	0xe0e048b7, // Button_Ch_Plus
	0xe0e008f7, // Button_Ch_Minus
	0xe0e0e01f, // Button_Vol_Plus
	0xe0e0d02f, // Button_Vol_Minus
	0xe0e0f00f, // Button_Mute
	0xe0e006f9, // Button_Up
	0xe0e08679, // Button_Down
	0xe0e0a659, // Button_Left
	0xe0e046b9, // Button_Right
	0xe0e016e9, // Button_Enter
	0xe0e058a7, // Button_Menu
	0xe0e0b44b, // Button_Exit
};

const ListCode_t Remote2 = {
	0xFF629D, // Button_Power
	0x000000, // Button_Info
	0x000000, // Button_Source
	0xFF6897, // Button_0
	0xFF30CF, // Button_1
	0xFF18E7, // Button_2
	0xFF7A85, // Button_3
	0xFF10EF, // Button_4
	0xFF38C7, // Button_5
	0xFF5AA5, // Button_6
	0xFF42BD, // Button_7
	0xFF4AB5, // Button_8
	0xFF52AD, // Button_9
	0xFF9867, // Button_100
	0xFFB04F, // Button_200
	0xFFE21D, // Button_Ch_Plus
	0xFFA25D, // Button_Ch_Minus
	0xFFA857, // Button_Vol_Plus
	0xFFE01F, // Button_Vol_Minus
	0x000000, // Button_Mute
	0x000000, // Button_Up
	0x000000, // Button_Down
	0xFF22DD, // Button_Left
	0xFF02FD, // Button_Right
	0xFFC23D, // Button_Enter
	0x000000, // Button_Menu
	0xFF906F, // Button_Exit
};

IRrecv irrecv(INPUT_IRC); //ToDo: ver como cambiar esto


ir_remote::ir_remote()
{
	WaitOnNextKey = 750;
	handler = NULL;
}

uint8_t ir_remote::begin(uint8_t Pin)
{
	//irrecv = IRrecv(Pin);
	irrecv.enableIRIn();  // Start the receiver

	Serial.printf("Iniciando receptor de control remoto infrarojo en pin %u\n", Pin);
}

void ir_remote::run()
{
	static uint32_t LastVal = 0xFFFFFFFF, LastTime, Code;

	if (irrecv.decode(&results)) {
		serialPrintUint64(results.value, HEX);
		Serial.println("");

		if ((results.value == 0xFFFFFFFFFFFFFFFFLL) || (results.value == LastVal)) { //Código de repetición
			if ((millis() - LastTime) > WaitOnNextKey) {
				Code = LastVal;
			}
			else
				Code = 0;
		}
		else {
			LastTime = millis();	//Última momento que se recibió una tecla
			Code = results.value;
			LastVal = Code;
		}
		if (Code) {
			Serial.printf("Recibido: 0x%x\n", Code);
			if (handler) {
				Serial.printf("onKey()\n");
				handler(Code);
			}
		}
		irrecv.resume();  // Receive the next value
	}

}

void ir_remote::addOnKeyPressed(void(*func)(uint32_t))
{
	handler = func;
}

void ir_remote::removeOnKeyPressed()
{
	handler = NULL;
}


