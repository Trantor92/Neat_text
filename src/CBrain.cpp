#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\CBrain.h"


//-----constructor-------
CBrain::CBrain()
{
	m_dFitness = 0; 
	m_dFitness_test = 0;
}


//-------------------------------Update()--------------------------------
//
//outputs deve avere size nulla poichè si utilizza il push_back, ciò viene fatto in Reset.
//carica in outputs i valori di attivazione dei nodi output, per ogni evento caricato in Input
//-----------------------------------------------------------------------
bool CBrain::Update(vector<vector<double>> Inputs)
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
}


//------------------------- EndOfRunCalculations() -----------------------
//
//Calcola la Fitness e la carica nella variabile private
//restituisce Rate% poichè serve esternamente alla classe
//------------------------------------------------------------------------
double CBrain::EndOfRunCalculations(vector<vector<double>> TrueOutputs, bool is_test)
{
	double sum, sum_perc;
	vector<double> vec_sum;

	int i_max, i_min;

	double max, min;
	
	sum = sum_perc = 0; vec_sum.resize(TrueOutputs[0].size(),0);

	for (int i_row = 0; i_row < TrueOutputs.size(); i_row++)
	{
		//calcolo della componente in sigma
		for (int i_col = 0; i_col < TrueOutputs[i_row].size(); i_col++)
		{
			vec_sum[i_col] += pow(outputs[i_row][i_col] - TrueOutputs[i_row][i_col], 2.);
		}

		//calcolo della componente in Rate%
		max = -1; min = 2;
		for (int i_col = 0; i_col < TrueOutputs[i_row].size(); i_col++)
		{
			if (max < outputs[i_row][i_col])
			{
				max = outputs[i_row][i_col];

				i_max = i_col;
			}

			if (min > outputs[i_row][i_col])
			{
				min = outputs[i_row][i_col];

				i_min = i_col;
			}

		}
		

		//numero di eventi tale che l'attivazione di output massima coincide con l'evento che si è verificato
		if (TrueOutputs[i_row][i_max])
			sum_perc++;
	}

	//calcolo della sigma2 media
	for (int i = 0; i < vec_sum.size(); i++)
	{
		vec_sum[i] /= TrueOutputs.size();

		sum += vec_sum[i];

		mean_sqe.push_back(vec_sum[i]);
	}

	sum /= CParams::iNumOutputs;


	//Rate%
	sum_perc /= TrueOutputs.size();

	//Fitness
	double result = 100 * ((1. / 2)*sum_perc + (1. / 2)*(1 - sum));
	

	
	if(!is_test)//Training Set
	{
		 m_dFitness = result;
	}
	else        //Test Set
	{
		m_dFitness_test = result;
	}
	
	return 100 * sum_perc;
}


void CBrain::Reset()
{
	for (int i = 0; i < outputs.size(); i++)
		outputs[i].clear();

	outputs.clear();

	mean_sqe.clear();
};


//------------------------- Write_output() -----------------------
//
//Scrive su file gli input, i true output e le attivazioni dei nodi di output
//------------------------------------------------------------------------
bool CBrain::Write_output(string name_file_output, bool is_train)
{
/*	//apertura del file in scrittura
	ofstream out(name_file_output);

	if (!out)
		return false;


	///////////////// SCRITTURA ///////////////////////////
	out << outputs.size() << endl;

	for (int row = 0; row < outputs.size(); row++)
	{
		if(is_train)//training set
		{
			out << CParams::squadre_train[row] << "\t";

			for (int col = 0; col < CParams::TrainingInputs[row].size(); col++)//input
			{
				out << CParams::TrainingInputs[row][col] << "\t";
			}

			out << endl;

			for (int col = 0; col < CParams::TrainingOutputs[row].size(); col++)//trueoutput
			{
				out << CParams::TrainingOutputs[row][col] << "\t";
			}

			out << endl;

			for (int col = 0; col < outputs[row].size(); col++)//outputs
			{
				out << outputs[row][col] << "\t";
			}

			out << endl;
		}
		else//test set
		{
			out << CParams::squadre_test[row] << "\t";

			for (int col = 0; col < CParams::TestInputs[row].size(); col++)
			{
				out << CParams::TestInputs[row][col] << "\t";
			}

			out << endl;

			for (int col = 0; col < CParams::TestOutputs[row].size(); col++)
			{
				out << CParams::TestOutputs[row][col] << "\t";
			}

			out << endl;

			for (int col = 0; col < outputs[row].size(); col++)
			{
				out << outputs[row][col] << "\t";
			}

			out << endl;
		}
		
	}*/

	return true;
}