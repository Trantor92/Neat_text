#include "CParams.h"


float CParams::dPi                       = 0.f;
float CParams::dHalfPi                   = 0.f;
float CParams::dTwoPi                    = 0.f;

#ifdef VIEWER
int CParams::WindowWidth = GetSystemMetrics(SM_CXSCREEN) / 2;
int CParams::WindowHeight = 400;
int CParams::InfoWindowWidth = GetSystemMetrics(SM_CXSCREEN) / 2;
int CParams::InfoWindowHeight = GetSystemMetrics(SM_CYSCREEN) * 0.9f;
#endif // VIEWER


int CParams::iNumInputs                   = 0;
int CParams::iNumOutputs                  = 0;
int CParams::iPopSize                     = 0;

float CParams::dSigmoidResponse          = 1.f;
int CParams::iNumAddLinkAttempts          = 0;

float CParams::dYoungFitnessBonus        = 0.f;
int CParams::iYoungBonusAgeThreshhold     = 0;
float CParams::dSurvivalRate             = 0.f;

int CParams::iNumGensAllowedNoImprovement = 0;
int CParams::iMaxPermittedNeurons         = 0;
float CParams::dChanceAddLink            = 0.f;
float CParams::dChanceAddNode            = 0.f;
float CParams::dChanceAddRecurrentLink = 0.f;
int CParams::iNumTrysToFindLoopedLink = 0;

float CParams::dMutationRate             = 0.f;
float CParams::dMaxWeightPerturbation = 0.f;
float CParams::dProbabilityWeightReplaced= 0.f;

float CParams::dActivationMutationRate   = 0.f;
float CParams::dMaxActivationPerturbation= 0.f;

float CParams::dCompatibilityThreshold   = 0.f;

int CParams::iOldAgeThreshold             = 0;
float CParams::dOldAgePenalty            = 0.f;
float CParams::dCrossoverRate            = 0.f;
int CParams::iMaxNumberOfSpecies          = 0;

int CParams::iNumBestBrains				  = 4;


int CParams::i_MaxDepth = 100;

string CParams::iNameTraining; 
string CParams::iNameTest;

//---------------------------------------------


vector<char> CParams::TrainingInputs;
vector<char> CParams::TestInputs;

std::map<char, int> CParams::dictionary;
std::map<int, int> CParams::elenco_best;

mod_type CParams::ModAddestramento = MODO_GLOBALE;
float CParams::soglia_prestazioni = 35.f;

/*vector<vector<float>> CParams::TrainingOutputs;
vector<vector<float>> CParams::TestOutputs;
vector<vector<float>> CParams::TrainingInputs;
vector<vector<float>> CParams::TestInputs;

vector<string> CParams::squadre_train; vector<string> CParams::squadre_test;*/

bool CParams::isNorm = true;
bool CParams::is_Random;


//questa funzinoe carica i parametri da file e carica i dati del dal data set.
//ritorna false se c'è stato un problema
bool CParams::LoadInParameters(char* szFileName)
{
  ifstream grab(szFileName);

  //controllo
  if (!grab)
  {
    return false;
  }

  //caricamento da file
  char ParamDescription[40];

 
  grab >> ParamDescription;
  grab >> is_Random;
  grab >> ParamDescription;
  grab >> iPopSize;
  grab >> ParamDescription;
  grab >> iMaxNumberOfSpecies;
  grab >> ParamDescription;
  grab >> iMaxPermittedNeurons;
  grab >> ParamDescription;
  grab >> iNumGensAllowedNoImprovement;


  grab >> ParamDescription;
  grab >> iYoungBonusAgeThreshhold;
  grab >> ParamDescription;
  grab >> dYoungFitnessBonus;
  grab >> ParamDescription;
  grab >> iOldAgeThreshold;
  grab >> ParamDescription;
  grab >> dOldAgePenalty;
  grab >> ParamDescription;
  grab >> dCompatibilityThreshold;
  

  grab >> ParamDescription;
  grab >> dSurvivalRate;
  grab >> ParamDescription;
  grab >> dCrossoverRate;
  grab >> ParamDescription;
  grab >> dChanceAddLink;
  grab >> ParamDescription;
  grab >> iNumAddLinkAttempts;
  grab >> ParamDescription;
  grab >> dChanceAddRecurrentLink;
  grab >> ParamDescription;
  grab >> iNumTrysToFindLoopedLink;
  grab >> ParamDescription;
  grab >> dChanceAddNode;
  grab >> ParamDescription;
  grab >> dMutationRate;
  grab >> ParamDescription;
  grab >> dProbabilityWeightReplaced;
  grab >> ParamDescription;
  grab >> dMaxWeightPerturbation;
  grab >> ParamDescription;
  grab >> dActivationMutationRate;
  grab >> ParamDescription;
  grab >> dMaxActivationPerturbation;
  



  grab >> ParamDescription;
  grab >> iNameTraining;
  grab >> ParamDescription;
  grab >> iNameTest;





  ////////////// LETTURA E CARICAMENTO //////////////////////////


 
  //apertura del file del Training Set
  ifstream train(iNameTraining);

  //controllo
  if (!train)
  {
	  return false;
  }

  //CParams::iNumInputs = CParams::iNumOutputs = 256;

  //CParams::iNumInputs = CParams::iNumOutputs = 97;

  //CParams::iNumInputs = CParams::iNumOutputs = 32;

  char c;
  int count = 0;

  while (!train.eof())
  {
	  train.get(c);//le char vanno da -128 a 127 usually (256 valori)

	  TrainingInputs.push_back(c);

	  if (!dictionary.count(c))
		  dictionary.insert(std::pair<char, int>(c, count++));	
  }

  //non so perche ma mi prende due volte l'ultimo carattere
  TrainingInputs.pop_back();


  ifstream test(iNameTest);

  while (!test.eof())
  {
	  test.get(c);//le char vanno da -128 a 127 usually (256 valori)

	  TestInputs.push_back(c);

	  if (!dictionary.count(c))
		  dictionary.insert(std::pair<char, int>(c, count++));
  }


  //non so perche ma mi prende due volte l'ultimo carattere
  TestInputs.pop_back();

  CParams::iNumInputs = CParams::iNumOutputs = dictionary.size();

  return true;
}
 






  
  
