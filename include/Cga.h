//#ifndef CGA_H
//#define	CGA_H

#pragma once

//------------------------------------------------------------------------
//
//	Name: Cga.h
//
//  Desc: classe che implementa l'algoritmo genetico evolutivo NEAT.
//       
//------------------------------------------------------------------------
#include <vector>
#include <fstream>


#include "phenotype.h"
#include "genotype.h"
#include "CSpecies.h"
#include "CParams.h"

#ifdef VIEWER
#include <windows.h>
#endif // VIEWER

using namespace std;


//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
class Cga
{

private:
	
	//popolazione complessiva di genomi nella generazione corrente
	vector<CGenome>   m_vecGenomes;

	//tiene traccia dei migliori individui, prima che avvenga la riproduzione
	//e quindi la popolazinìone si aggiorni
	vector<CGenome>   m_vecBestGenomes;

	//vector di tutte le specie presenti
	vector<CSpecies>  m_vecSpecies;

	//puntatore al gestore delle innovazioni che hanno luogo
	CInnovation*      m_pInnovation;


	//variabili utili
	int               m_iGeneration;//generazione corrente

	int               m_iNextGenomeID;//numero identificativo da associare al prossimo genoma

	int               m_iNextSpeciesID;//numero identificativo da associare alla prossima specie

	int               m_iPopSize;//numero di individui nella popolazione


	//variabili che servono nel calcolo del numero di figli associare alle specie
	float            m_dTotFitAdj,
					  m_dAvFitAdj;


	//fitness migliore mai trovata fino ad ora
	float            m_dBestEverFitness;


	

	//questa funzione resetta alcuni valori necessari alla generazione successiva, come il disassociare
	//i vecchi fenotipi.
	//svuota i vector dei membri delle specie poichè la popolazione è cambiata riproducendosi, le info
	//sui leader rimangono in modo tale che si possa speciare.
	//Nel caso in cui la specie non migliori, questa viene eliminata isieme a tutti i genomi membri dalla
	//popolazione.
	//Restituisce un vector contenente la fitness dei genomi sopravvissuti.
	vector<float>		ResetAndKill(vector<float> FitnessScores);

	//speciazione preliminare, serve nella funzione precedente per uccidere in modo efficace
	//le specie che non migliorano
	void				Speciate();


	//fornisce la posizione in m_vecGenomes a partire dall'ID, del genoma
	int					GetMemberPos(int ID);

	//classifica ogni genoma in una specie in accordo con la funzione di compatibilità.
	//La funzione quindi modifica la fitness degli individui attraverso la condivisione nella specie
	//e quindi determina quanti figli ogni individuo deve avere (questo valore serve a determinare
	//quanti figli sono concessi alla specie).
	void				SpeciateAndCalculateSpawnLevels();

	//calcola la fitness modificata degli individui di una specie.
	void				AdjustSpeciesFitnesses();

	//Questa funzione aggiusta la soglia di compatibilità (nel caso in cui sia
	//specificato un numero massimo di specie) nel tentivo di mantenere il numero
	//di specie presenti entro un certo valore
	void				AdjustCompatibilityThreshold();


	//implementa il crossover dell'algoritmo NEAT
	CGenome				Crossover(CGenome& mum, CGenome& dad);
	
	//utilizzata in Crossover. Controlla se un nodo è presente nel genoma
	//e nel caso lo aggiunge
	void				AddNeuronID(int nodeID, vector<int> &vec);

	//vengono estratti alcuni individui migliori nel caso in cui gli arrotondamenti
	//nel calcolo del numero di figli concessi alle specie, comportino la generazione
	//di un numero di individui inferiore a m_iPopSize
	CGenome				TournamentSelection(const int NumComparisons);


	//ordina i genomi in m_vecGenomes per fitness decrescente di fitness.
	//tiene traccia dei migliori individui sui quali appunto ricade l'attenzione m_vecBestGenomes 
	void				SortAndRecord();

	
public:

	//------------- constructor -----------------------------------
	//crea una popolazione iniziale avente topologia minima con il numero
	//specificato di nodi di input e di output.
	Cga(int   size,
		int   inputs,
		int   outputs);

	~Cga();

	int MaxGenAllowedNoImprovement;
	int Calcola_MaxGenerationNoImprovement();

	//funzione che esegue una generazione dell'algoritmo genetico
	vector<CNeuralNet*>	 Epoch(const vector<float> &FitnessScores);

	//itera sui genomi che compongono la popolazione e ne crea i finotipi
	vector<CNeuralNet*>  CreatePhenotypes();
  
	//tiene traccia dei migliori individui della generazione prima che avvenga la riproduzione.
	void                 StoreBestGenomes();

	//ritorna i fenotipi dei migliori individui in m_vecBestGenomes
	vector<CNeuralNet*>  GetBestPhenotypesFromLastGeneration();

#ifdef VIEWER
	//Disegna le statistiche relative alla speciazione ed in particolare quelle
	//renlative alla specie migliore.
	void                 RenderSpeciesInfo(HDC &surface, RECT db);
#else
	void                 RenderSpeciesInfo();
#endif // VIEWER

	//questa funzione scrive su file il genoma dato l'indice che questo ha nel
	//vector m_vecGenomes
	bool                 WriteGenome(string szFileName, const int idxGenome);

	//scrive le specie in un file
	bool                 SpeciesDump(string szFileName);

	void Destroym_vecBestPhenotypes() {
		for (int i = 0; i < m_vecBestGenomes.size(); ++i)
			m_vecBestGenomes[i].DeletePhenotype();//DestroyPhenotype();
	}

	bool is_improved = false;
	int count_improving = 0;
  
	//-------------------- accessor methods ----------------------------------------
	int					Generation() { return m_iGeneration; }

	int					NumSpecies()const{return m_vecSpecies.size();}

	float				BestEverFitness()const{return m_dBestEverFitness;}
	float				Fitness_Best(int pos) { return m_vecBestGenomes[pos].Fitness(); }

	void				SetFitness(int genome, float fitness){m_vecGenomes[genome].SetFitness(fitness);}

	void    SetBestEverFitness(double fitness) { m_dBestEverFitness = fitness; }

	void    SetFitnessSpecies(double fitness) {
		for (int i = 0; i < m_vecSpecies.size(); i++)
		{
			m_vecSpecies[i].SetBestFitness(0);
		}
	}

	void Set_is_improved(bool is) { is_improved = is; }
};

//#endif