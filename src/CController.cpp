#include "CController.h"
#include <omp.h>

//---------------------------------------constructor---------------------
//
//  inizializza i Brains, i loro genomi e tutto quello che serve all'algoritmo genetico
//
//-----------------------------------------------------------------------
CController::CController(HWND hwndMain,
	int  cxClient,
	int  cyClient) : m_NumBrains(CParams::iPopSize),
	m_hwndMain(hwndMain),
	m_hwndInfo(NULL),
	m_iGenerations(0),
	m_cxClient(cxClient),
	m_cyClient(cyClient)

{
	m_vecBrains.resize(m_NumBrains);
	//creazione dei Brains
	for (int i = 0; i < m_NumBrains; ++i)
	{
		m_vecBrains[i] = (CBrain());
	}

	m_vecBestBrains.resize(CParams::iNumBestBrains);
	//creazione dell'array che conterrà i migliori Brains
	for (int i = 0; i < CParams::iNumBestBrains; ++i)
	{
		m_vecBestBrains[i] = (CBrain());
	}


	//creazione della popolazione a cui applicare l'algoritmo genetico, associata ai Brains
	m_pPop = new Cga(CParams::iPopSize,
		CParams::iNumInputs,
		CParams::iNumOutputs);

	//creazione dei fenotipi associati ai genotipi della popolazione
	vector<CNeuralNet*> pBrains = m_pPop->CreatePhenotypes();

	//assegnazione dei fenotipi ai Brains
	for (int i = 0; i < m_NumBrains; i++)
	{
		m_vecBrains[i].SetBrain(pBrains[i]);
	}

	//creazione degli oggetti grafici che si utilizzeranno
	m_BluePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	m_RedPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	m_GreenPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	m_GreyPenDotted = CreatePen(PS_DOT, 1, RGB(100, 100, 100));
	m_RedPenDotted = CreatePen(PS_DOT, 1, RGB(200, 0, 0));

	m_OldPen = NULL;


	m_BlueBrush = CreateSolidBrush(RGB(0, 0, 244));
	m_RedBrush = CreateSolidBrush(RGB(150, 0, 0));
}

//--------------------------------------destructor-------------------------------------
//
//--------------------------------------------------------------------------------------
CController::~CController()
{
	if (m_pPop)
	{
		delete m_pPop;
	}

	DeleteObject(m_BluePen);
	DeleteObject(m_RedPen);
	DeleteObject(m_GreenPen);
	DeleteObject(m_OldPen);
	DeleteObject(m_GreyPenDotted);
	DeleteObject(m_RedPenDotted);
	DeleteObject(m_BlueBrush);
	DeleteObject(m_RedBrush);
}


//-------------------------------------Update---------------------------------------
//
//	L'intera simulazione è controllata da qui. Ogni chiamata esegue una generazione.
//
//--------------------------------------------------------------------------------------
bool CController::Update(ofstream &out0, ofstream &out1, ofstream &out2, ofstream &out3)
{
	m_pPop->Destroym_vecBestPhenotypes();

#pragma omp parallel for

		for (int i = 0; i<m_NumBrains; ++i)
		{

			m_vecBrains[i].Update(CParams::TrainingInputs);

			//calcola le previsioni dei Brains nei confronti del Training Set
			/*if (!m_vecBrains[i].Update(CParams::TrainingInputs))
			{
				//error
				MessageBox(m_hwndMain, L"Wrong amount of NN inputs!", L"Error", MB_OK);

				return false;
			}*/

			//calcola la fitness dei Brains
			m_vecBrains[i].EndOfRunCalculations(CParams::TrainingInputs);
		}




		//incrementa il contatore delle generazioni
		++m_iGenerations;


		//Performa la creazione della nuova generazione di individui e ne restituisce i fenotipi.
		//questa funzione modifica internamente il vector dei genomi.
vector<CNeuralNet*> pBrains = m_pPop->Epoch(GetFitnessScores());

		m_dBestFitness = m_pPop->BestEverFitness();


		//associa il nuovi fenotipi ai Brains
		for (int i = 0; i<m_NumBrains; ++i)
		{
			m_vecBrains[i].SetBrain(pBrains[i]);

			m_vecBrains[i].Reset();
		}

		//Restituisce i migliori fenotipi della generazione precedente; prima della chiamata Epoch.
		vector<CNeuralNet*> pBestBrains = m_pPop->GetBestPhenotypesFromLastGeneration();
			
		


		/////////////// Calcolo delle prestazioni complete per i migliori individui ////////////////////////////

		//float fit_perc;//prestazioni in termini di Rate%

//#pragma omp parallel for

		for (int i = 0; i<m_vecBestBrains.size(); ++i)
		{
			m_vecBestBrains[i].SetBrain(pBestBrains[i]);

			m_vecBestBrains[i].Update(CParams::TrainingInputs);

			/*if (!m_vecBestBrains[i].Update(CParams::TrainingInputs))
			{
				//error
				MessageBox(m_hwndMain, L"Wrong amount of NN inputs!", L"Error", MB_OK);

				return false;
			}*/

			m_vecBestBrains[i].EndOfRunCalculations(CParams::TrainingInputs);
				

			////////////// STAMPA DELLE PRESTAZIONI NEL RELATIVO FILE ////////////////////////////


			//stampa le attivazioni dei nodi output per ogni array di training 
			string name_file_output = "Member_" + itos(i) + "\\Trainoutput_" + itos(m_iGenerations - 1) + ".txt";

			m_vecBestBrains[i].Write_output(name_file_output, TRAIN);

			if (i == 0)
				out0 << m_vecBestBrains[i].Fitness() << endl;
			else if(i == 1)
				out1 << m_vecBestBrains[i].Fitness() << endl;
			else if(i == 2)
				out2 << m_vecBestBrains[i].Fitness() << endl;
			else if(i == 3)
				out3 << m_vecBestBrains[i].Fitness() << endl;



			/*if (i == 0)
			{
				out0 << m_vecBestBrains[i].Fitness() << "\t" << fit_perc << "\t";

				for (int i_out = 0; i_out < m_vecBestBrains[i].mean_sqe.size(); i_out++)
				{
					out0 << m_vecBestBrains[i].mean_sqe[i_out] << "\t";
				}


				//stampa le attivazioni dei nodi output per ogni array di training 
				string name_file_output = "Member_0\\Trainoutput_" + itos(m_iGenerations - 1) + ".txt";
				
				m_vecBestBrains[i].Write_output(name_file_output,TRAIN);
			}
			else if (i == 1)
			{
				out1 << m_vecBestBrains[i].Fitness() << "\t" << fit_perc << "\t";
				
				for (int i_out = 0; i_out < m_vecBestBrains[i].mean_sqe.size(); i_out++)
				{
					out1 << m_vecBestBrains[i].mean_sqe[i_out] << "\t";
				}
			}
			else if (i == 2)
			{
				out2 << m_vecBestBrains[i].Fitness() << "\t" << fit_perc << "\t";

				for (int i_out = 0; i_out < m_vecBestBrains[i].mean_sqe.size(); i_out++)
				{
					out2 << m_vecBestBrains[i].mean_sqe[i_out] << "\t";
				}
			}
			else
			{
				out3 << m_vecBestBrains[i].Fitness() << "\t" << fit_perc << "\t";

				for (int i_out = 0; i_out < m_vecBestBrains[i].mean_sqe.size(); i_out++)
				{
					out3 << m_vecBestBrains[i].mean_sqe[i_out] << "\t";
				}

			}
			*/
			//ripulisce gli oggetti del brain poichè ora li applico al test set
			m_vecBestBrains[i].Reset();
			pBestBrains[i]->Reset_activation();
			

			m_vecBestBrains[i].Update_test();

			/*if (!m_vecBestBrains[i].Update())
			{
				//error
				MessageBox(m_hwndMain, L"Wrong amount of NN inputs!", L"Error", MB_OK);

				return false;
			}

			fit_perc = m_vecBestBrains[i].EndOfRunCalculations(CParams::TestOutputs, true);//gli dico che sono per il test
			*/


			if (i == 0)
			{
				/*out0 << m_vecBestBrains[i].Fitness_test() << "\t" << fit_perc;
				out0 << endl;
				*/

				string name_file_output = "Member_0\\Testoutput_" + itos(m_iGenerations - 1) + ".txt";

				m_vecBestBrains[i].Write_output(name_file_output, TEST);
			}
			/*else if (i == 1)
			{
				out1 << m_vecBestBrains[i].Fitness_test() << "\t" << fit_perc;
				out1 << endl;
			}
			else if (i == 2)
			{
				out2 << m_vecBestBrains[i].Fitness_test() << "\t" << fit_perc;
				out2 << endl;
			}
			else
			{
				out3 << m_vecBestBrains[i].Fitness_test() << "\t" << fit_perc;
				out3 << endl;

			}*/

			m_vecBestBrains[i].Reset();
		}

		
		//questo invoca le callback in WM_PAINT, in modo da ridisegnare il contenuto delle finestre
		InvalidateRect(m_hwndInfo, NULL, TRUE);
		UpdateWindow(m_hwndInfo);

		InvalidateRect(m_hwndMain, NULL, TRUE);
		UpdateWindow(m_hwndMain);

		

	return true;
}



//---------------------------------- RenderNetworks ----------------------
//
//  Disegna i fenotipi dei migliori 4 individui
//------------------------------------------------------------------------
void CController::RenderNetworks(HDC &surface)
{
	if (m_iGenerations < 1)
	{
		return;
	}

	//Dimensioni della finestra figlia
	RECT rect;
	GetClientRect(m_hwndInfo, &rect);

	int	cxInfo = rect.right;
	int	cyInfo = rect.bottom;


	//disegna i fenotipi. In ordine di fitness, da destra a sinistra e dall'alto al basso
	/*m_vecBestBrains[0].DrawNet(surface, 0, cxInfo / 2, cyInfo / 2, 0);
	m_vecBestBrains[1].DrawNet(surface, cxInfo / 2, cxInfo, cyInfo / 2, 0);
	m_vecBestBrains[2].DrawNet(surface, 0, cxInfo / 2, cyInfo, cyInfo / 2);
	m_vecBestBrains[3].DrawNet(surface, cxInfo / 2, cxInfo, cyInfo, cyInfo / 2);*/

	m_vecBestBrains[0].DrawNet(surface, 0, cxInfo, cyInfo, 0);
	

}

//------------------------------------Render()--------------------------------------
//
//Disegna il contenuto informativo generale nella finestra principale
//----------------------------------------------------------------------------------
void CController::Render(HDC &surface)
{
	
	PlotStats(surface);

	RECT sr;
	sr.top = m_cyClient - 50;
	sr.bottom = m_cyClient;
	sr.left = 0;
	sr.right = m_cxClient;

	//disegna la barra delle specie
	m_pPop->RenderSpeciesInfo(surface, sr);
}


//--------------------------PlotStats-------------------------------------
//
//Data una surface, vi stampa le informazioni generali della simulazione
//------------------------------------------------------------------------
void CController::PlotStats(HDC surface)const
{
	string s_temp = "Generation:              " + itos(m_iGenerations);
	wstring s = s2ws(s_temp);

	TextOut(surface, 5, 25, s.c_str(), s.size());

	s_temp = "Num Species:          " + itos(m_pPop->NumSpecies());
	s = s2ws(s_temp);
	TextOut(surface, 5, 45, s.c_str(), s.size());

	s_temp = "Best Fitness so far: " + ftos(m_pPop->BestEverFitness());
	s = s2ws(s_temp);
	TextOut(surface, 5, 5, s.c_str(), s.size());

	s_temp = "                 Training          Test";
	s = s2ws(s_temp);
	TextOut(surface, 5, 85, s.c_str(), s.size());


	if(m_iGenerations > 0)
	{
		s_temp = "Fitness 1: " + ftos(m_vecBestBrains[0].Fitness()) + "        " + ftos(m_vecBestBrains[0].Fitness_test());
		s = s2ws(s_temp);
		TextOut(surface, 5, 105, s.c_str(), s.size());

		s_temp = "Fitness 2: " + ftos(m_vecBestBrains[1].Fitness()) + "        " + ftos(m_vecBestBrains[1].Fitness_test());
		s = s2ws(s_temp);
		TextOut(surface, 5, 125, s.c_str(), s.size());

		s_temp = "Fitness 3: " + ftos(m_vecBestBrains[2].Fitness()) + "        " + ftos(m_vecBestBrains[2].Fitness_test());
		s = s2ws(s_temp);
		TextOut(surface, 5, 145, s.c_str(), s.size());

		s_temp = "Fitness 4: " + ftos(m_vecBestBrains[3].Fitness()) + "        " + ftos(m_vecBestBrains[3].Fitness_test());
		s = s2ws(s_temp);
		TextOut(surface, 5, 165, s.c_str(), s.size());
	}
}


//------------------------------- GetFitnessScores -----------------------
//
//  ritorna un std::vector contenente le fitness dei Brains
//------------------------------------------------------------------------
vector<float> CController::GetFitnessScores()const
{
	vector<float> scores; scores.resize(m_vecBrains.size());

	for (int i = 0; i<m_vecBrains.size(); ++i)
	{
		scores[i] = (m_vecBrains[i].Fitness());
	}
	return scores;
}