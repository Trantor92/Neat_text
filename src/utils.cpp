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

