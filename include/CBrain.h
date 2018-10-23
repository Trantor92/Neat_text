#pragma once

//------------------------------------------------------------------------
//
//	Name: CBrain.h
//
//  Desc: Classe che rappresenta gli individui, ovvero classificatori costituiti da reti neurali acicliche.
//       
//
//------------------------------------------------------------------------

#include <vector>
#include <math.h>

#include "phenotype.h"
#include "utils.h"
#include "CParams.h"


#define TRAIN 1
#define TEST 0

using namespace std;


class CBrain
{
private:

	//fenotipo associato
	CNeuralNet * m_pItsBrain;

	//fitness score
	float			m_dFitness;
	float          m_dFitness_test;

	
	//attivazioni dei nodi di output. Viene caricato in CBrain::Update()
	//vector<vector<float>> outputs;


	//attivazioni dei nodi di output. Viene caricato in CBrain::Update()
	vector<vector<float>> outputs;
	vector<vector<float>> outputs_batch;


public:

	float		m_dFitness_perc;
	float       m_dFitness_batch;
	float       m_dFitness_batch_perc;

	//array delle sigma2
	vector<float> mean_sqe;

	CBrain();
	~CBrain();

	//char predette
	vector<char> output_char;

	//calcola le attivazioni dei nodi di output per ogni array di Input
	//bool			Update(vector<vector<float>> Input);

	//calcola le attivazioni dei nodi di output per ogni array di Input
	bool	Update(vector<char> Input);

	bool Update_Batch(vector<char> Input, int size_batch);
	vector<float> Update_insidebatch(vector<char> Inputs);

	bool Update_test(vector<char> TestInput);


	int index_act; //indici dell'ultimo iput da mostrare alla rete per portarla nel corretto stato
	void Update_until(int index);

	//fra le altre cose, cancella outputs.
	void			Reset();
	
	//calcola la fitness, confrontando outputs a TrueOutputs
	float    EndOfRunCalculations_locale(vector<char> Inputs, bool is_test = false);
	float    EndOfRunCalculations_globale(vector<char> Inputs, bool is_test = false);

	float    EndOfRunCalculations_Batch(vector<char> Inputs, int size_batch, bool is_test = false);
	float    EndOfRunCalculations_insideBatch(vector<char> Inputs);

	//disegna il fenotipo associato al Brain
	void      DrawNet(HDC &surface, int cxLeft, int cxRight, int cyTop, int cyBot)
	{
		m_pItsBrain->DrawNet(surface, cxLeft, cxRight, cyTop, cyBot);
	}

	//scrive su file le Input, TrueOutput e outputs
	bool Write_output(string name_file_output, bool is_train);



	//-------------------accessor functions
	float Fitness()const { return m_dFitness; }
	float Fitness_test()const { return m_dFitness_test; }

	void SetFitness(float fitness) { m_dFitness = fitness; }

	void SetBrain(CNeuralNet* brain) { m_pItsBrain = brain; }

	void              InsertNewBrain(CNeuralNet* brain) {	
		if (m_pItsBrain != NULL)
			m_pItsBrain->~CNeuralNet();

		m_pItsBrain = brain;
	}
	/*void              InsertNewBrain1(CNeuralNet* brain) { 
		if(brain != m_pItsBrain)
			m_pItsBrain->~CNeuralNet();

		m_pItsBrain = brain; }
		*/
	CNeuralNet* GetPhenotype()const { return m_pItsBrain; }
};