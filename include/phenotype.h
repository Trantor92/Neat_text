#ifndef PHENOTYPE_H
#define PHENOTYPE_H

//-----------------------------------------------------------------------
//
//  Name: phenotype.h
//
//	Desc: Definizioni per la creazione del fenotipo (reti neurali acicliche).
//        
//-----------------------------------------------------------------------

#include <vector>
#include <math.h>
#include <windows.h>
#include <algorithm>

#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\utils.h"
#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\CParams.h"
#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\genes.h"


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
  double  dWeight;

  //link ricorrente?
  bool    bRecurrent;



  //----- constructor -----------

  SLink(double dW, SNeuron* pIn, SNeuron* pOut, bool bRec) :dWeight(dW),
															pIn(pIn),
															pOut(pOut),
															bRecurrent(bRec)
  {}
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
  double        dOutput;

  //tipologia del neurone
  neuron_type   NeuronType;

  //nuemero d'identificazione
  int           iNeuronID;

  //parametro beta della funzione di attivazione di questo neurone
  double        dActivationResponse;

  //variabili utilizzate in fase grafica
  int           iPosX,   iPosY;
  double        dSplitY, dSplitX;


  //------------ construtor --------------------------------------
  SNeuron(neuron_type type,
          int         id,
          double      y,
          double      x,
          double      ActResponse):NeuronType(type),
                                   iNeuronID(id),
                                   dOutput(0),
                                   iPosX(0),
                                   iPosY(0),
                                   dSplitY(y),
                                   dSplitX(x),
                                   dActivationResponse(ActResponse)
  {}


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
  vector<double>  Update(const vector<double> &inputs, const run_type type);

  //funzione di attivazione del neurone
  double		  Sigmoid(float netinput, float response);



  //Disegna il fenotipo nella surface specificata
  void            DrawNet(HDC &surface,
                          int cxLeft,
                          int cxRight,
                          int cyTop,
                          int cyBot);



  //----------- accessor function ----------------------

  int			  GetDepth()const { return m_iDepth; }

};

#endif