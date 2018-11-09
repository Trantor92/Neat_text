#include "utils.h"
#include "CParams.h"
#include <math.h>


//crea una cartella al indirizzo_cartella nominando con nome_cartella
void crea_cartella(char *indirizzo_cartella, string nome_cartella)
{//OK
	int i, j;
	int n;
	char stringa[200];
	char comando[] = { 'M','K','D','I','R',' ' };

	n = (sizeof comando) / (sizeof comando[0]);

	for (j = 0; j < n; j++)
		stringa[j] = comando[j];

	i = 0;
	while (indirizzo_cartella[i] != '\0')
		i++;

	for (j = 0; j < i; j++)
		stringa[n + j] = indirizzo_cartella[j];


	n += i;

	j = 0;
	while (nome_cartella[j] != '\0')
		j++;


	for (i = 0; i < j + 1; i++)
		stringa[n + i] = nome_cartella[i];

	system(stringa);//esegue la stringa su riga di comando
}

int Softmax(vector<float> &output, float temperature)//esegue il softmax di un array, modificandone i valori e restituendo la posizione
								  // del valore del massimo
{
	float sum = 0.f;

	for (int i = 0; i < output.size(); i++)
	{
		sum += pow(exp(output[i]), 1.f/temperature);
	}

	float sum_prob = 0.f;
	//bool is_inf = true;

	float prob = RandFloat();

	for (int i = 0; i < output.size(); i++)//rinormalizza l'output e fornisce il carattere di output
	{
		output[i] = pow(exp(output[i]), 1.f / temperature)/sum;
		sum_prob += output[i];

		if (prob < sum_prob)
			return i;

	}
}

float Calcola_Scarto(int pos_true, vector<float> prediction)
{
	float err=0.f;

	/*if (CParams::ModAddestramento == MODO_BATCH || CParams::ModAddestramento == MODO_GLOBALE)
	{
		for (int i = 0; i < prediction.size(); ++i)
			err += (i == pos_true) ? pow(1 - prediction[i], 2) : pow(prediction[i], 2);
	}
	else
	{
		for (int i = 0; i < prediction.size(); ++i)
			err += (i == pos_true) ? fabs(1 - prediction[i]) : fabs(prediction[i]);

	}
	*/
	for (int i = 0; i < prediction.size(); ++i)
		err += (i == pos_true) ? pow(1 - prediction[i], 2) : pow(prediction[i], 2);
		

	return err /= prediction.size();
}


int Encoding_Char(char c)
{
	/*piccola, 32 caratteri*/
	/*return (c >= 'a') ? (c + 6 - 'a') :
		((c == '\n') ? 0 : ((c == ' ') ? 1 :
		((c == '!') ? 2 : ((c == ',') ? 3 :
			((c == '.') ? 4 :  5)))));*/

	/*media, solo caratteri stampabili più '\n', 97 caratteri*/
	//return (c == '\n') ? 0 : c - 31;

	/*completa, 256 caratteri*/
	//return (int)c + 128;

	std::map<char, int>::iterator it = CParams::dictionary.find(c);

	return it->second;

}

char Decoding_Char(int pos)
{
	/*piccola*/
	/*return (pos >= 6) ? (pos - 6 + 'a') :
		((pos == 0) ? '\n' : ((pos == 1) ? ' ' :
		((pos == 2) ? '!' : ((pos == 3) ? ',' :
			((pos == 4) ? '.' : '?')))));/*

	/*media*/
	//return (pos == 0) ? '\n' : pos + 31;


	/*completa*/
	//return (char)pos - 128;


	std::map<char, int>::iterator it = CParams::dictionary.begin();

	while (it->second != pos)
		++it;

	return it->first;
}