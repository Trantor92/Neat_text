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
	double			m_dFitness;
	double          m_dFitness_test;

	//attivazioni dei nodi di output. Viene caricato in CBrain::Update()
	vector<vector<double>> outputs;

public:

	//array delle sigma2
	vector<double> mean_sqe;

	CBrain();

	//calcola le attivazioni dei nodi di output per ogni array di Input
	bool			Update(vector<vector<double>> Input);

	//fra le altre cose, cancella outputs.
	void			Reset();
	
	//calcola la fitness, confrontando outputs a TrueOutputs
	double    EndOfRunCalculations(vector<vector<double>> TrueOutputs, bool is_test = false);

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

	void SetFitness(double fitness) { m_dFitness = fitness; }

	void              InsertNewBrain(CNeuralNet* brain) { m_pItsBrain = brain; }

	CNeuralNet* GetPhenotype()const { return m_pItsBrain; }
};