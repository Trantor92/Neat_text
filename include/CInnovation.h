#ifndef CINNOVATION_H
#define CINNOVATION_H

//-----------------------------------------------------------------------
//
//  Name: CInnovation.h
//
//	Desc: classe per gestire il tracciamento storico delle innovazioni del genoma.
//
//-----------------------------------------------------------------------

#include <vector>
#include <algorithm>
#include <fstream>
#include <iomanip>

#include "utils.h"
#include "genotype.h"
#include "phenotype.h"

using namespace std;


struct SLinkGene;

/////////////////////// structs/classes utili ///////////////////////////////

enum innov_type//tipologia di innovazione
{
	new_neuron,
	new_link
};

//------------------------------------------------------------------------
//
//  structure che definisce una innovazione
//------------------------------------------------------------------------
struct	SInnovation
{
	//nuovo nodo o nuovo link?
	innov_type  InnovationType;

	int         InnovationID;
	
	int         NeuronIn;
	int         NeuronOut;
	
	int         NeuronID;

	neuron_type NeuronType;

	//Se l'innovazione è un nuovo nodo è necessario tenere traccia della sua posizione
	//(scopo grafico)
	float      dSplitY,
				dSplitX;


	//---------- constuctors --------------------------------

	SInnovation() {};

	SInnovation(int        in,
                int        out, 
                innov_type t, 
               int        inov_id):NeuronIn(in),
                                   NeuronOut(out),
                                   InnovationType(t),
                                   InnovationID(inov_id),
                                   NeuronID(0),
                                   dSplitX(0.f),
                                   dSplitY(0.f),
                                   NeuronType(none)
	{}

	 SInnovation(SNeuronGene neuron,
                 int          innov_id,
                 int          neuron_id):InnovationID(innov_id),
                                         NeuronID(neuron_id),
                                         dSplitX(neuron.dSplitX),
                                         dSplitY(neuron.dSplitY),
                                         NeuronType(neuron.NeuronType),
                                         NeuronIn(-1),
                                         NeuronOut(-1)
	{}
  
	 SInnovation(int         in,
                 int         out, 
                 innov_type  t, 
                 int         inov_id,
                 neuron_type type,
                 float      x,
                 float      y):NeuronIn(in),
                               NeuronOut(out),
                               InnovationType(t),
                               InnovationID(inov_id),
                               NeuronID(0),
                               NeuronType(type),
                               dSplitX(x),
                               dSplitY(y)
	 {}
};


//------------------------------------------------------------------------
//
//  CInnovation class è utlizzata per tenere traccia di tutte le innovazioni
//  che vengono genereate durante l'evoluzione della popolazione.
//------------------------------------------------------------------------
class CInnovation
{

private:
	
  vector<SInnovation> m_vecInnovs;

  int                 m_NextNeuronID;

  int                 m_NextInnovationNum;


public:

  CInnovation(vector<SLinkGene>   start_genes,
              vector<SNeuronGene> start_neurons);

  //CInnovation(CGenome &ancestor);
		
  //controlla se innovazione in argomento è gia presente nella lista. 
  //se così ritorna il numero d'innovazione corrispondente, altrimenti -1.
  vector<int>   CheckInnovation(int in, int out, innov_type type);

  //crea una nuova innovazione e ne restituise il numero d'innovazione
  int   CreateNewInnovation(int in, int out, innov_type type);

  int   CreateNewInnovation(int         from,
                            int         to,
                            innov_type  InnovType,
                            neuron_type NeuronType,
                            float      x,
                            float      y);

  //crea un nodo standard partire dal numero identificativo in argomento
  SNeuronGene CreateNeuronFromID(int id);

  //scrive la lista delle innovazioni su file
  bool        Write(const char* szFileName, const int gen);


  //------------------ accessor methods -----------------------
  int   GetNeuronID(int inv)const{return m_vecInnovs[inv].NeuronID;}

  void  Flush(){m_vecInnovs.clear(); return;}
  
  int   NextNumber(int num = 0)
  {
    m_NextInnovationNum += num;

    return m_NextInnovationNum;
  } 
};

#endif