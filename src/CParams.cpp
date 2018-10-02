#include "CParams.h"


double CParams::dPi                       = 0;
double CParams::dHalfPi                   = 0;
double CParams::dTwoPi                    = 0;
int CParams::WindowWidth                  = GetSystemMetrics(SM_CXSCREEN) / 2;
int CParams::WindowHeight                 = 400;
int CParams::iNumInputs                   = 0;
int CParams::iNumOutputs                  = 0;
int CParams::iPopSize                     = 0;

double CParams::dSigmoidResponse          = 1;
int CParams::iNumAddLinkAttempts          = 0;

double CParams::dYoungFitnessBonus        = 0;
int CParams::iYoungBonusAgeThreshhold     = 0;
double CParams::dSurvivalRate             = 0;
int CParams::InfoWindowWidth              = GetSystemMetrics(SM_CXSCREEN) / 2;
int CParams::InfoWindowHeight             = GetSystemMetrics(SM_CYSCREEN) * 0.9;
int CParams::iNumGensAllowedNoImprovement = 0;
int CParams::iMaxPermittedNeurons         = 0;
double CParams::dChanceAddLink            = 0;
double CParams::dChanceAddNode            = 0;

double CParams::dMutationRate             = 0;
double CParams::dMaxWeightPerturbation    = 0;
double CParams::dProbabilityWeightReplaced= 0;

double CParams::dActivationMutationRate   = 0;
double CParams::dMaxActivationPerturbation= 0;

double CParams::dCompatibilityThreshold   = 0;

int CParams::iOldAgeThreshold             = 0;
double CParams::dOldAgePenalty            = 0;
double CParams::dCrossoverRate            = 0;
int CParams::iMaxNumberOfSpecies          = 0;

int CParams::iNumBestBrains				  = 4;


int CParams::i_MaxDepth = 100;

string CParams::iNameTraining; 
string CParams::iNameTest;

//---------------------------------------------


vector<vector<double>> CParams::TrainingOutputs;
vector<vector<double>> CParams::TestOutputs;
vector<vector<double>> CParams::TrainingInputs;
vector<vector<double>> CParams::TestInputs;

vector<string> CParams::squadre_train; vector<string> CParams::squadre_test;

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



  int rows, n_out, n_in;

  double norm;

  train >> rows; train >> CParams::iNumInputs; train >> CParams::iNumOutputs;
  
  n_in = CParams::iNumInputs; n_out = CParams::iNumOutputs;

  string squadre;
 
  vector<double> temp; double lec;
  for (int i = 0; i < rows; i++)
  {
	  train >> ParamDescription; squadre = ParamDescription; squadre += "\t";
	  train >> ParamDescription; squadre += ParamDescription;

	  squadre_train.push_back(squadre);
	  
	  norm = 0;
	  for (int j = 0; j < n_in; j++)
	  {
		  train >> lec;

		  norm += pow(lec,2.);

		  temp.push_back(lec);
		  
	  }

	  norm = pow(norm, 0.5);

	  //eventuale normalizzazione
	  if (isNorm)
	  {
		  for (int j = 0; j < n_in; j++)
		  {
			  temp[j] /= norm;
		  }
	  }


	  TrainingInputs.push_back(temp); temp.clear();

	  for (int j = 0; j < n_out; j++)
	  {
		  train >> lec;
		  temp.push_back(lec);
	  }

	  TrainingOutputs.push_back(temp); temp.clear();

  
  }





  //apertura del file del Test Set
  ifstream test(iNameTest);

  //controllo
  if (!test)
  {
	  return false;
  }


  test >> rows; test >> n_in; test >> n_out;

  if (n_in != CParams::iNumInputs || n_out != CParams::iNumOutputs)
  {
	  cout << "Errore: input e/o output non combaciano fra training e test." << endl;
  }

 
  for (int i = 0; i < rows; i++)
  {
	  test >> ParamDescription; squadre = ParamDescription; squadre += "\t";
	  test >> ParamDescription; squadre += ParamDescription;

	  squadre_test.push_back(squadre);

	  norm = 0;
	  for (int j = 0; j < n_in; j++)
	  {
		  test >> lec;

		  norm += pow(lec, 2.);

		  temp.push_back(lec);

	  }

	  norm = pow(norm, 0.5);
	  
	  //eventuale normalizzazione
	  if (isNorm)
	  {
		  for (int j = 0; j < n_in; j++)
		  {
			  temp[j] /= norm;

		  }
	  }
	  

	  TestInputs.push_back(temp); temp.clear();

	  for (int j = 0; j < n_out; j++)
	  {
		  test >> lec;
		  temp.push_back(lec);
	  }

	  TestOutputs.push_back(temp); temp.clear();

  }

  return true;
}
 






  
  
