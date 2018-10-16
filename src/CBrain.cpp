#include "CBrain.h"


//-----constructor-------
CBrain::CBrain()
{
	m_dFitness = 0.f; 
	m_dFitness_test = 0.f;

	mean_sqe.resize(CParams::iNumOutputs);
}

CBrain::~CBrain()
{
	;
}

//-------------------------------Update()--------------------------------
//
//outputs deve avere size nulla poichè si utilizza il push_back, ciò viene fatto in Reset.
//carica in outputs i valori di attivazione dei nodi output, per ogni evento caricato in Input
//-----------------------------------------------------------------------
/*bool CBrain::Update(vector<vector<float>> Inputs)
{

	if (GetPhenotype()->GetDepth() != -1)//profondità -1 è il valore di errore per dire che la rete è aciclica
	{
		for (int i_example = 0; i_example < Inputs.size(); i_example++)
		{
			outputs.push_back(m_pItsBrain->Update(Inputs[i_example], CNeuralNet::snapshot));
		}
	}
	else
	{//qualcosa è andato storto ed è arrivata fino a qui una rete aciclica
		MessageBox(NULL, L"Error in Update a Brain, la rete è ciclica", L"ERROR", MB_OK);
	}

	return true;
}*/


bool CBrain::Update(vector<char> Inputs)
{
	//dFitness = 0.f;//importante per il calcolo della fitness


	vector<float> input; input.resize(CParams::iNumInputs,0.f);

	//ENCODING DEL CHAR
	int pos_hot = Encoding_Char(Inputs[0]);

	input[pos_hot] = 1; //input codificato 


	output_char.resize(Inputs.size());
	output_char[0] = (Inputs[0]);


	outputs.resize(Inputs.size() - 1);

	float error, w_postc;
	bool is_wrong = false;

	for (int c = 1; c < Inputs.size(); c++)//per tutti caratteri nel testo di training
	{
		//ho il vector di input codificato one-hot
		outputs[c-1] = (m_pItsBrain->Update(input, CNeuralNet::active));

		//int pos_out = Softmax(outputs[c - 1]);//esegue la softmax sull'ouputs e restituisce la pos del max, magari definirla in utils.h

		/*if (Inputs[c] + 128 == pos_out)
			m_dFitness++;*/

		/*if (!is_wrong)
		{
			error = Calcola_Scarto(Encoding_Char(Inputs[c]), outputs[c - 1]);

			//m_dFitness += (1 - error);//modalità 1 che fa la cosa per tutti

			m_dFitness += 1.f;//modalità di ottimizzazione solo dell'ultimo char

			w_postc = 0.05f / (c - Inputs.size() + 1);
		}*/

		input[pos_hot] = 0.f;

		//pos_hot = Softmax(outputs[c - 1]);


		/*

		if ((!is_wrong) && (pos_hot != Encoding_Char(Inputs[c])))
		{
			is_wrong = true;

			m_dFitness -= error;//modalità di ottimizzazione solo dell'ultimo char
		}
		
		if ((is_wrong) && (pos_hot == Encoding_Char(Inputs[c])))//modalità globale
		{
			m_dFitness += w_postc;
		}
		*/
		
		pos_hot = std::distance(outputs[c - 1].begin(), max_element(outputs[c - 1].begin(), outputs[c - 1].end()));
		input[pos_hot] = 1.f;//metto in input nel prossimo turno la previsione del turno corrente
		output_char[c] = Decoding_Char(pos_hot);



		//solo con versione globale
		input[pos_hot] = 0.f;
		pos_hot = Encoding_Char(Inputs[c]);
		input[pos_hot] = 1.f;

		
	}

	return true;
}

bool CBrain::Update_test()
{

	vector<float> input; input.resize(CParams::iNumInputs, 0.f);

	//ENCODING DEL CHAR inizializzante
	int pos_hot = Encoding_Char('\n');

	input[pos_hot] = 1; //input codificato 


	int size_prev = CParams::TrainingInputs.size();
	
	output_char.resize(size_prev);
	//output_char[0] = (Inputs[0]);

	outputs.resize(size_prev);

	for (int c = 0; c < size_prev; c++)//per tutti caratteri nel testo di training
	{
		//ho il vector di input codificato one-hot
		outputs[c] = (m_pItsBrain->Update(input, CNeuralNet::active));

		//int pos_out = Softmax(outputs[c - 1]);//esegue la softmax sull'ouputs e restituisce la pos del max, magari definirla in utils.h

		/*if (Inputs[c] + 128 == pos_out)
		m_dFitness++;*/

		/*if (!is_wrong)
		{
			error = Calcola_Scarto(Encoding_Char(Inputs[c]), outputs[c - 1]);

			//m_dFitness += (1 - error);//modalità 1 che fa la cosa per tutti

			m_dFitness += 1.f;//modalità di ottimizzazione solo dell'ultimo char
		}*/

		input[pos_hot] = 0.f;

		pos_hot = Softmax(outputs[c]);//volendo bisognerebbe generare il carattere in modo casuale con probabilita
		                                  //data dal softmax

		/*if ((!is_wrong) && (pos_hot != Encoding_Char(Inputs[c])))
		{
			is_wrong = true;

			m_dFitness -= error;//modalità di ottimizzazione solo dell'ultimo char
		}*/



		//pos_hot = *max_element(outputs[c - 1].begin(), outputs[c - 1].end());//vedere se funziona
		input = outputs[c];//metto in input nel prossimo turno la previsione del turno corrente

		output_char[c] = Decoding_Char(pos_hot);
	}

	return true;



}


//------------------------- EndOfRunCalculations() -----------------------
//
//Calcola la Fitness e la carica nella variabile private
//restituisce Rate% poichè serve esternamente alla classe
//------------------------------------------------------------------------
float CBrain::EndOfRunCalculations(vector<char> Inputs, bool is_test)
{
	m_dFitness = 0.f;//importante per il calcolo della fitness


	float error, w_postc;
	bool is_wrong = false;

	for (int c = 1; c < Inputs.size(); c++)//per tutti caratteri nel testo di training
	{
		error = Calcola_Scarto(Encoding_Char(Inputs[c]), outputs[c - 1]);

		//entra qui per i caratteri predetti di fila
		if (!is_wrong)
		{
			m_dFitness += 1.f;//modalità di ottimizzazione solo dell'ultimo char

			w_postc = 0.05f / (c - Inputs.size() + 1);
		}


		if ((!is_wrong) && (output_char[c] != Inputs[c]))
		{
			is_wrong = true;

			m_dFitness -= error;//modalità di ottimizzazione solo dell'ultimo char
		}
		
		if ((is_wrong) && (output_char[c] == Inputs[c]))//modalità globale
		{
			m_dFitness += w_postc;
		}
		
	}

	return m_dFitness;
}


void CBrain::Reset()
{
	for (int i = 0; i < outputs.size(); i++)
		outputs[i].clear();

	outputs.clear();

	//mean_sqe.clear();
};


//------------------------- Write_output() -----------------------
//
//Scrive su file gli input, i true output e le attivazioni dei nodi di output
//------------------------------------------------------------------------
bool CBrain::Write_output(string name_file_output, bool is_train)
{
	//apertura del file in scrittura
	ofstream out(name_file_output);

	if (!out)
		return false;


	///////////////// SCRITTURA ///////////////////////////
	//out << outputs.size() << endl;

	for (int row = 0; row < output_char.size(); row++)
	{
		(is_train) ? (out << output_char[row]) : (out << output_char[row]);
	}

	return true;
}

int CBrain::Encoding_Char(char c)
{
	/*piccola, 32 caratteri*/
	return (c >= 'a') ? (c + 6 - 'a') :
		((c == '\n') ? 0 : ((c == ' ') ? 1 :
		((c == '!') ? 2 : ((c == ',') ? 3 :
			((c == '.') ? 4 : /*'?'*/ 5)))));

	/*media, solo caratteri stampabili più '\n', 97 caratteri*/
	//return (c == '\n') ? 0 : c - 31;

	/*completa, 256 caratteri*/
	//return (int)c + 128;
}

char CBrain::Decoding_Char(int pos) 
{
	/*piccola*/
	return (pos >= 6) ? (pos - 6 + 'a') :
		((pos == 0) ? '\n' : ((pos == 1) ? ' ' :
		((pos == 2) ? '!' : ((pos == 3) ? ',' :
			((pos == 4) ? '.' : /*5*/ '?')))));

	/*media*/
	//return (pos == 0) ? '\n' : pos + 31;


	/*completa*/
	//return (char)pos - 128;
}