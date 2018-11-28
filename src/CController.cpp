#include "CController.h"


#ifdef VIEWER

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

	size_batch = 0;

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
#else
CController::CController()
{
	m_NumBrains = CParams::iPopSize;

	m_iGenerations = 0;

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

	size_batch = size_prev = 0;

	//SetSizeBatch(1);

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

	m_dBestFitnessPerc_ever = 0.f;
}

CController::CController(const CGenome &ancestor, int generation, int max_gennoimpro, restart restart_mode)
{
	m_NumBrains = CParams::iPopSize;

	m_iGenerations = generation;

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

	size_batch = size_prev = 0;

	//creazione della popolazione a cui applicare l'algoritmo genetico, associata ai Brains
	if(restart_mode == ONE_SPECIES)
		m_pPop = new Cga(CParams::iPopSize, generation, ancestor, ancestor.Fitness(), 0.f);
	else if(restart_mode == CHANGE)
		m_pPop = new Cga(CParams::iPopSize, generation, ancestor, 0.f, 0.f);

	/*questo mantiene in memoria gen no impro*/
	m_pPop->Set_MaxGenAllowedNoImprovement(max_gennoimpro);

	//creazione dei fenotipi associati ai genotipi della popolazione
	vector<CNeuralNet*> pBrains = m_pPop->CreatePhenotypes();

	//assegnazione dei fenotipi ai Brains
	for (int i = 0; i < m_NumBrains; i++)
	{
		m_vecBrains[i].SetBrain(pBrains[i]);
	}

	CParams::dCompatibilityThreshold = CParams::dCompatibilityThreshold_initial;

	m_dBestFitnessPerc_ever = 0.f;
}


#endif // VIEWER


//--------------------------------------destructor-------------------------------------
//
//--------------------------------------------------------------------------------------
CController::~CController()
{
	if (m_pPop)
	{
		delete m_pPop;
	}

#ifdef VIEWER
	DeleteObject(m_BluePen);
	DeleteObject(m_RedPen);
	DeleteObject(m_GreenPen);
	DeleteObject(m_OldPen);
	DeleteObject(m_GreyPenDotted);
	DeleteObject(m_RedPenDotted);
	DeleteObject(m_BlueBrush);
	DeleteObject(m_RedBrush);
#endif
}


//-------------------------------------Update---------------------------------------
//
//	L'intera simulazione è controllata da qui. Ogni chiamata esegue una generazione.
//
//--------------------------------------------------------------------------------------
bool CController::Update(ofstream &out0/*, ofstream &out1, ofstream &out2, ofstream &out3*/)
{
	m_pPop->Destroym_vecBestPhenotypes();


	/*inrerire con un controllo sulla fitness percentuale ad esempio quando è sull'80%
	cambiare gli input e di conseguenza la size del batch (size_batch += 0.8*size_input_temp)*/

	if (CParams::ModAddestramento > MODO_GLOBALE)//ci si entra solo se siamo in batch
	{
		if (CParams::ModAddestramento == MODO_BATCH)
		{
			//size_batch = CParams::TrainingInputs.size() * 0.9f;
			//size_batch = 10;
			size_prev = CParams::TrainingInputs.size() - size_batch;
		}
		else if((m_vecBestBrains[0].m_dFitness_perc >= CParams::soglia_prestazioni) /*(m_vecBestBrains[0].Fitness() >= CParams::soglia_prestazioni)*/ || (m_iGenerations == 0))//condizione che fa cambiare il problema
		{
			//lo tolgo nella versione che incrementa
			//size_prev = 10;
			/*bisogna gestire la fine file, eventualmente ripartire da capo*/
			if (CParams::ModAddestramento == MODO_BATCH_INCREMENTALE)
			{
				conteggio_step = 0;

				//versione combo standard col batch che aumenta
				/*if ((size_batch + 2*size_prev) <= CParams::TrainingInputs.size())
					size_batch += size_prev;
				else if ((size_batch + size_prev) < CParams::TrainingInputs.size())
					size_batch = CParams::TrainingInputs.size() - size_prev;
				else
				{
					size_batch = size_prev;
					CParams::soglia_prestazioni += 5.f;
				}*/


				size_batch = 10;
				//versione combo standard col batch che aumenta
				if ((2*size_batch + size_prev) <= CParams::TrainingInputs.size())
					size_prev += size_batch;
				else if ((size_batch + size_prev) < CParams::TrainingInputs.size())
					size_prev = CParams::TrainingInputs.size() - size_batch;
				else
				{
					size_batch = size_prev;
					CParams::soglia_prestazioni += 5.f;

					//lo metto nella versione che incrementa
					ResetBestFitness();
				}



			}
			else	//versione batch mobile
			{
				size_batch = 3 * size_prev;

				if ((conteggio_step + 3)*size_prev + size_batch /*+ size_prev*/ <= CParams::TrainingInputs.size())
					;
				else if ((conteggio_step + 2)*size_prev + size_batch /*+ size_prev*/ < CParams::TrainingInputs.size())
					size_batch = CParams::TrainingInputs.size() /*- size_prev*/ - (conteggio_step+2) * size_prev;
				else
				{
					conteggio_step = -1;

					CParams::soglia_prestazioni += 5.f;
				}
				//(conteggio_step+1)*size_prev + size_batch + size_prev;

				conteggio_step++;

			}


			TrainingInputs_piccoli.clear();
			GeneraData_set(TrainingInputs_piccoli);

			//lo tolgo nella versione che incrementa
			//ResetBestFitness();
		}
	}


	switch (CParams::ModAddestramento)
	{
	case MODO_LOCALE:
	{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

		for (int i = 0; i < m_NumBrains; ++i)
		{
			m_vecBrains[i].Update(CParams::TrainingInputs);
			m_vecBrains[i].EndOfRunCalculations_locale(CParams::TrainingInputs);
		}

		break;
	}
	case MODO_GLOBALE:
	{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

		for (int i = 0; i < m_NumBrains; ++i)
		{
			m_vecBrains[i].Update(CParams::TrainingInputs);
			m_vecBrains[i].EndOfRunCalculations_globale(CParams::TrainingInputs);
		}

		break;
	}
	case MODO_BATCH:
	{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

		for (int i = 0; i < m_NumBrains; ++i)
		{
			m_vecBrains[i].index_act = 0;
			m_vecBrains[i].Update_Batch(CParams::TrainingInputs, size_batch);

			m_vecBrains[i].EndOfRunCalculations_Batch(CParams::TrainingInputs, size_batch);
		}

		break;
	}
	case MODO_BATCH_INCREMENTALE:
	{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

		for (int i = 0; i < m_NumBrains; ++i)
		{
			m_vecBrains[i].index_act = 0;
			m_vecBrains[i].Update_Batch(TrainingInputs_piccoli, size_batch);

			m_vecBrains[i].EndOfRunCalculations_Batch(TrainingInputs_piccoli, size_batch);
		}

		break;
	}
	case MODO_BATCH_MOBILE:
	{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

		for (int i = 0; i < m_NumBrains; ++i)
		{
			m_vecBrains[i].index_act = conteggio_step*size_prev;
			m_vecBrains[i].Update_Batch(TrainingInputs_piccoli, size_batch);

			m_vecBrains[i].EndOfRunCalculations_Batch(TrainingInputs_piccoli, size_batch);
		}

		break;
	}
	default:
		break;
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


		switch (CParams::ModAddestramento)
		{
		case MODO_LOCALE:
		{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

			for (int i = 0; i<m_vecBestBrains.size(); ++i)
			{
				float fit_perc;//prestazioni in termini di Rate%

				m_vecBestBrains[i].SetBrain(pBestBrains[i]);

				m_vecBestBrains[i].Update(CParams::TrainingInputs);
				fit_perc = m_vecBestBrains[i].EndOfRunCalculations_locale(CParams::TrainingInputs);


				//stampa le attivazioni dei nodi output per ogni array di training 
				string name_file_output = "Member_" + itos(i) + "\\Trainoutput_" + itos(m_iGenerations - 1) + ".txt";
				if (i == 0)
				{
					if (m_pPop->is_improved)
						m_vecBestBrains[i].Write_output(name_file_output, TRAIN);

					out0 << m_vecBestBrains[i].Fitness() << "\t" << fit_perc << "\t" << endl;
				}
				
				/*
				//---------- TEST -------------------
				m_vecBestBrains[i].Reset();

				m_vecBestBrains[i].Update_test(CParams::TestInputs);

				if (i == 0)
				{
					string name_file_output = "Member_0\\Testoutput_" + itos(m_iGenerations - 1) + ".txt";

					m_vecBestBrains[i].Write_output(name_file_output, TEST);
				}
				*/

				m_vecBestBrains[i].Reset();
			}

			if (m_pPop->is_improved)
			{
				Testing_Networks();
				m_pPop->Set_is_improved(false);
			}

			break;
		}
		case MODO_GLOBALE:
		{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

			for (int i = 0; i<m_vecBestBrains.size(); ++i)
			{
				float fit_perc;//prestazioni in termini di Rate%

				m_vecBestBrains[i].SetBrain(pBestBrains[i]);

				m_vecBestBrains[i].Update(CParams::TrainingInputs);
				fit_perc = m_vecBestBrains[i].EndOfRunCalculations_globale(CParams::TrainingInputs);


				//stampa le attivazioni dei nodi output per ogni array di training 
				string name_file_output = "Member_" + itos(i) + "\\Trainoutput_" + itos(m_iGenerations - 1) + ".txt";
				if (i == 0)
				{
					if (m_pPop->is_improved)
						m_vecBestBrains[i].Write_output(name_file_output, TRAIN);

					out0 << m_vecBestBrains[i].Fitness() <<"\t"<< fit_perc << "\t" << endl;
				}


				m_vecBestBrains[i].Reset();

				/*
				//---------- TEST -------------------
				m_vecBestBrains[i].Reset();

				m_vecBestBrains[i].Update_test(CParams::TestInputs);

				if (i == 0)
				{
					string name_file_output = "Member_0\\Testoutput_" + itos(m_iGenerations - 1) + ".txt";

					m_vecBestBrains[i].Write_output(name_file_output, TEST);
				}
				*/
				
				//m_vecBestBrains[i].Reset();
			}

			if (m_pPop->is_improved)
			{
				Testing_Networks();
				m_pPop->Set_is_improved(false);
			}
			break;
		}
		case MODO_BATCH:
		{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

			for (int i = 0; i<m_vecBestBrains.size(); ++i)
			{
				float fit_perc;//prestazioni in termini di Rate%

				m_vecBestBrains[i].SetBrain(pBestBrains[i]);

				m_vecBestBrains[i].index_act = 0;
				m_vecBestBrains[i].Update_Batch(CParams::TrainingInputs, size_batch);

				fit_perc = m_vecBestBrains[i].EndOfRunCalculations_Batch(CParams::TrainingInputs, size_batch);

				

				//stampa le attivazioni dei nodi output per ogni array di training 
				string name_file_output = "Member_" + itos(i) + "\\Trainoutput_" + itos(m_iGenerations - 1) + ".txt";
				if (i == 0)
				{
					m_dBestFitnessPerc = fit_perc;



					if (m_pPop->is_improved)
						m_vecBestBrains[i].Write_output(name_file_output, TRAIN);

					out0 << m_vecBestBrains[i].Fitness() << "\t" << fit_perc << "\t" 
						<< m_vecBestBrains[i].m_dFitness_batch << "\t" << m_vecBestBrains[i].m_dFitness_batch_perc << "\t"
						<< size_batch << endl;
				}

				/*
				//---------- TEST -------------------
				m_vecBestBrains[i].Reset();

				m_vecBestBrains[i].Update_test(CParams::TestInputs);

				if (i == 0)
				{
					string name_file_output = "Member_0\\Testoutput_" + itos(m_iGenerations - 1) + ".txt";

					m_vecBestBrains[i].Write_output(name_file_output, TEST);
				}
				*/
				m_vecBestBrains[i].Reset();

			}

			if (m_pPop->is_improved)
			{
				Testing_Networks();
				m_pPop->Set_is_improved(false);
			}

			break;
		}
		case MODO_BATCH_INCREMENTALE:
		{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

			for (int i = 0; i<m_vecBestBrains.size(); ++i)
			{
				float fit_perc;//prestazioni in termini di Rate%

				m_vecBestBrains[i].SetBrain(pBestBrains[i]);

				m_vecBestBrains[i].index_act = 0;
				m_vecBestBrains[i].Update_Batch(TrainingInputs_piccoli, size_batch);

				fit_perc = m_vecBestBrains[i].EndOfRunCalculations_Batch(TrainingInputs_piccoli, size_batch);


				//stampa le attivazioni dei nodi output per ogni array di training 
				string name_file_output = "Member_" + itos(i) + "\\Trainoutput_" + itos(m_iGenerations - 1) + ".txt";
				if (i == 0)
				{
					m_vecBestBrains[i].Write_output(name_file_output, TRAIN);

					out0 << m_vecBestBrains[i].Fitness() << "\t" << fit_perc << "\t"
						<< m_vecBestBrains[i].m_dFitness_batch << "\t" << m_vecBestBrains[i].m_dFitness_batch_perc
						<< "\t" << CParams::soglia_prestazioni << endl;
				}


				//---------- TEST -------------------
				m_vecBestBrains[i].Reset();

				m_vecBestBrains[i].Update_test(CParams::TestInputs);

				if (i == 0)
				{
					string name_file_output = "Member_0\\Testoutput_" + itos(m_iGenerations - 1) + ".txt";

					m_vecBestBrains[i].Write_output(name_file_output, TEST);
				}

				m_vecBestBrains[i].Reset();
			}

			break;
		}
		case MODO_BATCH_MOBILE:
		{
/*#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG
*/
			for (int i = 0; i<m_vecBestBrains.size(); ++i)
			{
				float fit_perc;//prestazioni in termini di Rate%

				m_vecBestBrains[i].SetBrain(pBestBrains[i]);

				m_vecBestBrains[i].index_act = conteggio_step*size_prev;;
				m_vecBestBrains[i].Update_Batch(TrainingInputs_piccoli, size_batch);

				fit_perc = m_vecBestBrains[i].EndOfRunCalculations_Batch(TrainingInputs_piccoli, size_batch);


				//stampa le attivazioni dei nodi output per ogni array di training 
				string name_file_output = "Member_" + itos(i) + "\\Trainoutput_" + itos(m_iGenerations - 1) + ".txt";
				if (i == 0)
				{
					m_vecBestBrains[i].Write_output(name_file_output, TRAIN);

					out0 << m_vecBestBrains[i].Fitness() << "\t" << fit_perc << "\t"
						<< m_vecBestBrains[i].m_dFitness_batch << "\t" << m_vecBestBrains[i].m_dFitness_batch_perc
						<< "\t" << CParams::soglia_prestazioni << endl;
				}


				//---------- TEST -------------------
				m_vecBestBrains[i].Reset();

				m_vecBestBrains[i].Update_test(CParams::TestInputs);

				if (i == 0)
				{
					string name_file_output = "Member_0\\Testoutput_" + itos(m_iGenerations - 1) + ".txt";

					m_vecBestBrains[i].Write_output(name_file_output, TEST);
				}

				m_vecBestBrains[i].Reset();
			}

			break;
		}
		default:
			break;
		}


#ifdef VIEWER
		//questo invoca le callback in WM_PAINT, in modo da ridisegnare il contenuto delle finestre
		InvalidateRect(m_hwndInfo, NULL, TRUE);
		UpdateWindow(m_hwndInfo);

		InvalidateRect(m_hwndMain, NULL, TRUE);
		UpdateWindow(m_hwndMain);

#else
		Render();
#endif //VIEWER
		

	return true;
}


#ifdef VIEWER
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
#else
void	CController::Render()
{
	PlotStats();

	//disegna la barra delle specie
	m_pPop->RenderSpeciesInfo();
}
void	CController::PlotStats() 
{

	string s_temp = "Best Fitness so far: " + ftos(m_pPop->BestEverFitness());
	cout << s_temp << endl;

	s_temp = "Generation\t   : " + itos(m_iGenerations);
	cout << s_temp << endl;

	s_temp = "Num Species\t   : " + itos(m_pPop->NumSpecies());
	cout << s_temp << endl << endl;


	s_temp = "\t\tTraining";
	cout << s_temp << endl;



	if (m_iGenerations > 0)
	{
		s_temp = "Fitness 1:      " + ftos(m_vecBestBrains[0].Fitness());
		cout << s_temp << endl;

		s_temp = "Fitness 2:      " + ftos(m_vecBestBrains[1].Fitness());
		cout << s_temp << endl;

		s_temp = "Fitness 3:      " + ftos(m_vecBestBrains[2].Fitness());
		cout << s_temp << endl;

		s_temp = "Fitness 4:      " + ftos(m_vecBestBrains[3].Fitness());
		cout << s_temp << endl << endl;
	}

}
#endif // VIEWER

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




bool CController::GeneraData_set(vector<char> &input)
{
	bool added = true;

	int dim_scelta = size_batch + size_prev, pos;

	if (dim_scelta > CParams::TrainingInputs.size())
		dim_scelta = CParams::TrainingInputs.size();

	/*while (indici.size() < dim_scelta)
	{
		pos = RandInt(0, CParams::TrainingInputs.size() - 1);

		for (int i = 0; i < indici.size(); i++)
		{
			if (pos == indici[i])
			{
				added = false;

				goto aggiungi;
			}
		}

	aggiungi:
		if (added)
		{
			indici.push_back(pos);
		}

		added = true;
	}
	*/


	input.resize(dim_scelta);

	//versione batch che aumenta
	/*for (int i = 0; i < dim_scelta; i++)
	{
		input[i] = CParams::TrainingInputs[i];
	}*/

	//versione batch mobile, provvisoria bisogna mettere un controllo sulla fine del file di input 
	//e nel caso resettare il contegio_step e ridefinire size_batch e size_prev
	/*for (int i = 0; i < dim_scelta; i++)
		input[i] = CParams::TrainingInputs[i + conteggio_step * dim_scelta];*/
	

	//versione batch mobile sovrapposta, provvisoria bisogna mettere un controllo sulla fine del file di input 
	//e nel caso resettare il contegio_step e ridefinire size_batch e size_prev
	for (int i = 0; i < dim_scelta; i++)
		input[i] = CParams::TrainingInputs[i + conteggio_step * size_prev];



	/*
	for (int i = 0; i < indici.size(); i++)
	{
		for (int j = 0; j < CParams::TrainingInputs[indici[i]].size(); j++)
		{
			temp_input.push_back(CParams::TrainingInputs[indici[i]][j]);
		}

		for (int j = 0; j < CParams::TrainingOutputs[indici[i]].size(); j++)
		{
			temp_output.push_back(CParams::TrainingOutputs[indici[i]][j]);
		}

		input.push_back(temp_input); output.push_back(temp_output);

		temp_input.clear(); temp_output.clear();
	}*/


	return true;
}






void CController::ResetBestFitness()
{
	m_dBestFitness = 0; m_pPop->SetBestEverFitness(0);
	for (int i = 0; i < m_pPop->NumSpecies(); i++)
	{
		m_pPop->SetFitnessSpecies(0);
	}
}


void CController::Testing_Networks()
{
	bool is_Done = false;
	int count;

	CBrain tested_brain;
	CGenome tested_genome;

	vector<float> testing_temperature(5);// [] = { 1.f, 0.6f, 0.2f, };
	testing_temperature[0] = 1.f;
	testing_temperature[1] = 0.6f;
	testing_temperature[2] = 0.2f;
	testing_temperature[3] = 0.1f;
	testing_temperature[4] = 0.05f;



	// creazione delle directory che conterranno i file riguardanti i migliori 4 individui
	char percorso[] = "Member_0\\";
	string cartella;

	for (int i = 0; i < 1/*4*/; i++)
	{
		cartella = "Test";

		crea_cartella(percorso, cartella);
	}

	string name_genome;

	int gen_scelta;

	while(!is_Done)
	{
		/*cout << "Digitare -1 per uscire,\nO inserire la generazione dell'individuo da testare [0, " + itos(m_iGenerations-1) + "]: ";
		cin >> gen_scelta;
		
		if (gen_scelta == -1)
		{
			is_Done = true;
			break;
		}*/

		gen_scelta = m_iGenerations-1;

		//chiedere all'utente l'individuo scelto (numero di generazione)
		name_genome = "Member_0\\dbg_BestGenome_gen" + itos(gen_scelta) + ".txt";
		
		//generare il brains con associato il genoma letto da file
		tested_genome.CreateFromFile(name_genome);
		tested_brain.SetBrain(tested_genome.CreatePhenotype());

		/*cout << "inserire il numero di volte che si vuole testare sweeppando in temperatura: ";
		cin >> count; cout << endl << endl;
		*/

		count = 3;

		while (count--)
		{
			for (int i = 0; i < testing_temperature.size(); i++)
			{
				float temperature = testing_temperature[i];

				tested_brain.Update_test(CParams::TestInputs, temperature);
				tested_brain.Reset();

				string name_file_output = percorso + cartella + "\\Testoutput_" + itos(gen_scelta) + "_T" + ftos(temperature) + "_iter"
					+ itos(count) + ".txt";

				tested_brain.Write_output(name_file_output, TEST);

			}
		}

		tested_genome.DeletePhenotype();

		break;

//skip:

	}
	//generare il file di test associato variando la temperatura
}

float CController::Calcola_BestFitnessPerc()
{

	int size_input = CParams::TrainingInputs.size();
	
	
	//posso sfruttare l'uguaglianza fra vector == restituisce true solo se hanno stessa dimensione e stessi elementi
	//forse posso creare una mappa di vector di char che punta al char successivo,
	vector<char> input_batch;
	input_batch.resize(size_batch);


	list<pair<vector<char>, char>> predictions;
	map<vector<char>, char> predictions_checked;
	int i, count = 0;


	//le mappe non vanno bene perché posso avere solo chiavi uniche, forse si può usare la lista di pair<vector<char>, char>
	while (size_input - (size_batch + count))
	{
		for (i = 0; i < size_batch; i++)
		{
			input_batch[i] = CParams::TrainingInputs[count + i];
		}

		predictions.push_back(pair<vector<char>, char>(input_batch, CParams::TrainingInputs[count + i]));

		++count;
	}

	//ora ho tutti i batch e le previsioni voglio, per ogni vector distinto, contare la frazione del carattere in previsione che
	//si propone più spesso.
	list<pair<vector<char>, char>>::iterator index, index_inner;
	index = predictions.begin();

	map<char, int> occurrency;
	map<char, int>::iterator it_occ;

	int sum = 0;

	while (index != predictions.end())
	{
		if (predictions_checked.count(index->first) == 0)
		{
			//testare tutte le second che hanno queso first
			for (index_inner = index; index_inner != predictions.end(); index_inner++)
			{
				if (index->first == index_inner->first)//se il batch è uguale aggiungo il second ad occurrency
				{
					if (occurrency.count(index_inner->second) == 0)
						occurrency.insert(pair<char, int>(index_inner->second, 1));
					else
						occurrency[index_inner->second]++;
				}
			}

			it_occ = occurrency.begin();
			int max = 0;

			while (it_occ != occurrency.end())
			{
				if (it_occ->second > max)
					max = it_occ->second;

				++it_occ;
			}

			sum += max;
			occurrency.clear();//ripulisco per il prossimo batch distinto

			predictions_checked.insert(*index);
		}

		++index;
	}

	return 100.f*(sum / ((float)predictions.size()));
}