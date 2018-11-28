#pragma once

//------------------------------------------------------------------------
//
//	Name: CController.h
//
//  Desc: Classe per gestire il flusso della simulazione, interfacciando
//        il NEAT con la definizione degli individui.
//       
//
//------------------------------------------------------------------------

#include <vector>
#include <sstream>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif // DEBUG


#include "utils.h"
#include "CBrain.h"
#include "CParams.h"
#include "Cga.h"

#ifdef VIEWER
#include <windows.h>
#endif // VIEWER


using namespace std;



class CController
{
private:

	//racchiude tutta la popolazione di genomi
	Cga					*m_pPop;

	//array di Brains (i classificatori)
	vector<CBrain>		 m_vecBrains;

	//array dei migliori Brains
	vector<CBrain>		 m_vecBestBrains;

	//numero di individui totale
	int					m_NumBrains;


	//miglior fitness di sempre
	float              m_dBestFitness;

	//contatore delle generazioni
	int					m_iGenerations;


#ifdef VIEWER
	//pens utilizzate in fase di disegno grafico
	HPEN				m_RedPen;
	HPEN				m_BluePen;
	HPEN				m_GreenPen;
	HPEN				m_GreyPenDotted;
	HPEN				m_RedPenDotted;
	HPEN				m_OldPen;

	HBRUSH				m_RedBrush;
	HBRUSH			    m_BlueBrush;


	//copia locale del handle alla finestra principale
	HWND				m_hwndMain;

	//copia locale del handle alla finestra figlia
	HWND				m_hwndInfo;

	//copie locali delle dimensioni della finestra principale 
	int         m_cxClient, m_cyClient;

	//stampa le informazioni generali della simulazione
	//see CController Render()
	void   PlotStats(HDC surface)const;
#else
	void PlotStats();
#endif // VIEWER


public:


#ifdef VIEWER
	CController(HWND hwndMain, int cxClient, int cyClient);

	
	//stampa le informazioni generali della simulazione
	void		Render(HDC &surface);

	//disegna i fenotipi dei migliori Brains 
	void		RenderNetworks(HDC &surface);
#else
	CController();

	CController(const CGenome &ancestor, int generation, int max_gennoimpro, restart restart_mode);

	void		Render();
#endif // VIEWER


	~CController();


	//esegue una generazione
	bool		Update(ofstream &out0/*, ofstream &out1, ofstream &out2, ofstream &out3*/);

	void Testing_Networks();

	vector<char> TrainingInputs_piccoli;

	bool GeneraData_set(vector<char> &input);
	void ResetBestFitness();

	int size_batch, size_prev;
	int minRec;
	int conteggio_step = -1;//serve per implementare la batch mobile
	float m_dBestFitnessPerc, m_dBestFitnessPerc_ever;

	//-------------------------------------accessor methods

#ifdef VIEWER
	void            PassInfoHandle(HWND hnd) { m_hwndInfo = hnd; }
#endif // VIEWER

	int GetGeneration() const { return m_iGenerations; }
	Cga* GetPop() const { return m_pPop; }
	vector<float>  GetFitnessScores()const;

	void SetSizeBatch(int size_B) { size_batch = size_B; }
	int GetSizeBatch()const { return size_batch; }

	void Set_minRec(int new_minRec) {
		minRec = new_minRec; 

		for (int i = 0; i < m_NumBrains; ++i)
			m_vecBrains[i].minRec = minRec;
		
		for (int i = 0; i < CParams::iNumBestBrains; ++i)
			m_vecBestBrains[i].minRec = minRec;
		
	}

	float Calcola_BestFitnessPerc();

	int MaxGenAllowedNoImprovement()const { return m_pPop->MaxGenAllowedNoImprovement; }
};

