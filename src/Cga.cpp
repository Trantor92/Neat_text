#include "Cga.h"


//---------------------------- constructor --------------------------------
//	crea una popolazione di size genomi aventi le caratteristiche topologiche
//  di struttura minima con solo inputs ed outputs; usguali strutture ma pesi
//  sinaptici diversi
//-------------------------------------------------------------------------
Cga::Cga(int  size,
         int  inputs,
         int  outputs):m_iPopSize(size),
                    m_iGeneration(0),
                    m_pInnovation(NULL),
                    m_iNextGenomeID(0),
                    m_iNextSpeciesID(0),
                    m_dBestEverFitness(0.f),
                    m_dTotFitAdj(0.f),
                    m_dAvFitAdj(0.f)
{

	m_vecGenomes.resize(m_iPopSize);

	//crea la popolazione di genomi
	for (int i=0; i<m_iPopSize; ++i)
	{
		m_vecGenomes[i] = (CGenome(m_iNextGenomeID++, inputs, outputs));
	}


	//crea la lista delle innovazioni sfruttando come modello il primo genoma, 
	//tanto sono tutti strutturalmente identici	
	m_pInnovation = new CInnovation(m_vecGenomes[0].LinkGenes(), m_vecGenomes[0].NeuronGenes());

	MaxGenAllowedNoImprovement = CParams::iNumGensAllowedNoImprovement;

}



//------------------------------------- destructor -----------------------------
//  cancella la lista delle innovazioni
//------------------------------------------------------------------------
Cga::~Cga()
{
  if (m_pInnovation)
  {
    delete m_pInnovation;
      
    m_pInnovation = NULL;
  } 
}

//-------------------------------CreatePhenotypes-------------------------
//
//	itera su tutti i genomi della popolazione e ne crea il fenotipo.
//  ritorna un vector contenente i fenotipi
//-------------------------------------------------------------------------
vector<CNeuralNet*> Cga::CreatePhenotypes()
{
	vector<CNeuralNet*> networks; networks.resize(m_iPopSize);

#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

	for (int i=0; i<m_iPopSize; i++)
	{
		//crea il fenotipo
		CNeuralNet* net = m_vecGenomes[i].CreatePhenotype();

		networks[i] = net;
	}

	return networks;
}



//------------------------------------- Epoch ----------------------------
//
//  Questa funzione esegue una generazione dell'algoritmo genetico e ritorna
//  un vector di puntatori ai fenotipi che sono generati dalla riproduzione 
//  della popolazione. viene fornita in argomento la fitness dei Brains associati
//  ai genomi della popolazione
//------------------------------------------------------------------------
vector<CNeuralNet*> Cga::Epoch(const vector<float> &FitnessScores)
{
  //controllo che le dimensioni siano corrette
  if (FitnessScores.size() != m_vecGenomes.size())
  {
	  string s_temp = "scores=" + itos(FitnessScores.size()) + "/gens=" + itos(m_vecGenomes.size());

#ifdef VIEWER
	  wstring s = s2ws(s_temp);
	  MessageBox(NULL, s.c_str(), L"Error", MB_OK);
#else
	  cout << s_temp << endl;
	  exit(0);
#endif // VIEWER

    
  }


  //si associa la fitness ai relativi genomi
  for (int gen = 0; gen<m_vecGenomes.size(); ++gen)
  {
	m_vecGenomes[gen].SetFitness(FitnessScores[gen]);
  }

  if (m_iGeneration == 0)//serve solo nel caso che le fitness siano negative, poichè m_dBestEverFitness viene
  {					     //inizializzato a 0

	m_dBestEverFitness = m_vecGenomes[0].Fitness();
  }
 

  //------- speciazione preliminare --------------------
  //si specia preliminarmente la popolazione per permettere a Cga::ResetAndKill()
  //di eseguire correttamente la funzione di eliminare le specie che non migliorano
  Speciate();


  //resetta alcuni valori che permettono la speciazione e uccide le specie che non migliorano
  vector<float> My_FitnessScores = ResetAndKill(FitnessScores);


  //ordina i genomi per fitness decrescente e ne registra i migliori in m_vecBestGenomes
  SortAndRecord();

  //raggruppa la popolazione in specie aventi topologie simili, calcola le fitness modificate
  //e il numero di figli concessi ad ogni specie
  SpeciateAndCalculateSpawnLevels();


  //------------- salvataggio su file delle info sulla generazione corrente ------------------
  //--------------------------- e sui migliori genomi ----------------------------------------
  string name;
  //info sulle specie
  //name = "dbg_SpeciesDump_gen" + itos(m_iGeneration) + ".txt";
  //SpeciesDump(name);//scrittura su file

  //scrittura su file della lista delle innovazioni.
  //questo file viene riscritto ad ogni generazione
  m_pInnovation->Write("dbg_Innovations.txt", m_iGeneration);

  //info sui 4 migliori genomi
  if (is_improved)
  {
	  name = "Member_0\\dbg_BestGenome_gen" + itos(m_iGeneration) + ".txt";
	  WriteGenome(name, 0);//scrittura su file*/
  }
  /*name = "Member_1\\dbg_BestGenome_gen" + itos(m_iGeneration) + ".txt";
  WriteGenome(name, 1);//scrittura su file

  name = "Member_2\\dbg_BestGenome_gen" + itos(m_iGeneration) + ".txt";
  WriteGenome(name, 2);//scrittura su file

  name = "Member_3\\dbg_BestGenome_gen" + itos(m_iGeneration) + ".txt";
  WriteGenome(name, 3);//scrittura su file*/



  //conterrà la nuova popolazione di genomi
  vector<CGenome> NewPop; NewPop.resize(CParams::iPopSize);

  //numero di genomi prodotti nella riproduzione fino ad ora,
  //serve per fermarsi una volta che si sono generati m_iPopSize individui. 
  int NumSpawnedSoFar = 0;
  
  //conterrà di volta in volta il genoma risultante dalla riproduzione
  CGenome baby;

  //ora si itera sulle specie estraendo al loro interno i genomi da accoppiare (crossover)
  //e mutare	
  for (int spc=0; spc<m_vecSpecies.size(); ++spc)
  {
    //poichè il numero di figli concessi ad ogni specie è un float che viene arrotondato
	//ad intero, è possibile sforare in eccesso o in difetto il vincolo di avere m_iPopSize 
	//individui ad ogni generazione. bisogna pertanto controllare che ciò non accada
    if (NumSpawnedSoFar < CParams::iPopSize)
    {
      //numero di figli concessi alla specie in esame
	  //Rounded esegue l'arrotondamento all'intero più vicino.
      int NumToSpawn = Rounded(m_vecSpecies[spc].NumToSpawn());

      bool bChosenBestYet = false;

      while (NumToSpawn--)//fino a che non ho generato tutti i figli che gli spettano
      {
        //prima di tutto si seleziona il miglior individuo della specie (Leader) e si trasferisce
		//tal quale, senza mutazioni, alla nuova generazione NewPop.
        if (!bChosenBestYet)
        {         
          baby = m_vecSpecies[spc].Leader();

          bChosenBestYet = true;
        }

        else//se ho già trasferito l'individuo migliore alla nuova generazione
        {
          //se il numero di membri di questa specie è 1, allora si può solo applicare
		  //le mutazioni e non il crossover
          if (m_vecSpecies[spc].NumMembers() == 1)
          {         
            //si estrae l'unico individuo
            baby = m_vecSpecies[spc].Spawn();
          }
          else//se sono presenti più membri allora si può applicare anche il crossover
          {
            //si estrae un primo individuo
            CGenome g1 = m_vecSpecies[spc].Spawn();

			//con probabilità CParams::dCrossoverRate si esegue la funzione crossover
            if (RandFloat() < CParams::dCrossoverRate)
            {
              //si estrae un secondo individuo, controllando che non sia lo stesso di prima
              CGenome g2 = m_vecSpecies[spc].Spawn();

              //numero di tentativi nel trovare un genoma membro valido, ovvero
			  //tale da essere diverso dal primo
              int NumAttempts = 5;

			  bool is_cross = false;

              while ((g1.ID() == g2.ID()) && (NumAttempts--))//finchè sono identici
			  {
                g2 = m_vecSpecies[spc].Spawn();

				if (g1.ID() != g2.ID())//se sono due individui distinti faccio il crossover
				{
				  is_cross = true;

				  //Cga::Crossover() è definito in modo tale che se viene fuori 
				  //un individuo aciclico restituisce g1
				  baby = Crossover(g1, g2);
				}
              }

			  //se il numero di tentativi non è bastato per trovare due individui distinti
			  if (!is_cross)
				  baby = g1;

            }//if crossover
            else
            {
              baby = g1;
            }
          }
		  
		  //arrivati qui si possiede un genoma baby al quale applcare le mutazioni
          
          //numero d'identificazione del nuovo genoma
          ++m_iNextGenomeID;
          baby.SetID(m_iNextGenomeID);


		  //------------ mutazioni --------------------------

          //eventuale aggiunta di un nodo
          if (baby.NumNeurons() < CParams::iMaxPermittedNeurons)
          {      
            baby.AddNeuron(CParams::dChanceAddNode,
                           *m_pInnovation);
          }

          //eventuale aggiunta di un link
          baby.AddLink(CParams::dChanceAddLink,
                       *m_pInnovation,
                       CParams::iNumAddLinkAttempts,
						CParams::dChanceAddRecurrentLink,
						CParams::iNumTrysToFindLoopedLink);

          //eventuale mutazione dei pesi
          baby.MutateWeights(CParams::dMutationRate,
                             CParams::dProbabilityWeightReplaced,
                             CParams::dMaxWeightPerturbation);

		  //eventuale mutazione del parametro beta dei nodi
          baby.MutateActivationResponse(CParams::dActivationMutationRate,
                                        CParams::dMaxActivationPerturbation);
        }

        //si ordinano i geni nel genoma per numero d'innovazione crescente.
		//fondamentale per la speciazione nel calcolo della compatibilità.
        baby.SortGenes();

        //si aggiunge il nuovo individuo alla popolazione
        NewPop[NumSpawnedSoFar] = (baby);

        ++NumSpawnedSoFar;

        if (NumSpawnedSoFar == CParams::iPopSize)
        {        
          NumToSpawn = 0;
        }

      }//end while
       
    }//end if
     
  }//next species


  //Se a causa dell'arrotondamento del numero di figli di ogni specie
  //se ne sono generati meno del dovuto, allora si estraggono quelli mancanti fra i milgiori
  //della popolazione. questo viene ottenuto attraverso la funzione Cga::TournamentSelection().
  if (NumSpawnedSoFar < CParams::iPopSize)
  {

    //si calcola il numero di figli che mancano
    int Rqd = CParams::iPopSize - NumSpawnedSoFar;

    //si estraggono e si aggiungono alla nuova popolazione
    while (Rqd)
    {
      NewPop[CParams::iPopSize - Rqd] = (TournamentSelection(m_iPopSize/5));

	  --Rqd;
    }
  }

  //si rimpiazza la popolazione vecchia con quella nuova
  m_vecGenomes = NewPop;


  //si creano i fenotipi dei genomi della nuova popolazione
  vector<CNeuralNet*> new_phenotypes; new_phenotypes.resize(m_vecGenomes.size());

#ifdef _OPENMP
#pragma omp parallel for
#endif // DEBUG

  for (int gen=0; gen<m_vecGenomes.size(); ++gen)
  {    
    CNeuralNet* phenotype = m_vecGenomes[gen].CreatePhenotype();

    new_phenotypes[gen] = phenotype;
  }

  //incremento del contatore delle generazione e si ritornano i nuovi fenotipi
  ++m_iGeneration;

  return new_phenotypes;
}


//--------------------------- SortAndRecord-------------------------------
//
//  Ordina la popolazione per fitness decrescente e tiene traccia in m_vecBestGenomes
//  dei migliori individui
//------------------------------------------------------------------------
void Cga::SortAndRecord()
{
  //si ordinano gli individui per fitness decrescente
  sort(m_vecGenomes.begin(), m_vecGenomes.end());

  //si controlla che se il milgior genoma abbia superato il record
  if (m_vecGenomes[0].Fitness() > m_dBestEverFitness)
  {
	  is_improved = true;
	  CParams::elenco_best.insert(std::pair<int, int>(m_iGeneration, count_improving++));
    m_dBestEverFitness = m_vecGenomes[0].Fitness();
  }

  //si tiene traccia in m_vecBestGenomes dei migliori individui
  StoreBestGenomes();
}

//----------------------------- StoreBestGenomes -------------------------
//
//  funzione usata per tenere traccia dei migliori individui prima che avvenga
//  riproduzione. questi vengono salvati in m_vecBestGenomes.
//------------------------------------------------------------------------
void Cga::StoreBestGenomes()
{
  //si cancella il vecchio vector e si carica il nuovo
	m_vecBestGenomes.clear(); m_vecBestGenomes.resize(CParams::iNumBestBrains);

  for (int gen=0; gen<CParams::iNumBestBrains; ++gen)
  {
    m_vecBestGenomes[gen] = (m_vecGenomes[gen]);
  }
}

//----------------- GetBestPhenotypesFromLastGeneration ------------------
//
//  ritorna un vector contente i fenotipi dei migliori individui prima che avvenisse
//  la riproduzione (fenotipi associati ai genomi in m_vecBestGenomes)
//------------------------------------------------------------------------
vector<CNeuralNet*> Cga::GetBestPhenotypesFromLastGeneration()
{
	vector<CNeuralNet*> brains; brains.resize(m_vecBestGenomes.size());

  for (int gen=0; gen<m_vecBestGenomes.size(); ++gen)
  {
    brains[gen] = (m_vecBestGenomes[gen].CreatePhenotype());
  }

  return brains;
}


//--------------------------- AdjustSpecies ------------------------------
//
//  questa funzione semplicemente itera su ogni specie la funzione che ne
//  calcola la fitness modificata dei membri
//------------------------------------------------------------------------
void Cga::AdjustSpeciesFitnesses()
{
  for (int sp=0; sp<m_vecSpecies.size(); ++sp)
  {
    m_vecSpecies[sp].AdjustFitnesses();
  }
}



//------------------ SpeciateAndCalculateSpawnLevels ---------------------
//
//  Raggruppo i genomi in specie calcolando la compatibilità che questi hanno con
//  i genomi Leader delle specie esistenti. La funzione qunidi calcola la fitness modificata
//  di ogni individuo e ne determina il numero di figli concessi (float).
//------------------------------------------------------------------------
void Cga::SpeciateAndCalculateSpawnLevels()
{

  bool bAdded = false;

 
  //si itera su tutti i genomi controllando man mano la compatibilità
  //con le speci in lista.
  for (int gen=0; gen<m_vecGenomes.size(); ++gen)
  {

    //si calcola la compatibilità che il genoma ha con il leader della specie.
	//Se risultano compatibili allora lo associo a questa specie. se non risulta
	//compatibile con nessuna specie, ne viene creata una con lui come leader
    for (int spc=0; spc<m_vecSpecies.size(); ++spc)
    {
      float compatibility = m_vecGenomes[gen].GetCompatibilityScore(m_vecSpecies[spc].Leader());

      //se risulta compatibile con questa specie
      if (compatibility <= CParams::dCompatibilityThreshold)
      {
		//lo associo a questa specie
        m_vecSpecies[spc].AddMember(m_vecGenomes[gen]);

        //faccio sapere al genoma in quale specie è
        m_vecGenomes[gen].SetSpecies(m_vecSpecies[spc].ID());

        bAdded = true;

        break;//facendo così non lo si aggiunge ad altre specie compatibili 
      }
    }
    
    if (!bAdded)//se non è compatibile a nessuna specie esistente
    {
      //ne creo una nuova

		m_vecSpecies.resize(m_vecSpecies.size() + 1, CSpecies(m_vecGenomes[gen], m_iNextSpeciesID));
      //m_vecSpecies.push_back(CSpecies(m_vecGenomes[gen], m_iNextSpeciesID));

	  m_vecGenomes[gen].SetSpecies(m_iNextSpeciesID++);

    }

    bAdded = false;
	
  }


  //nb: grazie a Cga::SortAndRecord() i membri delle specie sono stati 
  //inseriti in ordine decrescente di fitness.

  //--------------- correzione dei Leader ----------------------------

  //questo codice serve per settare correttamente i leader visto che la modificazione della soglia
  //di compatibilità può comportare lo spostamento del leader verso specie a più passo numero
  //d'identificazione o anche causarne un completo svuotamento.


  for (int spc = 0; spc < m_vecSpecies.size(); ++spc)
  {
	  if (m_vecSpecies[spc].GenomesOfMembers().size() > 0)
	  {
		  //visto che il leader può essere cambiato bisogna risettare lui
		  //e le variabili ad esso connesse
		  float FitnessOfBest = m_vecSpecies[spc].GenomesOfMembers()[0]->Fitness();
		  if (m_vecSpecies[spc].BestFitness() != FitnessOfBest)
		  {
			  m_vecSpecies[spc].SetLeader();

			  if (m_vecSpecies[spc].BestFitness() < FitnessOfBest)
				  m_vecSpecies[spc].GensNoImprovement(0);
		  }
	  }
  }


  //------------------- calcolo della fitness modificata --------------------

  //adesso tutti i genomi sono stati assegnati ad una specie, perciò la fitness deve essere
  //modificata per in virtu della condivisione della fitness all'interno della specie
  //(tenendo in considerazione l'età della specie).
  
  AdjustSpeciesFitnesses();//tiene conto dell'età
  
  //si calcola la fitness modificata
  for (int gen=0; gen<m_vecGenomes.size(); ++gen)
  {   
     m_dTotFitAdj += m_vecGenomes[gen].GetAdjFitness();//viene inizializzata a zero in Cga::ResetAndKill()
  }

  m_dAvFitAdj = m_dTotFitAdj/m_vecGenomes.size();

  //si calcola quanti figli ogni genoma può avere
  for (int gen=0; gen<m_vecGenomes.size(); ++gen)
  {   
     float ToSpawn = m_vecGenomes[gen].GetAdjFitness() / m_dAvFitAdj;

     m_vecGenomes[gen].SetAmountToSpawn(ToSpawn);
  }

  //si itera su tutte le specie e si calcola quanti figli sono loro concessi
  for (int spc=0; spc<m_vecSpecies.size(); ++spc)
  {
    m_vecSpecies[spc].CalculateSpawnAmount();
  }
}

//  come sopra, esegue una speciazione preliminare, senza cacolare il numero di figli
//  concessi alle specie, che serve solo a Cga::ResetAndKill().
//  tuttavia è in questa funzione e non in quella sopra che viene eseguito l'aggiustamento
//  della variabile di soglia di compatibilità
void Cga::Speciate()
{

	vector<CSpecies>::iterator curSp = m_vecSpecies.begin();

	while (curSp != m_vecSpecies.end())
	{
		curSp->Purge_preliminar();

		++curSp;
	}

	bool bAdded = false;

	//viene eseuita qui la modificazione eventuale della soglia di compatibilità poichè
	//Cga::Speciate() viene eseguita prima di SpeciateAndCalculateSpawnLevels() in Cga::Epoch().
	//questa funzione modifica, se desiderato, la soglia di compatibilità al fine controllare
	//il numero di specie presenti nel sistema
	AdjustCompatibilityThreshold();

	for (int i = 0; i < m_vecGenomes.size(); ++i)
		m_vecGenomes[i].DeletePhenotype();//DestroyPhenotype();

	sort(m_vecGenomes.begin(), m_vecGenomes.end());

	//speciazione
	for (int gen = 0; gen<m_vecGenomes.size(); ++gen)
	{
		for (int spc = 0; spc<m_vecSpecies.size(); ++spc)
		{
			float compatibility = m_vecGenomes[gen].GetCompatibilityScore(m_vecSpecies[spc].Leader());

			if (compatibility <= CParams::dCompatibilityThreshold)
			{
				m_vecSpecies[spc].AddMember(m_vecGenomes[gen], true);

				m_vecGenomes[gen].SetSpecies(m_vecSpecies[spc].ID());

				bAdded = true;

				break;
			}
		}
	}

	//serve per garantire che la corretta assegnazione del Leader alle specie ed il relativo
	//conteggio delle generazioni senza miglioramenti

	for (int spc = 0; spc < m_vecSpecies.size(); ++spc)
	{
		if (m_vecSpecies[spc].GenomesOfMembers().size() > 0)
		{
			float FitnessOfBest = m_vecSpecies[spc].GenomesOfMembers()[0]->Fitness();
			if (m_vecSpecies[spc].BestFitness() != FitnessOfBest)
			{
				m_vecSpecies[spc].SetLeader();

				if (m_vecSpecies[spc].BestFitness() < FitnessOfBest)
					m_vecSpecies[spc].GensNoImprovement(0);
			}
		}
	}
}


//--------------------- AdjustCompatibilityThreshold ---------------------
//
//  questa funzione modifica, se deisderato, la soglia dicompatibilità nel tentativo
//  di controllare in numero di specie presenti nel sistema sotto un certo massimo.
//------------------------------------------------------------------------
void Cga::AdjustCompatibilityThreshold()
{
  //se CParams::iMaxNumberOfSpecies < 1 allora l'utente ha disattivato questa
  //funzione.
  if (CParams::iMaxNumberOfSpecies < 1) return;
  
  const float ThresholdIncrement = 0.01f;

  //troppe specie, alzo la soglia
  if (m_vecSpecies.size() > CParams::iMaxNumberOfSpecies)
  {
    CParams::dCompatibilityThreshold += ThresholdIncrement;
  }

  //poche specie, abbasso la soglia
  else if (m_vecSpecies.size() <= (CParams::iMaxNumberOfSpecies/3 + 1))
  {
    CParams::dCompatibilityThreshold -= ThresholdIncrement;
  }

  return;  
}





//-----------------------------------AddNeuronID----------------------------
//
//	controlla se il numero identificativo di un nodo è presente in un vector 
//  contenente i numeri identificativi dei nodi di un genoma.
//  Se non è presente allora lo aggiunge a tale vector.
//  Utilizzata Cga::Crossover().
//------------------------------------------------------------------------
void Cga::AddNeuronID(const int nodeID, vector<int> &vec)
{
	for (int i = 0; i<vec.size(); i++)
	{
		if (vec[i] == nodeID)
		{
			//già presente
			return;
		}
	}

	vec.resize(vec.size() + 1, nodeID);

	return;
}

              
//-----------------------------------Crossover----------------------------
//	questa funzione esegue il crossover come definito nel NEAT di due genomi,
//  mum e dad, e ne retituisce il genoma risultante
//------------------------------------------------------------------------
CGenome Cga::Crossover(CGenome& mum, CGenome& dad)
{

  //aiuta a chiarire il codice
  enum parent_type{MUM, DAD,};
  
  //prima di tutto si individua da quale genoma ereditare i geni disjoint/excess
  //Ovvero il genoma a più alta fitness. 
  parent_type best;

  //se i due genomi sono a pari fitness, i geni vengono ereditati dal genoma (dei link) più corto,
  //perchè si vuole mantenere il più piccola possibile la topologia delle reti.
  if (mum.Fitness() == dad.Fitness())
  {
    //se i due genomi (dei link) sono di egual linghezza allora i geni vengono
	//scelti a casa fra mum e dad
    if (mum.NumGenes() == dad.NumGenes())
    {
      best = (parent_type)RandInt(0, 1);
    }
    else
    {
      if (mum.NumGenes() < dad.NumGenes())
      {
        best = MUM;
      }  
      else
      {
        best = DAD;
      }
    }
  }
  else 
  {
    if (mum.Fitness() > dad.Fitness())
    {
      best = MUM;
    }

    else
    {
      best = DAD;
    }
  }//end best


  //questi vector conterranno il genoma dei nodi e dei link del genoma figlio
  vector<SNeuronGene>  BabyNeurons;
  vector<SLinkGene>    BabyGenes;

  //vector temporaneo per immagazzinare i numeri identificativi dei nodi aggiunti
  vector<int> vecNeurons;

  //si creano degli iteratori per muoversi lungo i genomi dei link dei genitori,
  //inizializzandoli con la posizione del primo gene
  vector<SLinkGene>::iterator curMum = mum.StartOfGenes();
  vector<SLinkGene>::iterator curDad = dad.StartOfGenes();

  //conterrà il gene di volta in volta selezionato per essere aggiunto al genoma figlio
  SLinkGene SelectedGene;

  //si itera sui genomi dei link dei genonitori finchè non si raggiunge 
  //la fine di entrambi
  while (!((curMum == mum.EndOfGenes()) && (curDad == dad.EndOfGenes())))
  {
	
    //si è raggiunta la fine del genoma della madre ma non del padre (excess del padre)
    if ((curMum == mum.EndOfGenes())&&(curDad != dad.EndOfGenes()))
    {
      //se il padre è il migliore si ereditano i suoi geni excess
      if (best == DAD)
      {
        SelectedGene = *curDad;
      }

      //ci si sposta sul prossimo gene del padre
	  ++curDad;
    }
	//altrimneti, se si è raggiunta la fine del genoma del padre ma non della madre (excess della madre)
    else if ( (curDad == dad.EndOfGenes()) && (curMum != mum.EndOfGenes()))
    {
	  //se la madre è la migliore si ereditano i suoi geni excess
      if (best == MUM)
      {
        SelectedGene = *curMum;
      }
			
	  //ci si sposta sul prossimo gene della madre
      ++curMum;
    }
    //altrimenti, se il numero d'innovazione del gene della madre è minore di quello
	//del padre (disjoint della madre)
    else if (curMum->InnovationID < curDad->InnovationID)
    {
      //se la madre è la migliore si ereditano i suoi geni disjoint
      if (best == MUM)
      {
        SelectedGene = *curMum;
      }

	  //ci si sposta sul prossimo gene della madre
      ++curMum;
    }
	//altrimenti, se il numero d'innovazione del gene del padre è minore di quello
	//della madre (disjoint del padre)
    else if (curDad->InnovationID < curMum->InnovationID)
    {
	  //se il padre è il migliore si ereditano i suoi geni disjoint
      if (best = DAD)
      {
        SelectedGene = *curDad;
      }

	  //ci si sposta sul prossimo gene del padre
      ++curDad;
    }
    //altrimenti, se i geni hanno lo stesso numero d'innovazione (matching genes)
	//allora vengono ereditati in modo casuale dal padre o dalla madre
    else if (curDad->InnovationID == curMum->InnovationID)
    {
      //sceglie il gene dal padre o dalla madre (prob 0.5)
      if (RandFloat() < 0.5f)
      {
        SelectedGene = *curMum;
      }
      else
      {
        SelectedGene = *curDad;
      }

      //ci si sposta sui prossimi geni
      ++curMum;
      ++curDad;
    }
	
	//si aggiunge il gene al genoma del figlio con un
    //meccanismo di controllo: si impedisce di aggiungere lo stesso gene due volte di fila
    if (BabyGenes.size() == 0)
    {
      BabyGenes.resize(BabyGenes.size() + 1, SelectedGene);
    }
    else
    {
      if (BabyGenes[BabyGenes.size()-1].InnovationID !=
          SelectedGene.InnovationID)
      {
		  BabyGenes.resize(BabyGenes.size() + 1, SelectedGene);
      }
    }   


    //si aggiungono i numeri identificativi dei nodi che vengono connessi dal gene
	//appena introdotto, nel vector vecNeurons, controllando questi non siano già presenti.		
    AddNeuronID(SelectedGene.FromNeuron, vecNeurons);
    AddNeuronID(SelectedGene.ToNeuron, vecNeurons);
		
  }//end while

  //si mettono in ordine i nodi per numero identificativo crescente
  sort(vecNeurons.begin(), vecNeurons.end());
  
  //e si aggiungono al genoma dei nodi del figlio
  BabyNeurons.resize(vecNeurons.size());
  for (int i=0; i<vecNeurons.size(); i++)
  {
      BabyNeurons[i] = (m_pInnovation->CreateNeuronFromID(vecNeurons[i]));
	  //BabyNeurons.push_back(m_pInnovation->CreateNeuronFromID(vecNeurons[i]));
  }

  //in ultimo si crea il genoma completo del figlio
  CGenome babyGenome(m_iNextGenomeID,
                     BabyNeurons,
                     BabyGenes,
                     mum.NumInputs(),
                     mum.NumOutputs());



  //si construsce e associa il fenotipo 
  /*babyGenome.CreatePhenotype();

  //si controlla che il figlio non sia ciclico, altrimenti si restituisce di default
  //il genoma dellla madre
  if (babyGenome.Depth() == -1)
	  babyGenome = mum;*/



  return babyGenome;
}


//--------------------------- TournamentSelection ------------------------
//  Questa funzione estrae dall'intera popolazione un individuo fra i migliori.
//  In particolare esegue NumComparisons estrazioni casuali e restituisce il genoma 
//  migliore.
//------------------------------------------------------------------------
CGenome Cga::TournamentSelection(const int NumComparisons)
{
	//conterrà la fitness migliore fra quelli estratti
	float BestFitnessSoFar = 0.f;

	//conterrà l'indice del genoma migliore
	int ChosenOne = 0;

	//Selezione NumComparisons individui dalla popolazione, a caso e
	//li testa contro il migliore trovato fino a quel momemento
	for (int i = 0; i<NumComparisons; ++i)
	{
		int ThisTry = RandInt(0, m_vecGenomes.size() - 1);

		//se ho estratto un individuo migliore
		if (m_vecGenomes[ThisTry].Fitness() > BestFitnessSoFar)
		{
			ChosenOne = ThisTry;

			BestFitnessSoFar = m_vecGenomes[ThisTry].Fitness();
		}
	}

	//return the champion
	return m_vecGenomes[ChosenOne];
}



//--------------------------- ResetAndKill ----------------------------
//
//  questa funzione resetta alcuni valori necessari alla generazione successiva, come il disassociare
//  i vecchi fenotipi.
//  Svuota i vector dei membri delle specie poichè la popolazione è cambiata riproducendosi, mantenendo
//  tuttavia le info sui leader in modo tale che si possa speciare la nuova popolazione.
//  Nel caso in cui la specie non migliori, questa viene eliminata insieme a tutti i genomi membri, dalla
//  popolazione.
//  Restituisce un vector contenente la fitness dei genomi sopravvissuti.
//--------------------------------------------------------------------
vector<float> Cga::ResetAndKill(vector<float> FitnessScores)
{
  m_dTotFitAdj = 0.f;
  m_dAvFitAdj  = 0.f;

  //iteratore sulle specie presenti nella popolazione
  vector<CSpecies>::iterator curSp = m_vecSpecies.begin();


  //--- salvataggio delle informazioni sui membri delle specie -------------
  
  //vector di vector dei membri di ogni specie
  //vector<vector<CGenome*>> genome_members;
  vector<CGenome*> genome_members;

  //ed i relativi numeri identificativi
  vector<int> ID_temp;
  vector<vector<int>> ID_member; ID_member.resize(m_vecSpecies.size());

  int i = 0;
  while (curSp != m_vecSpecies.end())
  {

	//genome_members[i].push_back(curSp->GenomesOfMembers());
	  genome_members = (curSp->GenomesOfMembers());

	//int genome_member_size = genome_members[i].size();
	  int genome_member_size = genome_members.size();
	  ID_temp.resize(genome_member_size);
	for (int j = 0; j < genome_member_size; j++)
	{
	  //ID_temp.push_back(genome_members[i][j]->ID());
		ID_temp[j] = (genome_members[j]->ID());
	}

	//fose qui devo deallocare genome_members

	ID_member[i] = (ID_temp); ID_temp.clear();

	curSp++; i++;
  }

  genome_members.clear();

  //--- si uccidono le specie che non migliorano e si -------------------------------
  //-------- disassociano i genomi dalle specie -------------------------------------

  int Treshold_noimprovement = Calcola_MaxGenerationNoImprovement();

  int spc = 0;
  curSp = m_vecSpecies.begin();
  while (curSp != m_vecSpecies.end())
  {
	 
    //si uccidono le specie che sono rimaste vuote. Anche quelle che non migliorano
	//a meno che non sia la specie milgiore
    if ( (curSp->NumMembers()==0) || ((curSp->GensNoImprovement() >= Treshold_noimprovement) &&
         (curSp->BestFitness() < m_dBestEverFitness)) )
	{

	  vector<CGenome>::iterator curMember;

	  //si eliminano dalla popolazione i membri di questa specie
	  for(int i = 0; i < ID_member[spc].size(); i++)
	  {
		int Pos_member = GetMemberPos(ID_member[spc][i]);

		curMember = m_vecGenomes.begin() + Pos_member;
		m_vecGenomes.erase(curMember);

		//si modifica il vector delle fitness di conseguenza
		FitnessScores.erase(FitnessScores.begin() + Pos_member);
	  }
	  
	  

	  //disaddocia i genomi alla specie
	  //curSp->Purge_specializzata();
	  curSp->Purge();
	  /*prababilmente qui si devono deallocare gli elementi del m_vecmember della specie*/
	  /*forse basta definire una nuova funzione purge specializzata*/

	  //si elimina la specie svuotata
	  curSp = m_vecSpecies.erase(curSp);;
    }
	else
	{
	  curSp->Purge();

      ++curSp;
	}
		
	ID_member[spc].clear();
	spc++;
  }

  ID_member.clear();

  //cancelliamo anche i vecchi fenotipi
  for (int gen=0; gen<m_vecGenomes.size(); ++gen)
  {
    m_vecGenomes[gen].DeletePhenotype();
  }

  return FitnessScores;
}

int Cga::Calcola_MaxGenerationNoImprovement()
{
	for (int i = 0; i < m_vecSpecies.size(); i++)
	{
		if (m_vecSpecies[i].BestFitness() == m_dBestEverFitness)
		{
			int GenNoImproBest = m_vecSpecies[i].GensNoImprovement();

			return MaxGenAllowedNoImprovement = (GenNoImproBest > CParams::iNumGensAllowedNoImprovement) ?
				((MaxGenAllowedNoImprovement < GenNoImproBest) ? GenNoImproBest : MaxGenAllowedNoImprovement) :
				CParams::iNumGensAllowedNoImprovement;
		}
	}
}


//-------------------------- GetMemberPos ----------------------------
//
//  fornisce la posizione in m_vecGenomes a partire dall'ID, del genoma.
//  se tale genoma non è presente, restituisce il valore di errore -1
//--------------------------------------------------------------------
int	Cga::GetMemberPos(int ID)
{
	for (size_t i = 0; i< m_vecGenomes.size(); i++)
	{
		int d = m_vecGenomes[i].ID();

		if (d == ID)
		{
			return i;
		}
	}

	string s_temp = itos(ID);

#ifdef VIEWER
	wstring s = s2ws(s_temp);
	MessageBox(NULL, L"Error in CGa::GetMemberPos", s.c_str(), MB_OK);
#else
	cout << s_temp << endl;
#endif // VIEWER


	return -1;
}

#ifdef VIEWER

//--------------------------- RenderSpeciesInfo --------------------------
//
//  questa funzione sulla surface grafica in argomento tutte le caratteristiche
//  della specie migliore e costrisce la barra colorata che rappresenta l'intera
//  popolazione speciata
//------------------------------------------------------------------------
void Cga::RenderSpeciesInfo(HDC &surface, RECT db)
{
  //se non c'è ancora nessuna specie (generazione 0) allora non si disegna nulla
  if (m_vecSpecies.size() < 1) return;
  
  //intervallo di colore per la barra delle specie
  int numColours = 255/m_vecSpecies.size();

  //larghezza nella barra di un individuo
  float SlicePerBrain = (float)(db.right-db.left)/(float)(CParams::iPopSize-1);

  float left = db.left;

  //ora si disegnano rettagoli, di larghezza proporzionale al numero di membri della specie e di diverso colore,
  //per ogni specie presente
  for (int spc=0; spc<m_vecSpecies.size(); ++spc)
  {

     //si seleziona il colore
     HBRUSH PieBrush = CreateSolidBrush(RGB(numColours*spc, 255, 255 - numColours*spc));

     HBRUSH OldBrush = (HBRUSH)SelectObject(surface, PieBrush);
     
	 if (spc == m_vecSpecies.size() - 1)
	 {
	   Rectangle(surface,
			     left,
			     db.top,
			     db.right,
			     db.bottom);
	 }
     else
     {
       Rectangle(surface, 
                 left,
                 db.top,
                 left+SlicePerBrain*m_vecSpecies[spc].NumMembers(),
                 db.bottom);
     }

     left += SlicePerBrain * m_vecSpecies[spc].NumMembers();

     SelectObject(surface, OldBrush);
     DeleteObject(PieBrush);

     //scrittura sulla finestra delle caratteristiche della specie migliore
     if ( m_vecSpecies[spc].BestFitness() == m_dBestEverFitness)
     {
	   string s_temp = "Best Species ID: " + itos(m_vecSpecies[spc].ID());
	   wstring s = s2ws(s_temp);
       TextOut(surface, 5, db.top - 80, s.c_str(), s.size());
       
       s_temp = "Species Age: " + itos(m_vecSpecies[spc].Age());  
	   s = s2ws(s_temp);
       TextOut(surface, 5, db.top - 60, s.c_str(), s.size());

       s_temp = "Gens no improvement: " + itos(m_vecSpecies[spc].GensNoImprovement());
	   s = s2ws(s_temp);
       TextOut(surface, 5, db.top - 40, s.c_str(), s.size());

       s_temp = "Threshold: " + ftos(CParams::dCompatibilityThreshold);
	   s = s2ws(s_temp);
       TextOut(surface, 5, db.top - 100, s.c_str(), s.size());
     }
  }
  
  string s_temp = "Species Distribution Bar";
  wstring s = s2ws(s_temp);
  TextOut(surface, 5, db.top - 20, s.c_str(), s.size());
}
#else
void Cga::RenderSpeciesInfo() 
{
	//se non c'è ancora nessuna specie (generazione 0) allora non si disegna nulla
	if (m_vecSpecies.size() < 1) return;

	//ora si disegnano rettagoli, di larghezza proporzionale al numero di membri della specie e di diverso colore,
	//per ogni specie presente
	for (int spc = 0; spc<m_vecSpecies.size(); ++spc)
	{


		//scrittura sulla finestra delle caratteristiche della specie migliore
		if (m_vecSpecies[spc].BestFitness() == m_dBestEverFitness)
		{
			string s_temp = "Gens no improvement: " + itos(m_vecSpecies[spc].GensNoImprovement());
			cout << s_temp << endl;
			
			s_temp = "Species Age        : " + itos(m_vecSpecies[spc].Age());
			cout << s_temp << endl;

			s_temp = "Best Species ID    : " + itos(m_vecSpecies[spc].ID());
			cout << s_temp << endl;

			s_temp = "Threshold          : " + ftos(CParams::dCompatibilityThreshold);
			cout << s_temp << endl;
			cout << "\n---------------------------------------------------\n\n";
		}
	}
}
#endif // VIEWER


//--------------------------- WriteGenome --------------------------------
//
//  questa funzione, data la posizione del genoma in m_vecGenomes e il nome del file
//  in cui scrivere, ne stampa l'intero genoma.
//------------------------------------------------------------------------
bool Cga::WriteGenome(string szFileName, const int idxGenome)
{
  ofstream out(szFileName);

  if (!out) return false;//error

  if (m_vecGenomes[idxGenome].Write(out))
  {
    return true;
  }

  return false;
}


//--------------------- SpeciesDump -------------------------------------
//  questa funzione scrive lo stato delle specie attuale, sul file in argomento
//-----------------------------------------------------------------------
bool Cga::SpeciesDump(string szFileName)
{
  ofstream file(szFileName);

  if (!file) return false;//error

  file << "Best Ever Fitness is " << m_dBestEverFitness << "\n\n";
  file << "Treshold " << CParams::dCompatibilityThreshold << "\n";

  for (int i=0; i<m_vecSpecies.size(); ++i)
  {
    file << m_vecSpecies[i];
  }

  return true;

}