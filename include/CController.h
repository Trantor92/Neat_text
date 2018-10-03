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
#include <windows.h>



#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\utils.h"
#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\CBrain.h"
#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\CParams.h"
#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\Cga.h"


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
	double              m_dBestFitness;

	//contatore delle generazioni
	int					m_iGenerations;


	

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

public:

	CController(HWND hwndMain, int cxClient, int cyClient);

	~CController();
	
	//stampa le informazioni generali della simulazione
	void		Render(HDC &surface);

	//disegna i fenotipi dei migliori Brains 
	void		RenderNetworks(HDC &surface);

	//esegue una generazione
	bool		Update(ofstream &out0, ofstream &out1, ofstream &out2, ofstream &out3);



	//-------------------------------------accessor methods

	void            PassInfoHandle(HWND hnd) { m_hwndInfo = hnd; }

	vector<double>  GetFitnessScores()const;

};

