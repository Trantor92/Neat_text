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

int Softmax(vector<float> &output)//esegue il softmax di un array, modificandone i valori e restituendo la posizione
								  // del valore del massimo
{
	float sum = 0.f;

	int pos_max = 0;
	float max = output[pos_max];

	for (int i = 0; i < output.size(); i++)
	{
		sum += exp(output[i]);

		if (output[i] > max)
		{
			max = output[i];

			pos_max = i;
		}
	}

	/*for (int i = 0; i < output.size(); i++)//rinormalizza l'output
	{
		output[i] /= sum;
	}*/

	return pos_max;
}

float Calcola_Scarto(int pos_true, vector<float> prediction)
{
	float err=0.f;

	for (int i = 0; i < prediction.size(); ++i)
		err += (i == pos_true) ? fabs(1 - prediction[i]) : fabs(prediction[i]);


	/*for (int i = 0; i < prediction.size(); ++i)
		err += (i == pos_true) ? pow(1 - prediction[i], 2) : pow(prediction[i],2);*/


	return err /= prediction.size();
}