#include "CController.h"

#ifdef _OPENMP
#include <omp.h>
#endif // DEBUG

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
			size_batch = 100;
			size_prev = CParams::TrainingInputs.size() - size_batch;
		}
		else if((m_vecBestBrains[0].Fitness() >= CParams::soglia_prestazioni) || (m_iGenerations == 0))//condizione che fa cambiare il problema
		{
			size_prev = 10;
			/*bisogna gestire la fine file, eventualmente ripartire da capo*/
			if (CParams::ModAddestramento == MODO_BATCH_INCREMENTALE)
			{
				conteggio_step = 0;

				//versione combo standard col batch che aumenta
				if ((size_batch + 2*size_prev) <= CParams::TrainingInputs.size())
					size_batch += size_prev;
				else if ((size_batch + size_prev) < CParams::TrainingInputs.size())
					size_batch = CParams::TrainingInputs.size() - size_prev;
				else
				{
					size_batch = size_prev;
					CParams::soglia_prestazioni += 5.f;
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

			ResetBestFitness();
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
				m_vecBestBrains[i].SetBrain(pBestBrains[i]);

				m_vecBestBrains[i].Update(CParams::TrainingInputs);
				m_vecBestBrains[i].EndOfRunCalculations_locale(CParams::TrainingInputs);


				//stampa le attivazioni dei nodi output per ogni array di training 
				string name_file_output = "Member_" + itos(i) + "\\Trainoutput_" + itos(m_iGenerations - 1) + ".txt";
				if (i == 0)
				{
					m_vecBestBrains[i].Write_output(name_file_output, TRAIN);

					out0 << m_vecBestBrains[i].Fitness() <<  endl;
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
		case MODO_GLOBALE:
		{
#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

			for (int i = 0; i<m_vecBestBrains.size(); ++i)
			{
				m_vecBestBrains[i].SetBrain(pBestBrains[i]);

				m_vecBestBrains[i].Update(CParams::TrainingInputs);
				m_vecBestBrains[i].EndOfRunCalculations_globale(CParams::TrainingInputs);


				//stampa le attivazioni dei nodi output per ogni array di training 
				string name_file_output = "Member_" + itos(i) + "\\Trainoutput_" + itos(m_iGenerations - 1) + ".txt";
				if (i == 0)
				{
					m_vecBestBrains[i].Write_output(name_file_output, TRAIN);

					out0 << m_vecBestBrains[i].Fitness() << endl;
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
					m_vecBestBrains[i].Write_output(name_file_output, TRAIN);

					out0 << m_vecBestBrains[i].Fitness() << "\t" << fit_perc << "\t" 
						<< m_vecBestBrains[i].m_dFitness_batch << "\t" << m_vecBestBrains[i].m_dFitness_batch_perc << endl;
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

	s_temp = "Generation:              " + itos(m_iGenerations);
	cout << s_temp << endl;

	s_temp = "Num Species:          " + itos(m_pPop->NumSpecies());
	cout << s_temp << endl << endl;


	s_temp = "\t\tTraining";
	cout << s_temp << endl;



	if (m_iGenerations > 0)
	{
		s_temp = "Fitness 1: " + ftos(m_vecBestBrains[0].Fitness());
		cout << s_temp << endl;

		s_temp = "Fitness 2: " + ftos(m_vecBestBrains[1].Fitness());
		cout << s_temp << endl;

		s_temp = "Fitness 3: " + ftos(m_vecBestBrains[2].Fitness());
		cout << s_temp << endl;

		s_temp = "Fitness 4: " + ftos(m_vecBestBrains[3].Fitness());
		cout << s_temp << endl;
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