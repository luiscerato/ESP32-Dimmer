#include "ac_dimmer.h"
#include "HardwareSerial.h"
#include "board.h"


#define AC_NumberOfOutputs	10				//Cantidad de salidas del dimmmer
#define AC_TimerScaler	8					//Escalado del timer. Incrementa cada 0.1uS. El reloj del timer es 80Mhz
#define AC_ZC_CountToPeriod 25				//Cantidad de cruces por cero para calcular periódo
#define AC_ScaleUsToTimer(x)	(x*(80/AC_TimerScaler)) //Convierte un valor en US a cuentas de timer
#define AC_ScaleTimerToUs(x)	(x/(80/AC_TimerScaler)) //Convierte un valor en US a cuentas de timer

//Definiciones de funcionamiento
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t * ptimer = NULL;

volatile uint8_t Timer;						//Timer a usar

volatile uint8_t Pin_ZeroCross;
volatile uint16_t Outs[AC_NumberOfOutputs];			//Buffer con los valores de salidas seteados
volatile uint16_t Outs_Pins[AC_NumberOfOutputs];		//Definición de pines
volatile uint32_t AC_Outs_Force0;			//Salidas a forzar a 0
volatile uint32_t AC_Outs_Force1;			//Salidas a forzar a 1
uint16_t Out_Min, Out_Max;					//Límites del dimmer, fuera de ellos es 0% o 100%

//Variables de timing AC
volatile uint32_t AC_Period = 0;			//Perído del cruce por cero promediado de 50 muestras (en ciclos de timer)
volatile uint16_t AC_Resolution = 100;		//Resolución del dimmer
volatile uint32_t AC_ZC_Offset = 0;			//Offset de tiempo del detector de cruce por cero respecto el valor real de la onda alterna
volatile uint16_t AC_ZC_Counter = 0;		//Contador de cruces por cero (para detectar falta de frecuencia)

void IRAM_ATTR onZeroCross();
void IRAM_ATTR onTimer();

/*
 * El funcionamiento del dimmer es de la siguiente manera:
 *  Se usa un solor timer para medir el período de la onda y generar las interrupciones necesarias para actualizar las salidas.
 *  Cuando se produce un cruce por cero se captura el valor del timer y se lo reinicia para volver a medir la señal. Este
 *  valor se usa como referencia para generar los pulsos de salida (según la resolución seteada, por lo que no es necesario
 *  ajustes de frecuencia, sino que el sistema se ajusta a la frecuencia de entrada automáticamente.
 *
 *  Para generar las interrupciones de actualización de salida se usa el registro de alarmas del timer, que se va actualizando
 *  en cada pulso de salida. En el cruce por cero se setea la primer alarma, que es el offset para compensar la respuesta de tiempo
 *  del cruce por cero. En los siguientes ciclos el valor de la alarma se incrementa proporcionalmente hasta llegar al próximo
 *  cruce por cero, momento en el que vuelve a comenzar todo el ciclo.
 */


int8_t AC_Dimmer::begin(uint8_t nTimer, uint8_t ZCD_Pin, const uint8_t *Out_Pins)
{
	Timer = nTimer;

	AC_Period = 0;
	AC_Resolution = 100;
	AC_Period = 0;
	AC_ZC_Offset = 150;

	Pin_ZeroCross = ZCD_Pin;
	pinMode(Pin_ZeroCross, INPUT);

	for (int i = 0; i < AC_NumberOfOutputs; i++) {
		Outs_Pins[i] = *Out_Pins++;
		pinMode(Outs_Pins[i], OUTPUT);
		Outs[i] = 0;
		//Serial.printf("Out%u: pin: %u, val: %u\n", i, Outs_Pins[i], Outs[i]);
	}

	pinMode(IO12, OUTPUT);

	Out_Min = 5;
	Out_Max = 95;

	//Interrupción para el detector de cruce por cero
	attachInterrupt(Pin_ZeroCross, onZeroCross, FALLING);

	//Iniciar timer y asignar ISR
	ptimer = timerBegin(Timer, AC_TimerScaler, true);
	timerAttachInterrupt(ptimer, &onTimer, false);
	return 0;
}


int8_t AC_Dimmer::stop()
{
	detachInterrupt(Pin_ZeroCross);
	timerStop(ptimer);
	return 0;
}

int8_t AC_Dimmer::run()
{
	static uint32_t last_time = 0;

	if (millis() - last_time < 500)
		return 0;
	last_time = millis();

	//	Serial.printf("Period: %u, interval: %u\n", AC_Period, _AC_Interval);

		//Comprobar que ocurran eventos de cruce por cero
	//Serial.printf("AC zc counter: %d\n", AC_ZC_Counter);
	portENTER_CRITICAL(&mux);
	if (AC_ZC_Counter < 10)
		AC_Period = 0.0;
	AC_ZC_Counter = 0;
	portEXIT_CRITICAL(&mux);

	return 0;
}

/*
*	setZeroCrossDelay:
*	Setea el retardo del detector de cruce por cero
*/
int8_t AC_Dimmer::setZeroCrossDelay(uint16_t Delay)
{
	if (Delay > 200)
		Delay = 200;
	portENTER_CRITICAL(&mux);
	AC_ZC_Offset = Delay * (80 / AC_TimerScaler);
	portEXIT_CRITICAL(&mux);
}

uint16_t AC_Dimmer::getZeroCrossDelay()
{
	portENTER_CRITICAL(&mux);
	uint16_t Val = AC_ZC_Offset / (80 / AC_TimerScaler);
	portEXIT_CRITICAL(&mux);
	return Val;
}

/*
	setResolution:
	Define la resolución del dimmer, es decir, la cantidad de puntos en los que se divide un período
	de la onda AC. Valor recomendado 100.
*/
int8_t AC_Dimmer::setResolution(uint16_t Resolution)
{
	if (Resolution > 256)
		return -1;
	portENTER_CRITICAL(&mux);
	AC_Resolution = Resolution;
	portEXIT_CRITICAL(&mux);
	return 0;
}

uint16_t AC_Dimmer::getResolution()
{
	portENTER_CRITICAL(&mux);
	uint16_t Val = AC_Resolution;
	portEXIT_CRITICAL(&mux);
	return Val;
}

/*
*	setLimits:
*	Define los límites del dimmer. Debajo del mínimo la salida se fuerza a 0 (siempre a 0),
*	por encima del máximo, la salida se fuerza al máximo (siempre en 1)
*/
int8_t AC_Dimmer::setLimits(uint16_t Min, uint16_t Max)
{
	portENTER_CRITICAL(&mux);
	uint16_t MaxVal = AC_Resolution;
	portEXIT_CRITICAL(&mux);

	if (Min > Max)
		return -1;
	else if (Max > MaxVal)
		return -1;

	Out_Max = Max;
	Out_Min = Min;
	return 0;
}

uint16_t AC_Dimmer::getMinLimit()
{
	return Out_Min;
}

uint16_t AC_Dimmer::getMaxLimit()
{
	return Out_Max;
}


float AC_Dimmer::getACFreq()
{
	portENTER_CRITICAL(&mux);
	uint32_t Val = AC_Period; //En ciclos de timer
	portEXIT_CRITICAL(&mux);
	return 1000000.0 / 2.0 / (float)AC_ScaleTimerToUs(Val);
}

/*
	Setea la salida indicada al valor deseado, en rango 0:AC_Resol
*/
void AC_Dimmer::setOutVal(uint16_t Out, uint16_t Val)
{
	portENTER_CRITICAL(&mux);
	uint16_t MaxVal = AC_Resolution;
	portEXIT_CRITICAL(&mux);

	if (Out > AC_NumberOfOutputs - 1)
		return;
	if (Val > AC_Resolution)
		Val = AC_Resolution;

	uint32_t bit_mask = 0x1 << Out;

	AC_Outs_Force0 &= ~bit_mask; //Limpiar bits de forzados
	AC_Outs_Force1 &= ~bit_mask;

	//Chequear límites de forzados
	if (Val < (Out_Min * AC_Resolution / 100))
		AC_Outs_Force0 |= bit_mask;
	else if (Val > (Out_Max * AC_Resolution / 100))
		AC_Outs_Force1 |= bit_mask;

	portENTER_CRITICAL(&mux);
	//Asignar valor
	Outs[Out] = Val;
	portEXIT_CRITICAL(&mux);
}

/*
	Setea la salida indicada al valor deseado, en rango 0:100%
*/
void AC_Dimmer::setOutDuty(uint16_t Out, float Val)
{
	if (Out > AC_NumberOfOutputs - 1)
		return;

	if (Val > 100)
		Val = 100;

	uint32_t bit_mask = 0x1 << Out;

	AC_Outs_Force0 &= ~bit_mask; //Limpiar bits de forzados
	AC_Outs_Force1 &= ~bit_mask;

	//Chequear límites de forzados
	if (Val < Out_Min)
		AC_Outs_Force0 |= bit_mask;
	else if (Val > Out_Max)
		AC_Outs_Force1 |= bit_mask;

	Serial.printf("Set out %u to %.2f\n", Out, Val);

	//Asignar valor
	uint16_t iVal = Val * (float)AC_Resolution / 100.0;
	portENTER_CRITICAL(&mux);
	//Asignar valor
	Outs[Out] = iVal;
	portEXIT_CRITICAL(&mux);
}

uint16_t AC_Dimmer::getOutVal(uint16_t Out)
{
	if (Out > AC_NumberOfOutputs - 1)
		return 0;

	portENTER_CRITICAL(&mux);
	uint16_t Val = Outs[Out];
	portEXIT_CRITICAL(&mux);

	return Val;
}

float AC_Dimmer::getOutDuty(uint16_t Out)
{
	if (Out > AC_NumberOfOutputs - 1)
		return 0;

	portENTER_CRITICAL(&mux);
	uint16_t Val = Outs[Out];
	portEXIT_CRITICAL(&mux);

	float res = (float)Val / (float)AC_Resolution * 100.0;
	return res;
}


//Variables de interrupciones
volatile uint16_t _AC_Index = 0;				//Posición en la onda AC
volatile uint16_t _AC_Outs[AC_NumberOfOutputs];	//Buffer con los valores de salidas
volatile uint32_t _AC_Outs_Force0;				//Salidas a forzar a 0
volatile uint32_t _AC_Outs_Force1;				//Salidas a forzar a 1
volatile uint32_t _AC_Alarm;					//Valor actual de la alarma del timer
volatile uint32_t _AC_Interval;					//Intervalo de generación de interrupciones del timer

//volatile uint32_t _AC_IntervalOk = 0;			//Indica que ya se calculó al menos un ciclo completo de cruces por cero

/*
 * Cuando se genera la interrupción por cruce por cero se lee el valor del timer y luego se lo pone a cero.
 * El valor leído es el período de la onda AC. Este valor se divide por la resolución deseada y se lo usa
 * para setear el tiempo con el que se van a generar las interrupciones del timer.
 * Per_Max y Per_Min son los valores mínimos que debe tener la medición, sino se aborta
 *
 * Se limita el rango de trabajo del dimmer a 25-100HZ
 */
void IRAM_ATTR onZeroCross()
{
	static uint32_t PeriodAccum = 0, Aux, Period = 0;
	static uint16_t PeriodCount = 0;

	portENTER_CRITICAL_ISR(&mux);

	//Leer período del ciclo
	TIMERG0.hw_timer[Timer].update = 1;
	Aux = TIMERG0.hw_timer[Timer].cnt_low;

	//Poner timer en 0
	TIMERG0.hw_timer[Timer].load_high = 0;
	TIMERG0.hw_timer[Timer].load_low = 0;
	TIMERG0.hw_timer[Timer].reload = 1;

	//Cargar próximo valor de interrupción del timer
	_AC_Alarm = AC_ZC_Offset;
	TIMERG0.hw_timer[Timer].alarm_high = 0;
	TIMERG0.hw_timer[Timer].alarm_low = _AC_Alarm;
	TIMERG0.hw_timer[Timer].config.autoreload = 0;
	TIMERG0.hw_timer[Timer].config.alarm_en = 1;

	//Copiar valores de salidas en buffer de salida
	for (uint8_t i = 0; i < AC_NumberOfOutputs; i++)
		_AC_Outs[i] = Outs[i];

	//Acumulador de período
	PeriodAccum += Aux;
	PeriodCount++;

	if (PeriodCount == AC_ZC_CountToPeriod) {
		AC_Period = PeriodAccum / AC_ZC_CountToPeriod; //Promediar medición de período para luego calcular frecuencia
		_AC_Interval = AC_Period / AC_Resolution; //Calcular intervalo de incremento del timer
		PeriodAccum = 0;
		PeriodCount = 0;
	}
	_AC_Outs_Force0 = AC_Outs_Force0;
	_AC_Outs_Force1 = AC_Outs_Force1;

	_AC_Index = 0;
	AC_ZC_Counter++;
	portEXIT_CRITICAL_ISR(&mux);
}


void IRAM_ATTR onTimer()
{
	int32_t i, out, mask;

	portENTER_CRITICAL_ISR(&mux);

	//Solo si ya se tiene un valor de período calculado.
	if (_AC_Interval > 100) {
		//Actualizar registro de alarma
		_AC_Alarm += _AC_Interval; //This does not work, even with the same value on the variable.
		TIMERG0.hw_timer[Timer].alarm_high = 0;
		TIMERG0.hw_timer[Timer].alarm_low = _AC_Alarm;
		TIMERG0.hw_timer[Timer].config.autoreload = 0;
		TIMERG0.hw_timer[Timer].config.alarm_en = 1;


		//Calcular valores de salida
		out = 0;
		mask = 1;
		for (i = 0; i < AC_NumberOfOutputs; i++) {
			if (_AC_Outs_Force0 & mask)
				digitalWrite(Outs_Pins[i], 0);
			else if (_AC_Outs_Force1 & mask)
				digitalWrite(Outs_Pins[i], 1);
			else {
				if (_AC_Outs[i] == (AC_Resolution - _AC_Index))
					digitalWrite(Outs_Pins[i], 1);
				else
					digitalWrite(Outs_Pins[i], 0);
			}
			mask <<= 1;
		}
		_AC_Index++;
		if (_AC_Index > AC_Resolution * 3) { //Si se deja de reiniciar el offset es porque no se reciben más pulsos de ZCD, cortar la actualización
			_AC_Index = AC_Resolution;
			_AC_Interval = 0;
		}
	}

	portEXIT_CRITICAL_ISR(&mux);
}


//void IRAM_ATTR Write74HC595(uint32_t Val, uint8_t bytes)
//{
//	if (bytes > 3)
//		return;
//
//	uint8_t bits = bytes * 8;
//	uint32_t mask = 0x1 << (bits-1);
//
//	digitalWrite(Pin_EN, 0);
//
//	while (bits--) {
//		digitalWrite(Pin_CLK, 0);
//		if (Val & mask)
//			digitalWrite(Pin_Data, 1);
//		else
//			digitalWrite(Pin_Data, 0);
//		digitalWrite(Pin_CLK, 1);
//		mask >>= 1;
//	}
//	digitalWrite(Pin_CLK, 0);
//	digitalWrite(Pin_EN, 1);
//	digitalWrite(Pin_Data, 0);
//	digitalWrite(Pin_EN, 0);
//}

//
//String AC_Channel::getName()
//{
//	return _Name;
//}
//
//void AC_Channel::setName(String Name)
//{
//	_Name = Name;
//}
//
//bool AC_Channel::getIsDimmer()
//{
//	return _CanDim;
//}
//
//void AC_Channel::setIsDimmer(bool Dim)
//{
//	if (Dim) {
//		if (_Value > 0)
//			_Value = 100;
//		else
//			_Value = 100;
//	}
//}
//
//uint16_t AC_Channel::getActualValue()
//{
//	return uint16_t();
//}
//
//void AC_Channel::setActualValue(uint16_t Val)
//{
//}
//
//bool AC_Channel::getOutState()
//{
//	return false;
//}
//
//void AC_Channel::setOutState(bool State)
//{
//}
//
//void AC_Channel::setOutOff()
//{
//}
//
//void AC_Channel::setOutOn()
//{
//}
//
//void AC_Channel::setOutToggle()
//{
//}
//
//void AC_Channel::setLimits(uint16_t Min, uint16_t Max)
//{
//}
//
//uint16_t AC_Channel::getLimitMin()
//{
//	return uint16_t();
//}
//
//uint16_t AC_Channel::getLimitMax()
//{
//	return uint16_t();
//}
//
//void AC_Channel::Fade(uint16_t Val, uint16_t Time)
//{
//}
//
//void AC_Channel::run()
//{
//}
