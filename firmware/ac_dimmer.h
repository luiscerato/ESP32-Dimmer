#include "esp32-hal-timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/xtensa_api.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "soc/timer_group_struct.h"
#include "soc/dport_reg.h"
#include "esp_attr.h"
#include "esp_intr.h"

#ifndef AC_STUFF_H_
#define AC_STUFF_H_
#endif AC_STUFF_H_


#ifndef AC_STUFF_H_
#endif AC_STUFF_H_

class AC_Dimmer
{
public:
  int8_t begin(uint8_t nTimer, uint8_t ZCD_Pin, const uint8_t *Out_Pins);

  int8_t stop();

  int8_t run();

  int8_t setZeroCrossDelay(uint16_t Delay);

  uint16_t getZeroCrossDelay();

  int8_t setResolution(uint16_t Delay);

  uint16_t getResolution();

  int8_t setLimits(uint16_t Min, uint16_t Max);

  uint16_t getMinLimit();

  uint16_t getMaxLimit();

  float getACFreq();

  void setOutVal(uint16_t Out, uint16_t Val);

  void setOutDuty(uint16_t Out, float Val);

  uint16_t getOutVal(uint16_t Out);

  float getOutDuty(uint16_t Out);
};
//
//class AC_Channel
//{
//public:
//
//	AC_Channel(uint16_t Channel) {
//		_AcChannel = Channel;
//		_CanDim = false;
//		_Value = 0;
//		_MinValue = 0;
//		_MaxValue = 100;
//	};
//
//	AC_Channel(uint16_t Channel, bool CanDim) {
//		_AcChannel = Channel;
//		_CanDim = CanDim;
//		_Value = 0;
//		_MinValue = 0;
//		_MaxValue = 100;
//	};
//
//	AC_Channel(uint16_t Channel, bool CanDim, String Name) {
//		_AcChannel = Channel;
//		_CanDim = CanDim;
//		_Value = 0;
//		_Name = Name;
//		_MinValue = 0;
//		_MaxValue = 100;
//	};
//
//	AC_Channel(uint16_t Channel, bool CanDim, String Name, bool State) {
//		_AcChannel = Channel;
//		_CanDim = CanDim;
//		_Value = 0;
//		_Name = Name;
//		_OnOff = State;
//		_MinValue = 0;
//		_MaxValue = 100;
//	};
//
//	String getName();
//	void setName(String Name);
//
//	bool getIsDimmer();
//	void setIsDimmer(bool Dim);
//
//	uint16_t getActualValue();
//	void setActualValue(uint16_t Val);
//
//	bool getOutState();
//	void setOutState(bool State);
//	void setOutOff();
//	void setOutOn();
//	void setOutToggle();
//
//	void setLimits(uint16_t Min, uint16_t Max);
//
//	uint16_t getLimitMin();
//	uint16_t getLimitMax();
//
//	void Fade(uint16_t Val, uint16_t Time = 1000);
//
//	void run();
//
//
//private:
//	uint16_t _AcChannel;	// Numero de canal de salida
//	String _Name;			// Nombre de la salida
//	bool _CanDim;			// Si la salida puede dimerizar o no
//	bool _OnOff;			// Estado On-Off de la salida
//	uint16_t _Value;		// Valor actual de la salida 
//	uint16_t _MinValue;		// Valor máximo de la salida 
//	uint16_t _MaxValue;		// Valor mínimo de la salida 
//	uint16_t _GoToValue;	// Valor al que irá la salida
//	uint16_t _StartTime;	// Momento de inicio del ciclo
//	uint16_t _IncTime;		// Incremento de tiempo para llegar a la salida deseada
//};
