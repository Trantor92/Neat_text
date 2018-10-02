#ifndef CPARAMS_H
#define CPARAMS_H

//------------------------------------------------------------------------
//
//	Name: CParams.h
//
//  Desc: Classe per gestire tutti i dati e parametri della simulazione.
//        I valori sono caricati da un ini file, params.
//       
//
//------------------------------------------------------------------------

#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

#include "utils.h"

using namespace std;	



class CParams
{

public:

  //-------------------------------------------------------------------
  //  parametri generali
  //-------------------------------------------------------------------
	 
  static bool is_Random;//se la simulazione deve essere riproducibile o meno

  static double dPi;
  static double dHalfPi;
  static double dTwoPi;

  static int    WindowWidth;
  static int    WindowHeight;

  static int    InfoWindowWidth;
  static int    InfoWindowHeight;


  //----------------------------------------------------------------------
  // usati in phenotype.h/cpp
  //----------------------------------------------------------------------

  static int    iNumInputs;
  static int    iNumOutputs;
  
  //starting value per il beta della funzione di attivazione
  static double dSigmoidResponse;

  //numero massimo di iterazione ricorrenti per cacolare la depth, dopo viene considerata ciclica
  static int	i_MaxDepth;



  //----------------------------------------------------------------------
  // usati in genotype.h/cpp
  //----------------------------------------------------------------------
  
  //numero di tentativi per mutare un genoma aggiungerndo un link.
  //see CGenome::AddLink()
  static int    iNumAddLinkAttempts;

  //probabilità di aggiungere un link o nodo
  static double dChanceAddLink;
  static double dChanceAddNode;

  //probabilità di mutare i pesi sinaptici
  //see CGenome::Mutate()
  static double dMutationRate;
  static double dMaxWeightPerturbation;
  static double dProbabilityWeightReplaced;

  //probabilità di mutare la funzione di attivazione
  static double dActivationMutationRate;
  static double dMaxActivationPerturbation;

  //soglia di compatibilità per la speciazione
  static double dCompatibilityThreshold;



  //----------------------------------------------------------------------
  // usati in CSpecies.h/cpp
  //----------------------------------------------------------------------

  //bonus per le specie giovani
  static double dYoungFitnessBonus;

  //soglia per giudicare le specie giovani
  static int    iYoungBonusAgeThreshhold;

  //come vengono estratte le specie nella procedura di crossover
  //see CSpecies::Spawn()
  static double dSurvivalRate;

  //penalizzazione per le specie vecchie
  static double dOldAgePenalty;

  //soglia per giudicare le specie vecchie
  static int    iOldAgeThreshold;


  
  //----------------------------------------------------------------------
  // usati in Cga.h/cpp
  //----------------------------------------------------------------------

  //numero di individui nella popolazione
  static int    iPopSize;

  //numero di generazinoi concesse alle specie senza miglioramenti
  static int    iNumGensAllowedNoImprovement;

  //numero massimo di nodi ammessi per le reti
  static int    iMaxPermittedNeurons;

  //numero degli individui migliori da stampare
  static int    iNumBestBrains;

  //probabilità di eseguire crossover
  static double dCrossoverRate;

  //numero massimo di specie ammesse
  static int    iMaxNumberOfSpecies;


  //---------------------------------------------


  //////////// DATA SET ////////////////////////////

  static string iNameTraining, iNameTest; //nomi dei file che contengono i dati

  //dati
  static vector<vector<double>> TrainingInputs;
  static vector<vector<double>> TrainingOutputs; static vector<string> squadre_train;
  static vector<vector<double>> TestInputs;
  static vector<vector<double>> TestOutputs; static vector<string> squadre_test;


  static bool isNorm;//se effettuare normalizzazione dell'array delle features o acquisirlo tal quale


  //costruttore di default
  CParams(){}

  //inizializza tutti i valori della classe dal file params.ini
  bool Initialize()
  {
    if(!LoadInParameters((char*)"params.ini"))
    {
      MessageBox(NULL, L"Cannot find 'params.ini'", L"Error", 0);

      return false;
    }

    dPi     = 3.14159265358979;
    dHalfPi = dPi / 2;
    dTwoPi  = dPi * 2;

  

    return true;

  }

  //legge e carica da file
  bool LoadInParameters(char* szFileName);
};

#endif