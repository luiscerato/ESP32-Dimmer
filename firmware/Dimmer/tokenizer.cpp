// 
// 
// 

#include "tokenizer.h"


bool Tokenizer::HashNext()
{
	if (Str)
		return true;
	return false;
}


Token_Val Tokenizer::GetNext()
{
	char *p, c;
	uint16_t Count = 0;

	p = Str;
	Token_Val Tk;

	if (Str == NULL || *p == NULL) { //Si Str es NULL o apunta a NULL, volver
		Tk.Val.Type = Token_EOF;
		Str = NULL;
		return Tk;
	}

	c = *p;
	while (c == ' ' || c == '\t') //Esquivar espacios
		c = *++p;
	Tk.Val.Pos = p;

	if (c == '=') { //                  Chequear por '='
		Tk.Val.Type = Token_Equal;
		Tk.Val.Len = 1;
		p++;
	}
	else if (c >= '0' && c <= '9') { //          Chequear por número
	lToken_Num:
		Tk.Val.Type = Token_Int;
		Tk.Val.Len = 0;
		if (c == '-') {
			c = *++p;
			Tk.Val.Len++;
		}
		while (c >= '0' && c <= '9') {
			c = *++p;
			Tk.Val.Len++;
		}
		if (c == '.') {
			Tk.Val.Len++;
		lToken_ChkFloat:
			Tk.Val.Type = Token_Float;
			c = *++p;
			while (c >= '0' && c <= '9') {
				c = *++p;
				Tk.Val.Len++;
			}
		}
	}
	else if (c == ',') { //          Chequear por coma
		Tk.Val.Type = Token_Comma;
		Tk.Val.Len = 1;
		p++;
	}
	else if (c == NULL || c == '\n' || c == 13) { //          Chequear por NULL o fin de linea
		Tk.Val.Type = Token_EOF;
		Tk.Val.Len = 0;
	}
	else if (c == '-') { //          Chequear por signo negativo
		c = *(p + 1);
		if (c < '0' || c > '9')
			goto lToken_Math;
		else {
			goto lToken_Num;
		}
	}
	else if (c == '.') { //          Chequear por '.' podría ser un número decimal
		c = *(p + 1);
		if (c >= '0' && c <= '9')
			goto lToken_ChkFloat;
		Tk.Val.Len++;
		p++;
	}
	else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') { //Buscar keyword (no puede empezar por número)
		Tk.Val.Type = Token_Keyword;
		Tk.Val.Len = 0;
		Count = 0;
		while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9')) {
			if (c == '_')
				Count++;
			c = *++p;
			Tk.Val.Len++;
		}
		if (Count == Tk.Val.Len) //Controlar que no sean todos guiones bajos
			Tk.Val.Type = Token_Undefined;
	}
	else if (c == '"') { //          Chequear por (") comillas dobles, puede ser un string
		Tk.Val.Type = Token_String;
		for (Tk.Val.Len = 0;; Tk.Val.Len++) { //Bottom contiene la longitud del string
			c = *++p;
			if (c == '"')
				break;
			else if (c == '\\') {
				if (*(p + 1) == '"') {
					p++;
					Tk.Val.Len++;
				}
			}
			else if (c == '\n' || c == 0)
				Tk.Val.Type = Token_Undefined;
		}
		if (Tk.Val.Type != Token_String) { //Si no es un string, apuntar nuevamente a la comilla doble y salir
			Tk.Val.Len = 1;
			p = Tk.Val.Pos + 1;
		}
		else {//Si es un string, apuntar p a la posición siguiente de la comilla doble
			p++;
			Tk.Val.Pos++;
		}
	}
	else if (c == '+' || c == '/' || c == '*') { //          Chequear por operación matemática
	lToken_Math:
		Tk.Val.Type = Token_Math;
		Tk.Val.Len = 1;
		p++;
	}
	else if (c == ':' || c == ';') { //          Chequear por símbolos
		Tk.Val.Type = Token_Simbol;
		Tk.Val.Len = 1;
		p++;
	}
	else { //No definido, avanzar de caracter
		Tk.Val.Len++;
		p++;
	}

	Str = p; //Apuntar el string de origen al siguiente caracter
	return Tk;
}


float Token_Val::GetFloat()
{
	if (Val.Type != Token_Float)
		return 0;

	float Res = atof(Val.Pos);
	return Res;
}


int Token_Val::GetInt()
{
	if (Val.Type != Token_Int)
		return 0;

	int Res = atoi(Val.Pos);
	return Res;
}


unsigned int Token_Val::GetUInt()
{
	if (Val.Type != Token_Int)
		return 0;

	unsigned int Res = atoi(Val.Pos);
	return Res;
}


String Token_Val::GetString()
{
	String Res = String(Val.Pos);
	return Res.substring(0, Val.Len);
}

