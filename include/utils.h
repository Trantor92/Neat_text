#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <windows.h>

using namespace std;

//----------------------------------------------------------------------------
//	funzioni random.
//----------------------------------------------------------------------------


//ritorna un random float in [0,1[
inline double RandFloat() {return (rand())/(RAND_MAX+1.0);}

//ritorna un random float in [0,1]
inline double RandFloat_1() { return (rand()) / ((double)RAND_MAX); }

//ritorna un random int in [x,y]
inline int	  RandInt(int x, int y) 
{
	if (RAND_MAX < (y - x))
		return (int)((y - x)*RandFloat_1() + x);
	else
		return rand() % (y - x + 1) + x;//non fornisce il risultato voluto se RAND_MAX < y-x poichè esclude il valore y
}


//ritorna un random bool
inline bool   RandBool()
{
	if (RandInt(0,1)) return true;

	else return false;
}

//ritorna un random float in ]-1,1[, con una distribuzione lineare a tratti 
inline double RandomClamped()	   {return RandFloat() - RandFloat();}


/////////////////////////////////////////////////////////////////////
//
//	funzioni utili
//
/////////////////////////////////////////////////////////////////////


//converte un intero in una stringa
inline string itos(int arg)
{
    ostringstream buffer;
	
	buffer << arg;	
	
    return buffer.str();		
}

//converte un float a stringa
inline string ftos(float arg)
{
    ostringstream buffer;

    buffer << arg;	
	
    return buffer.str();		
}

//mantiene il primo argomento fra gli altri due
inline void Clamp(double &arg, double min, double max)
{
	if (arg < min)
	{
		arg = min;
	}

	if (arg > max)
	{
		arg = max;
	}
}

//mantiene il primo argomento fra gli altri due
inline void Clamp(int &arg, int min, int max)
{
	if (arg < min)
	{
		arg = min;
	}

	if (arg > max)
	{
		arg = max;
	}
}

//funzione di arrotondamento ad intero
inline int Rounded(double val)
{
  int    integral = (int)val;
  double mantissa = val - integral;

  if (mantissa < 0.5)
  {
    return integral;
  }

  else
  {
    return integral + 1;
  }
}


//conversione fromati stringa
inline wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

//crea una directory
void crea_cartella(char *indirizzo_cartella, string nome_cartella);

#endif