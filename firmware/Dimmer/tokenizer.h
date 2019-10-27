// tokenizer.h

#ifndef _TOKENIZER_h
#define _TOKENIZER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

typedef enum {
	Token_Undefined = 0,	//No est� definido
	Token_Int,		//Token tipo entero (+-ddddddd)
	Token_Float,	//Token n�mero flotante (+-dddd.ffff)
	Token_String,	//Token tipo string ("ssssss")
	Token_Equal,	//Token simbolo '='
	Token_Math,		//Token s�mbolo (+-*/)
	Token_Simbol,	//Token s�mbolo (:;)
	Token_Comma,	//Token coma (,)
	Token_Keyword,	//Token keyword (palabra fuera de un string, debe empezar por una letra, puede contener n�meros y guion bajo)
	Token_EOF,		//Fin del string o nueva l�nea
}Token_Type_t;

class Token_Val
{
public:
	typedef struct {
		Token_Type_t Type;	//Tipo de token a convertir (Ver Token_Type_t)
		char *Pos;			//Posici�n del token
		uint16_t Len;		//Cantidad de bytes que tiene el token
	} Val_t;			//Informaci�n del token procesado por GetNext()

	Val_t Val;

	Token_Val() {
		Val.Pos = NULL;
		Val.Len = 0;
		Val.Type = Token_Undefined;
	};

	float GetFloat();

	int GetInt();

	unsigned int GetUInt();

	String GetString();
};

class Tokenizer
{
protected:
	char *Str;	//Posici�n de trabajo
	char *StrInit;	//Posici�n inicial del string

public:
	Tokenizer() {
		Str = NULL;
		StrInit = NULL;
	};	//Inicializa tokenizer como NULL

	Tokenizer(char * Str) {
		this->Str = Str;
		StrInit = Str;
	};	//Inicializa tokenizer con un array de chars

	Tokenizer(String Str) {
		this->Str = (char*)Str.c_str();
		StrInit = (char*)Str.c_str();
	};	//Inicializa tokenizer con un String

	void restart() {
		Str = StrInit;
	};	//Vuelve a la posici�n de inicio del string

	bool HashNext();	//Devuelve true si hay m�s tokens en el string

	Token_Val GetNext();	//Devuelve el pr�ximo token v�lido
};


#endif

