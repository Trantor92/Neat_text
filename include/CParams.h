//#ifndef CPARAMS_H
//#define CPARAMS_H

#pragma once

//------------------------------------------------------------------------
//
//	Name: CParams.h
//
//  Desc: Classe per gestire tutti i dati e parametri della simulazione.
//        I valori sono caricati da un ini file, params.
//       
//
//------------------------------------------------------------------------

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <map>

#include "utils.h"

#ifdef VIEWER
#include <windows.h>
#endif

using namespace std;	


enum mod_type { MODO_LOCALE, MODO_GLOBALE, MODO_BATCH, MODO_BATCH_INCREMENTALE, MODO_BATCH_MOBILE };

enum update_type { MODO_ZERO, MODO_AGAIN };

enum restart { ONE_SPECIES, CHANGE };

class CParams
{

public:

  //-------------------------------------------------------------------
  //  parametri generali
  //-------------------------------------------------------------------
	 
  static bool is_Random;//se la simulazione deve essere riproducibile o meno

  static float dPi;
  static float dHalfPi;
  static float dTwoPi;

#ifdef VIEWER
  static int    WindowWidth;
  static int    WindowHeight;

  static int    InfoWindowWidth;
  static int    InfoWindowHeight;
#endif // VIEWER



  //----------------------------------------------------------------------
  // usati in phenotype.h/cpp
  //----------------------------------------------------------------------

  static int    iNumInputs;
  static int    iNumOutputs;
  
  //starting value per il beta della funzione di attivazione
  static float dSigmoidResponse;

  //numero massimo di iterazione ricorrenti per cacolare la depth, dopo viene considerata ciclica
  static int	i_MaxDepth;



  //----------------------------------------------------------------------
  // usati in genotype.h/cpp
  //----------------------------------------------------------------------
  
  //numero di tentativi per mutare un genoma aggiungerndo un link.
  //see CGenome::AddLink()
  static int    iNumAddLinkAttempts;
  static float dChanceAddRecurrentLink;
  static int iNumTrysToFindLoopedLink;


  //probabilità di aggiungere un link o nodo
  static float dChanceAddLink;
  static float dChanceAddNode;

  //probabilità di mutare i pesi sinaptici
  //see CGenome::Mutate()
  static float dMutationRate;
  static float dMaxWeightPerturbation;
  static float dProbabilityWeightReplaced;

  //probabilità di mutare la funzione di attivazione
  static float dActivationMutationRate;
  static float dMaxActivationPerturbation;

  //soglia di compatibilità per la speciazione
  static float dCompatibilityThreshold;
  static float dCompatibilityThreshold_initial;



  //----------------------------------------------------------------------
  // usati in CSpecies.h/cpp
  //----------------------------------------------------------------------

  //bonus per le specie giovani
  static float dYoungFitnessBonus;

  //soglia per giudicare le specie giovani
  static int    iYoungBonusAgeThreshhold;

  //come vengono estratte le specie nella procedura di crossover
  //see CSpecies::Spawn()
  static float dSurvivalRate;

  //penalizzazione per le specie vecchie
  static float dOldAgePenalty;

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
  static float dCrossoverRate;

  //numero massimo di specie ammesse
  static int    iMaxNumberOfSpecies;


  //---------------------------------------------


  //////////// DATA SET ////////////////////////////

  static string iNameTraining, iNameTest; //nomi dei file che contengono i dati

  //dati

  static vector<char> TrainingInputs;
  static vector<char> TestInputs;

  static std::map<char, int> dictionary;
  static std::map<int, int> elenco_best;

  //modalità di addestramento
  static mod_type ModAddestramento;
  static update_type ModUpdate;
  
  static float soglia_prestazioni;

  /*static vector<vector<float>> TrainingInputs;
  static vector<vector<float>> TrainingOutputs; static vector<string> squadre_train;
  static vector<vector<float>> TestInputs;
  static vector<vector<float>> TestOutputs; static vector<string> squadre_test;*/


  static bool isNorm;//se effettuare normalizzazione dell'array delle features o acquisirlo tal quale


  //costruttore di default
  CParams(){}

  //inizializza tutti i valori della classe dal file params.ini
  bool Initialize()
  {
    if(!LoadInParameters((char*)"params.ini"))
    {
#ifdef VIEWER
		MessageBox(NULL, L"Cannot find 'params.ini'", L"Error", 0);
#else
		cout << "Cannot find 'params.ini'" << endl;
#endif // VIEWER

      return false;
    }

    dPi     = 3.14159265358979f;
    dHalfPi = dPi / 2.f;
    dTwoPi  = dPi * 2.f;

  

    return true;

  }

  //legge e carica da file
  bool LoadInParameters(char* szFileName);
};

//#endif