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


bool CBrain::Update(vector<char> Inputs)
{//questa funzione non va bene bisogna correggerla e forse anche il endofruncalculation relativo
	//bisogna portare il for principale da zero e gli output hanno la dimensione esattamente uguale a quella degl
	//input, non di uno inferiore

	
	//dFitness = 0.f;//importante per il calcolo della fitness


	vector<float> input; input.resize(CParams::iNumInputs,0.f);



	output_char.resize(Inputs.size()+1);
	output_char[0] = (Inputs[0]);


	outputs.resize(Inputs.size());

	float error, w_postc;
	bool is_wrong = false;

	for (int c = 0; c < Inputs.size(); c++)//per tutti caratteri nel testo di training
	{

		//ENCODING DEL CHAR
		int pos_hot = Encoding_Char(Inputs[c]);

		input[pos_hot] = 1.f; //input codificato 


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

			w_postc = 0.05f / (c - Inputs.size() + 1);
		}*/

		
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
		
		input[pos_hot] = 0.f;

		pos_hot = std::distance(outputs[c].begin(), max_element(outputs[c].begin(), outputs[c].end()));
		input[pos_hot] = 1.f;//metto in input nel prossimo turno la previsione del turno corrente
		output_char[c+1] = Decoding_Char(pos_hot);
		input[pos_hot] = 0.f;


		//solo con versione globale
		/*input[pos_hot] = 0.f;
		pos_hot = Encoding_Char(Inputs[c]);
		input[pos_hot] = 1.f;*/

		
	}

	return true;
}

bool CBrain::Update_Batch(vector<char> Inputs, int size_batch)
{
	//fare i resize necessari
	//le batchate diverse sono Inputs.size - size_batch
	int size_input = Inputs.size();

	vector<char> input_batch; input_batch.resize(size_batch);
	//costruire il vector di input_batch

	int count = 0;

	outputs.resize(size_input - size_batch);

	output_char.resize(size_input);
	for (int i = 0; i < size_batch; i++)
		output_char[i] = Inputs[i];


	m_dFitness_batch = 0.f;
	m_dFitness_batch_perc = 0.f;

	while (size_input - (size_batch + count))
	{
		//costruisco l'input

		for (int i = 0; i < size_batch; i++)
		{
			input_batch[i] = Inputs[count + i];
		}

		outputs[count] = Update_insidebatch(input_batch);

		m_dFitness_batch += EndOfRunCalculations_insideBatch(input_batch);

		m_pItsBrain->Reset_activation();//pone a zero le attivazioni dei nodi

		output_char[size_batch + count] = Decoding_Char(std::distance(outputs[count].begin(),
			max_element(outputs[count].begin(), outputs[count].end())));

		index_act++;
		++count;
	}

	m_dFitness_batch /= count;
	m_dFitness_batch_perc /= count;

	return true;
}

vector<float> CBrain::Update_insidebatch(vector<char> Inputs)
{

	vector<float> input; input.resize(CParams::iNumInputs, 0.f);

	Update_until(index_act);//porta le attivazioni al punto descritto da index act 

	/*output_char.resize(Inputs.size());
	output_char[0] = (Inputs[0]);*/

	vector<float> last_output;

	outputs_batch.resize(Inputs.size());

	float error, w_postc;
	bool is_wrong = false;

	for (int c = 0; c < Inputs.size(); c++)//per tutti caratteri nel testo di training
	{
		//ENCODING DEL CHAR
		int pos_hot = Encoding_Char(Inputs[c]);
		input[pos_hot] = 1.f; //input codificato 

		//ho il vector di input codificato one-hot
		last_output = (m_pItsBrain->Update(input, CNeuralNet::active));

		outputs_batch[c] = last_output;

		//solo con versione globale
		input[pos_hot] = 0.f;
	}

	outputs_batch.pop_back();

	return last_output;
}

void CBrain::Update_until(int index)
{
	vector<float> input; input.resize(CParams::iNumInputs, 0.f);



	for (int c = 0; c < index; c++)//per tutti caratteri nel testo di training
	{

		//ENCODING DEL CHAR
		int pos_hot = Encoding_Char(CParams::TrainingInputs[c]);

		input[pos_hot] = 1; //input codificato 



		//ho il vector di input codificato one-hot
		(m_pItsBrain->Update(input, CNeuralNet::active));

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


		//solo con versione globale
		input[pos_hot] = 0.f;


	}
}

float CBrain::EndOfRunCalculations_insideBatch(vector<char> True_Outputs)
{
	float sum, sum_perc;
	vector<float> true_outputs, vec_sum;

	sum = sum_perc = 0.f; vec_sum.resize(CParams::iNumOutputs, 0.f);
	true_outputs.resize(CParams::iNumOutputs, 0.f);

	int pos_hot;

	for (int i_row = 0; i_row < outputs_batch.size(); i_row++)
	{
		pos_hot = Encoding_Char(True_Outputs[i_row + 1]);
		true_outputs[pos_hot] = 1.f;

		//calcolo della componente in sigma, sul numero di output
		for (int i_col = 0; i_col < outputs_batch[i_row].size(); i_col++)
		{
			vec_sum[i_col] += pow(outputs_batch[i_row][i_col] - true_outputs[i_col], 2.f);
		}

		true_outputs[pos_hot] = 0.f;


		//calcolo se la ha presa
		sum_perc += (pos_hot == std::distance(outputs_batch[i_row].begin(),
			max_element(outputs_batch[i_row].begin(), outputs_batch[i_row].end()))) ? 1.f : 0.f;

	}

	//calcolo della sigma2 media
	for (int i = 0; i < vec_sum.size(); i++)
	{
		vec_sum[i] /= outputs_batch.size();//e ottengo le sigma di ogni output

		sum += vec_sum[i];

		//mean_sqe[i] = vec_sum[i];
	}

	sum /= CParams::iNumOutputs; //ottenfo la sigma media sugli output


								 //Rate%
	sum_perc /= outputs_batch.size();

	

	/*if (!is_test)//Training Set
	{
	m_dFitness = result;
	}
	else        //Test Set
	{
	m_dFitness_test = result;
	}*/

	m_dFitness_batch_perc += 100 * sum_perc;

	return 100 * ((1.f / 2.f)*sum_perc + (1.f / 2.f)*(1.f - sum));
}

bool CBrain::Update_test(vector<char> TestInputs)
{

	vector<float> input; input.resize(CParams::iNumInputs, 0.f);

	int pos_hot;

	//ENCODING DEI CHAR inizializzanti
	int size_test = TestInputs.size();
	int size_prev = CParams::TrainingInputs.size();

	output_char.resize(size_test + size_prev);

	vector<float> output_seed;

	for (int c = 0; c < size_test; ++c)
	{
		pos_hot = Encoding_Char(TestInputs[c]);

		input[pos_hot] = 1.f; //input codificato 

		output_seed = m_pItsBrain->Update(input, CNeuralNet::active);

		input[pos_hot] = 0.f;

		output_char[c] = TestInputs[c];
	}
	
	//output_char[0] = (Inputs[0]);

	float temperature = 0.2f;

	pos_hot = Softmax(output_seed, temperature);//volendo bisognerebbe generare il carattere in modo casuale con probabilita
	//data dal softmax

	//pos_hot = std::distance(output_seed.begin(), max_element(output_seed.begin(), output_seed.end()));
	input[pos_hot] = 1.f;//metto in input nel prossimo turno la previsione del turno corrente

	output_char[size_test] = Decoding_Char(pos_hot);

	outputs.resize(size_prev-1);

	for (int c = 0; c < size_prev-1; c++)//per tutti caratteri nel testo di training
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

		

		//pos_hot = Softmax(outputs[c]);//volendo bisognerebbe generare il carattere in modo casuale con probabilita
		                                  //data dal softmax

		/*if ((!is_wrong) && (pos_hot != Encoding_Char(Inputs[c])))
		{
			is_wrong = true;

			m_dFitness -= error;//modalità di ottimizzazione solo dell'ultimo char
		}*/
		input[pos_hot] = 0.f;

		pos_hot = Softmax(outputs[c], temperature);//volendo bisognerebbe generare il carattere in modo casuale con probabilita
		//data dal softmax

		//pos_hot = std::distance(outputs[c].begin(), max_element(outputs[c].begin(), outputs[c].end()));
		input[pos_hot] = 1.f;//metto in input nel prossimo turno la previsione del turno corrente

		
		
		output_char[size_test + 1 + c] = Decoding_Char(pos_hot);

	}

	return true;



}


//------------------------- EndOfRunCalculations() -----------------------
//
//Calcola la Fitness e la carica nella variabile private
//restituisce Rate% poichè serve esternamente alla classe
//------------------------------------------------------------------------
float CBrain::EndOfRunCalculations_locale(vector<char> Inputs, bool is_test)
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
		
		/*if ((is_wrong) && (output_char[c] == Inputs[c]))//modalità globale
		{
			m_dFitness += w_postc;
		}
		*/
	}

	return m_dFitness;
}

float CBrain::EndOfRunCalculations_globale(vector<char> Inputs, bool is_test)
{
	m_dFitness = 0.f;//importante per il calcolo della fitness


	float error, sum;
	bool is_wrong = false;

	int group = 1;

	for (int c = 1; c < Inputs.size(); c++)//per tutti caratteri nel testo di training
	{
		error = Calcola_Scarto(Encoding_Char(Inputs[c]), outputs[c - 1]);

		if (output_char[c] != Inputs[c])
		{
			is_wrong = true;

			group = 0;
		}

		if (group)//se è corretta
		{
			int counter = group;

			sum = 0.f;

			while(counter)
			{
				//m_dFitness += (counter == group)? counter : 1.f;

				sum += (counter == group) ? counter : 1.f;

				--counter;
			}

			m_dFitness += sum / (c - group + 1);
		}
		else {
			m_dFitness += (1.f - error) / c;
		}

		++group;

	}

	return m_dFitness;
}

float CBrain::EndOfRunCalculations_Batch(vector<char> True_Outputs, int size_batch, bool is_test)
{
	float sum, sum_perc;
	vector<float> true_outputs, vec_sum;

	sum = sum_perc = 0.f; vec_sum.resize(CParams::iNumOutputs, 0.f);
	true_outputs.resize(CParams::iNumOutputs, 0.f);

	int pos_hot;

	int size_prev = outputs.size();

	for (int i_row = 0; i_row < outputs.size(); i_row++)
	{
		pos_hot = Encoding_Char(True_Outputs[i_row + size_batch]);
		true_outputs[pos_hot] = 1.f;

		//calcolo della componente in sigma, sul numero di output
		for (int i_col = 0; i_col < outputs[i_row].size(); i_col++)
		{
			vec_sum[i_col] += pow(outputs[i_row][i_col] - true_outputs[i_col], 2.f);
		}

		true_outputs[pos_hot] = 0.f;
		

		//calcolo se la ha presa
		sum_perc += (pos_hot == std::distance(outputs[i_row].begin(),
			max_element(outputs[i_row].begin(), outputs[i_row].end()))) ? 1.f : 0.f;

	}

	//calcolo della sigma2 media
	for (int i = 0; i < vec_sum.size(); i++)
	{
		vec_sum[i] /= outputs.size();//e ottengo le sigma di ogni output

		sum += vec_sum[i];

		mean_sqe[i] = vec_sum[i];
	}

	sum /= CParams::iNumOutputs; //ottenfo la sigma media sugli output


	//Rate%
	sum_perc /= outputs.size();

	//Fitness
	m_dFitness = 100 * ((1.f / 2.f)*sum_perc + (1.f / 2.f)*(1.f - sum));

	m_dFitness = (size_prev /(float)(size_batch + size_prev))*m_dFitness + 
		(size_batch / (float)(size_batch + size_prev))*m_dFitness_batch;

	/*if (!is_test)//Training Set
	{
		m_dFitness = result;
	}
	else        //Test Set
	{
		m_dFitness_test = result;
	}*/

	return m_dFitness_perc = 100.f * sum_perc;
}


void CBrain::Reset()
{
	for (int i = 0; i < outputs.size(); i++)
		outputs[i].clear();

	outputs.clear();

	m_pItsBrain->Reset_activation();//pone a zero le attivazioni dei nodi

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
		(is_train) ? (out << output_char[row]) : 
			((row == CParams::TestInputs.size()) ? out << "\n------------------\n" <<output_char[row]
				: out << output_char[row]);
	}

	return true;
}
