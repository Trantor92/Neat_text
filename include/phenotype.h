//#ifndef PHENOTYPE_H
//#define PHENOTYPE_H

#pragma once

//-----------------------------------------------------------------------
//
//  Name: phenotype.h
//
//	Desc: Definizioni per la creazione del fenotipo (reti neurali acicliche).
//        
//-----------------------------------------------------------------------

#include <vector>
#include <math.h>
#include <algorithm>


#include "utils.h"
#include "CParams.h"
#include "genes.h"

#ifdef VIEWER
#include <windows.h>
#endif // VIEWER


using namespace std;



struct SNeuron;

//------------------------------------------------------------------------
//
//  SLink structure che definisce il link del fenotipo 
//------------------------------------------------------------------------
struct SLink
{
  //puntatori ai neuroni che questo link connette
  SNeuron*  pIn;
  SNeuron*  pOut;

  //peso sinaptico
  float  dWeight;

  //link ricorrente?
  bool    bRecurrent;



  //----- constructor -----------

  SLink(float dW, SNeuron* pIn, SNeuron* pOut, bool bRec) :dWeight(dW),
															pIn(pIn),
															pOut(pOut),
															bRecurrent(bRec)
  {}

  //~SLink();
};


//------------------------------------------------------------------------
//
//  SNeuron structure che definisce il nodo del fenotipo (neurone)
//------------------------------------------------------------------------
struct SNeuron
{
  //vector di tutti i link che entrano in questo neurone
  vector<SLink> vecLinksIn;

  //link che vi escono
  vector<SLink> vecLinksOut;

  //attivazione del neurone
  float        dOutput;

  //tipologia del neurone
  neuron_type   NeuronType;

  //nuemero d'identificazione
  int           iNeuronID;

  //parametro beta della funzione di attivazione di questo neurone
  float        dActivationResponse;

  //variabili utilizzate in fase grafica
  int           iPosX,   iPosY;
  float        dSplitY, dSplitX;


  //------------ construtor --------------------------------------
  SNeuron(neuron_type type,
          int         id,
          float      y,
          float      x,
          float      ActResponse):NeuronType(type),
                                   iNeuronID(id),
                                   dOutput(0.f),
                                   iPosX(0.f),
                                   iPosY(0.f),
                                   dSplitY(y),
                                   dSplitX(x),
                                   dActivationResponse(ActResponse)
  {}

  
  //~SNeuron();

};

//------------------------------------------------------------------------
//
//  CNeuralNet class che defisce il fenotipo utilizzando le strutture dei
//  neuroni e link
//------------------------------------------------------------------------
class CNeuralNet
{

private:
  
  //neuroni presenti nel fenotipo, questi contengono all'interno l'informazione sui link
  vector<SNeuron*>  m_vecpNeurons;

  //profondità del fenotipo
  int               m_iDepth;

public:
	
  //----- constructor ----------------
  CNeuralNet(vector<SNeuron*> neurons,
             int              depth);
								

  ~CNeuralNet();

  //E' necessario scegliere una di queste tipologie di esecuzione
  //della funzione Update(), che calcola le attivazioni dei neuroni dato l'input.
  //Si utilizza snapshot se si vuole che gli input percorrano tutta
  //la strada verso gli output (a tale scopo è necessaria l'informazione sulla profondità).
  //Si utilizza active se si vuole che le attivazioni evolvano solo ai primi vicini
  //(opzione da utilizzare nel caso si vogliano reti ricorrenti).
  enum run_type{snapshot, active};

  //esegue il calcolo della rete neurale, facendo 'diffondere' le attivazioni degli input
  vector<float>  Update(const vector<float> &inputs, const run_type type, bool no_bias = false);

  //funzione di attivazione del neurone
  float		  Sigmoid(float netinput, float response);

  void        Reset_activation();

#ifdef VIEWER
  //Disegna il fenotipo nella surface specificata
  void            DrawNet(HDC &surface,
	  int cxLeft,
	  int cxRight,
	  int cyTop,
	  int cyBot);
#endif // VIEWER


  vector<std::map<int,int>> Calcola_NodeRecurrency(int size_batch, int minRec);

  //bool Test_recurrency(std::map<int, int> &node_recurrency, vector<int> node_ignoti);

  //dato il numero identificativo di un nodo, questa funzione restituisce il suo indice
  //nel vector dei nodi m_vecpNeurons
  int     GetElementPos(int neuron_id);

  //----------- accessor function ----------------------

  int			  GetDepth()const { return m_iDepth; }

};

//#endif