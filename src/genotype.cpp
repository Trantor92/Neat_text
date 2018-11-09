#include "genotype.h"


//------------------------------------------------------------------------
//
//  default constructor
//------------------------------------------------------------------------
CGenome::CGenome():m_pPhenotype(NULL),
                   m_GenomeID(0),
                   m_dFitness(0),
                   m_dAdjustedFitness(0.f),
                   m_iNumInputs(0),
                   m_iNumOutPuts(0),
                   m_dAmountToSpawn(0.f)
{}


//-----------------------------constructor--------------------------------
//	questo costruttore crea un genoma minimo in cui sono presenti un numero di input pari a inputs + 1 (bias)
//  e un numero di output pari a outputs. Ogni input è connesso ad ogni output.
//------------------------------------------------------------------------
CGenome::CGenome(int id, int inputs, int outputs) :m_pPhenotype(NULL),
m_GenomeID(id),
m_dFitness(0.f),
m_dAdjustedFitness(0.f),
m_iNumInputs(inputs),
m_iNumOutPuts(outputs),
m_dAmountToSpawn(0.f),
m_iSpecies(0)

{
	//crea i geni dei nodi di input
	float InputRowSlice = 0.8f / (float)(inputs);//solo scopo grafico

	m_vecNeurons.resize(inputs + 1 + outputs);
	int i;
	for (i = 0; i<inputs; i++)
	{
		m_vecNeurons[i] = (SNeuronGene(input, i, 0.f, 0.1f + i * InputRowSlice));
	}

	//crea il gene del nodo di bias
	m_vecNeurons[i] = (SNeuronGene(bias, inputs, 0.f, 0.9f));


	//crea i geni dei nodi di output
	float OutputRowSlice = 1 / (float)(outputs + 1);//solo scopo grafico

	for (int j = 0; j<outputs; j++)
	{
		m_vecNeurons[i+1+j] = (SNeuronGene(output, j + inputs + 1, 1.f, (j + 1)*OutputRowSlice));
	}

	//crea un gene di linkche connette ogni nodo di input ad ogni nodo di output 
	//assegna un peso casuale in -1 < w < 1, con distribuzione di probabilità lineare a tenda centrata in 0

	//m_vecLinks.resize((inputs + 1)*outputs);

	for (int i = 0; i<inputs + 1; i++)
	{
		for (int j = 0; j<outputs; j++)
		{
			m_vecLinks.push_back(SLinkGene(m_vecNeurons[i].iID,
				m_vecNeurons[inputs + j + 1].iID,
				true,
				inputs + outputs + 1 + NumGenes(),
				RandomClamped()));
		}
	}

}

//------------------------------------------------------------------------
//
//  questo costruttore crea un genome a partire dai vector dei genomi dei nodi e dei link.
//------------------------------------------------------------------------
CGenome::CGenome(int                 id,
                 vector<SNeuronGene> neurons,
                 vector<SLinkGene>   genes,
                 int                 inputs,
                 int                 outputs):m_GenomeID(id),
                                                  m_pPhenotype(NULL),
                                                  m_vecLinks(genes),
                                                  m_vecNeurons(neurons),
                                                  m_dAmountToSpawn(0.f),
                                                  m_dFitness(0.f),
                                                  m_dAdjustedFitness(0.f),
                                                  m_iNumInputs(inputs),
                                                  m_iNumOutPuts(outputs)
                                           
{}

//-------------------------------destructor-----------------------------------------------------
//  cancella il fenotipo se esiste
//----------------------------------------------------------------------------------------
CGenome::~CGenome()
{  
	if (m_pPhenotype)
	{
		delete m_pPhenotype;

		m_pPhenotype = NULL;
	}
}

//---------------------------------copy ctor---------------------------------------------
//
//---------------------------------------------------------------------------------------
CGenome::CGenome(const CGenome& g)
{
    m_GenomeID   = g.m_GenomeID;
    m_vecNeurons   = g.m_vecNeurons;
    m_vecLinks   = g.m_vecLinks;
    m_pPhenotype = NULL;						//no need to perform a deep copy
    m_dFitness   = g.m_dFitness;
    m_dAdjustedFitness = g.m_dAdjustedFitness;
    m_iNumInputs  = g.m_iNumInputs;
    m_iNumOutPuts = g.m_iNumOutPuts;
    m_dAmountToSpawn = g.m_dAmountToSpawn;
	m_iNetDepth = g.m_iNetDepth;
}

//---------------------------------assignment operator-----------------------------------
//
//----------------------------------------------------------------------------------------
CGenome& CGenome::operator =(const CGenome& g)
{
	  //controllo di non auto-assegnazione
	  if (this != &g)
	  {
      m_GenomeID         = g.m_GenomeID;
      m_vecNeurons         = g.m_vecNeurons;
      m_vecLinks         = g.m_vecLinks;
      m_pPhenotype       = NULL;				//no need to perform a deep copy
      m_dFitness         = g.m_dFitness;
      m_dAdjustedFitness = g.m_dAdjustedFitness;
      m_iNumInputs        = g.m_iNumInputs;
      m_iNumOutPuts       = g.m_iNumOutPuts;
      m_dAmountToSpawn   = g.m_dAmountToSpawn;
	  m_iNetDepth = g.m_iNetDepth;
    }

    return *this;
}

//-------------------------- InitializeWeights ---------------------------
//
//  inizializza tutti i pesi dei link a valori nell'intervallo ]-1,1[
//  con distribuzione di probabilità lineare a tenda centrata in 0
//------------------------------------------------------------------------
void CGenome::InitializeWeights()
{
  for (size_t l=0; l<m_vecLinks.size(); ++l)
  {
    m_vecLinks[l].dWeight = RandomClamped();
  }
}

//-------------------------------CreatePhenotype--------------------------
//
//	Crea un fenotipo secondo le informazioni contenute nel genoma. Ne calcola inoltre la profondità
//	ritorna un puntatore al fenotipo appena creato
//------------------------------------------------------------------------
CNeuralNet* CGenome::CreatePhenotype()
{
  //si cancella un eventuale fenotipo preesistente
  DeletePhenotype();
	
  //contenitore dei nodi del fenotipo (da notare che questa è la classe SNeuron e non SNeuronGene)
  vector<SNeuron*>  vecNeurons; vecNeurons.resize(m_vecNeurons.size());

  //si creano i nodi del fenotipo

  SNeuron* pNeuron = nullptr;
  for (size_t i=0; i<m_vecNeurons.size(); i++)
  {
    pNeuron = new SNeuron(m_vecNeurons[i].NeuronType,
                                   m_vecNeurons[i].iID,
                                   m_vecNeurons[i].dSplitY,
                                   m_vecNeurons[i].dSplitX,
                                   m_vecNeurons[i].dActivationResponse);
    
    vecNeurons[i] = pNeuron;
  }

  //pNeuron = nullptr; delete pNeuron;
	
  //ora si creano i link. è necessario questa informazione nella classe SNeuron
  for (size_t cGene=0; cGene<m_vecLinks.size(); ++cGene)
  {
    //si controlla che il link del genoma sia abilitato prima di aggiungerlo al fenotipo
    if (m_vecLinks[cGene].bEnabled)
    {
      //Ricava i puntatotori ai nodi di uscita ed entrata per il link
      int element         = GetElementPos(m_vecLinks[cGene].FromNeuron);
      SNeuron* FromNeuron = vecNeurons[element];

      element           = GetElementPos(m_vecLinks[cGene].ToNeuron);
      SNeuron* ToNeuron = vecNeurons[element];

      //crea un link temporaneo fra i nodi appena individuati, con le caratteristiche specificate nel genoma
      SLink tmpLink(m_vecLinks[cGene].dWeight,
                    FromNeuron,
                    ToNeuron,
					m_vecLinks[cGene].bRecurrent);
			
      //aggiungo il link ai nodi da cui esce ed in cui entra, modificandone rispettivamente il vettore del link
	  //in uscita e quello dei link in ingresso.
      FromNeuron->vecLinksOut.resize(FromNeuron->vecLinksOut.size()+1, tmpLink); //far questo modifica l'elemento di vecNeurons poichè è un puntatore 
      ToNeuron->vecLinksIn.resize(ToNeuron->vecLinksIn.size()+1, tmpLink);
    }
  }


 //calcolo e assegnazione della profondità del fenotipo
  //SetDepth(CalculateDepthPhenotype(vecNeurons));
  
  SetDepth(1);

  //Ora i nodi contengono tutte le informazioni sulle connessioni e una rete neurale (fenotipo)
  //può esseren creata a partire da essi.
  m_pPhenotype = new CNeuralNet(vecNeurons, m_iNetDepth);


  return m_pPhenotype;
}

/*CNeuralNet* CGenome::CreatePhenotype(int ID1, int ID2)
{//	funzione solo di controllo nell'ambito della mutazione strutturale dell'aggiunta di un link
 //	viene creato un fenotipo temporaneo solo per calcolarne la profondità, se è -1 significa che il nuovo link 
 //	rende la rete aciclica


 //si cancella un eventuale fenotipo preesistente
	DeletePhenotype();

	//contenitore dei nodi del fenotipo (da notare che questa è la classe SNeuron e non SNeuronGene)
	vector<SNeuron*>  vecNeurons;

	//si creano i nodi del fenotipo
	SNeuron* pNeuron = nullptr;
	for (size_t i = 0; i<m_vecNeurons.size(); i++)
	{
		    pNeuron = new SNeuron(m_vecNeurons[i].NeuronType,
			m_vecNeurons[i].iID,
			m_vecNeurons[i].dSplitY,
			m_vecNeurons[i].dSplitX,
			m_vecNeurons[i].dActivationResponse);

		vecNeurons.push_back(pNeuron);
	}

	pNeuron = nullptr; delete pNeuron;

	//ora si creano i link. è necessario questa informazione nella classe SNeuron
	for (size_t cGene = 0; cGene<m_vecLinks.size(); ++cGene)
	{
		//si controlla che il link del genoma sia abilitato prima di aggiungerlo al fenotipo
		if (m_vecLinks[cGene].bEnabled)
		{
			//Ricava i puntatotori ai nodi di uscita ed entrata per il link
			int element = GetElementPos(m_vecLinks[cGene].FromNeuron);
			SNeuron* FromNeuron = vecNeurons[element];

			element = GetElementPos(m_vecLinks[cGene].ToNeuron);
			SNeuron* ToNeuron = vecNeurons[element];

			//crea un link temporaneo fra i nodi appena individuati, con le caratteristiche specificate nel genoma
			SLink tmpLink(m_vecLinks[cGene].dWeight,
						  FromNeuron,
				          ToNeuron,
				          false);

			//aggiungo il link ai nodi da cui esce ed in cui entra, modificandone rispettivamente il vettore del link
			//in uscita e quello dei link in ingresso.
			FromNeuron->vecLinksOut.push_back(tmpLink); //far questo modifica l'elemento di vecNeurons poichè è un puntatore 
			ToNeuron->vecLinksIn.push_back(tmpLink);
		}
	}


	//------- si aggiunge il nuovo link ------------------
	SNeuron* FromNeuron = vecNeurons[GetElementPos(ID1)];

	SNeuron* ToNeuron = vecNeurons[GetElementPos(ID2)];

	//crazione del nuovo link
	SLink tmpLink(1.f,
		FromNeuron,
		ToNeuron,
		false);


	FromNeuron->vecLinksOut.push_back(tmpLink);
	ToNeuron->vecLinksIn.push_back(tmpLink);


	//si calcola e assegna la profondità del fenotipo così prodotto
	SetDepth(CalculateDepthPhenotype(vecNeurons));


	//si crea il fenotipo
	m_pPhenotype = new CNeuralNet(vecNeurons, m_iNetDepth);

	return m_pPhenotype;
}
*/

//-------------------------------CalculateDepthPhenotype--------------------------
//
//	funzione che restituisce la profondità del fenotipo: massimo numero di link 
//	che bisogna percorrere partendo da un nodo di input per raggiungere un output privo di
//	link in uscita.
//	se la rete è ciclica viene restituito il valore di errore -1.
//------------------------------------------------------------------------
int CGenome::CalculateDepthPhenotype( vector<SNeuron*> vecNeurons)
{
	int depth, depth_max = 0;

	for (int nd = 0; nd < m_iNumInputs + 1; nd++)//per ogni nodo di input controllo 
	{											 //tutti i percorsi che partono da esso

		//funzione ricorsiva che restituisce la profondità massima dei percorsi che partono dal nodo nd 
		depth = CalculateDepthPhenotype_rec(nd, vecNeurons, 0);

		if (depth != -1)
		{
			if (depth_max < depth)//registra la profondità massima
				depth_max = depth;
		}
		else//ho riscontrato un percorso ciclico
			return -1;
	}

	return depth_max-1;
}

int CGenome::CalculateDepthPhenotype_rec(int PosNeuron, vector<SNeuron*> vecNeurons, int it_rec)
{	//passa dal nodo PosNeuron ad uno ad esso connesso, richiamando se stesso fino a che non si passa ad un nodo privo
	//di link in uscita e sommando gli autoincapsulamenti.

	int depth, depthMax = 0;
	
	int nLinkOut = vecNeurons[PosNeuron]->vecLinksOut.size();

	int PosToNeuron;

	//contatore di controllo che il sistema esca se si supera un certo numero di incapsulamenti
	++it_rec;
	if (it_rec > CParams::i_MaxDepth)
		goto ciclica;

	//se size di vecLinksOut e nulla allora non esce nulla da questo neurone
	if (nLinkOut)
	{
		for (int lnk = 0; lnk < nLinkOut; lnk++)
		{
			//id del neurone cui il link entra, viene fornito alla funzione GetElementPos
			//che ne restituisce la posizione nell'array vecNeurons
			PosToNeuron = GetElementPos(vecNeurons[PosNeuron]->vecLinksOut[lnk].pOut->iNeuronID); 

			//si riapplica la funzione cambiando starting point
			depth = CalculateDepthPhenotype_rec(PosToNeuron, vecNeurons, it_rec);

			if (depth != -1)
			{
				if (depthMax < depth)//registra la profondità massima
					depthMax = depth;
			}
			else
				goto ciclica;//ho riscontrato un percorso ciclico
		}

		//ho percorso tutte le strade
		return depthMax+1;

	}
	else//punto di uscita iniziale dell'incapsulamento
		return depthMax+1;

ciclica://errore rete ciclica

	return -1;
}



//--------------------------- DeletePhenotype ----------------------------
//
//------------------------------------------------------------------------
void CGenome::DeletePhenotype()
{
  if (m_pPhenotype)
  {
    delete m_pPhenotype;
  }

  m_pPhenotype = NULL;
}


//---------------------------- GetElementPos -----------------------------
//
//	dato il numero identificato di un nodo, viene fornita la sua posizione nell'array m_vecNeurons
//------------------------------------------------------------------------
int CGenome::GetElementPos(int neuron_id)
{  
	for (size_t i=0; i<m_vecNeurons.size(); i++)
	{
		int d = m_vecNeurons[i].iID;
 
		if (d == neuron_id)
		{
			return i;
		}
	}

	string s_temp = itos(neuron_id);

#ifdef VIEWER
	wstring s = s2ws(s_temp);
	MessageBox(NULL, L"Error in CGenome::GetElementPos", s.c_str(), MB_OK);
#else
	cout << s_temp << endl;
#endif // VIEWER


	return -1;
}


//------------------------------DuplicateLink-----------------------------
//
//  ritoran vero se il link fra NeuronIn e NeuronOut è gia parte del genoma
//------------------------------------------------------------------------
bool CGenome::DuplicateLink(int NeuronIn, int NeuronOut)
{
	for (size_t cGene = 0; cGene < m_vecLinks.size(); ++cGene)
	{
		if ((m_vecLinks[cGene].FromNeuron == NeuronIn) && 
        (m_vecLinks[cGene].ToNeuron == NeuronOut))
		{
			//il link è già presente
			return true;
		}
	}

	return false;
}


//--------------------------------AddLink---------------------------------
//
//  crea un nuovo link con probabilità CParams::dChanceAddLink
//------------------------------------------------------------------------
void CGenome::AddLink(float       MutationRate,
                      CInnovation  &innovation,
                      int          NumTrysToAddLink,
					  float			ChanceOfLooped,
				      int NumTrysToFindLoop)
{
  //semplicemente esce senza far nulla, con probabilità 1-MutationRate
  if (RandFloat() > MutationRate) return;

  //defniscono i numeri indentificativi dei due nodi che verranno connessi 1 -> 2. 
  //-1 indica errore: coppia di nodi non valida.
  int ID_neuron1 = -1;
  int ID_neuron2 = -1;

  //flag che indica se il link punta su un layer inferiore (scopo grafico)
  bool bRecurrent = false;

  //first test to see if an attempt shpould be made to create a 
  //link that loops back into the same neuron
  if (RandFloat() < ChanceOfLooped)
  {
	  //YES: try NumTrysToFindLoop times to find a neuron that is not an
	  //input or bias neuron and that does not already have a loopback
	  //connection
	  while (NumTrysToFindLoop--)
	  {
		  //grab a random neuron
		  int NeuronPos = RandInt(m_iNumInputs + 1, m_vecNeurons.size() - 1);

		  //check to make sure the neuron does not already have a loopback 
		  //link and that it is not an input or bias neuron
		  if (!m_vecNeurons[NeuronPos].bRecurrent &&
			  (m_vecNeurons[NeuronPos].NeuronType != bias) &&
			  (m_vecNeurons[NeuronPos].NeuronType != input))
		  {
			  ID_neuron1 = ID_neuron2 = m_vecNeurons[NeuronPos].iID;

			  m_vecNeurons[NeuronPos].bRecurrent = true;

			  bRecurrent = true;

			  NumTrysToFindLoop = 0;
		  }
	  }
  }
  else {
	  //si cerca una coppia di nodi validi. si fanno NumTrysToAddLink tentativi
	  while (NumTrysToAddLink--)
	  {
		  //si scelgono due nodi, il secondo non deve essere di input o bias
		  ID_neuron1 = m_vecNeurons[RandInt(0, m_vecNeurons.size() - 1)].iID;

		  ID_neuron2 =
			  m_vecNeurons[RandInt(m_iNumInputs + 1, m_vecNeurons.size() - 1)].iID;//scegliendolo così sono sicuro
																				   //che non sia un input o bias
		  int NeuronPos1 = GetElementPos(ID_neuron1);
		  int NeuronPos2 = GetElementPos(ID_neuron2);



		  //controllo che i due nodi non siano in realtà lo stesso nodo o che non siano già connessi
		  if (DuplicateLink(ID_neuron1, ID_neuron2) ||
			  (ID_neuron1 == ID_neuron2))
		  {

			  ID_neuron1 = -1;
			  ID_neuron2 = -1;
		  }
		  else//la coppia è valida
		  {
			  //controllo che la rete non diventi ciclica con l'aggiunta di questo link
			  /*CreatePhenotype(ID_neuron1, ID_neuron2);

			  if (m_iNetDepth == -1)//messaggio di errore significa che la rete è ciclica
			  {
			  ID_neuron1 = -1;
			  ID_neuron2 = -1;
			  }
			  else//la coppia è valida e non cerco più
			  NumTrysToAddLink = 0;*/


			  NumTrysToAddLink = 0;
		  }
	  }

  }

  
  //termina la funzione se non si è trovata un coppia valida nel numero di tentativi prefissato
  if ( (ID_neuron1 < 0) || (ID_neuron2 < 0) )
  {
    return;
  }
  


  if ((ID_neuron1 == ID_neuron2) || (m_vecNeurons[GetElementPos(ID_neuron1)].dSplitY >
	  m_vecNeurons[GetElementPos(ID_neuron2)].dSplitY))
  {
	  bRecurrent = true;
  }

  //controlla nella lista delle innovazioni se questa esiste già
  int id = innovation.CheckInnovation(ID_neuron1, ID_neuron2, new_link);


  //l'innovazione non esista ancora quindi va creata
  if ( id < 0)
  {
    //creazione della nuova innovazione
    innovation.CreateNewInnovation(ID_neuron1, ID_neuron2, new_link);

    //creazione del nuovo gene
    int id = innovation.NextNumber() - 1;

    SLinkGene NewGene(ID_neuron1,
                          ID_neuron2,
                          true,
                          id,
                          RandomClamped(),
                          bRecurrent);
		
    m_vecLinks.resize(m_vecLinks.size()+1, NewGene);
  }
  else
  {
    //l'innovazione esiste già quindi si deve creare solo il nuovo gene
    SLinkGene NewGene(ID_neuron1,
                          ID_neuron2,
                          true,
                          id,
                          RandomClamped(),
                          bRecurrent);
	
	m_vecLinks.resize(m_vecLinks.size() + 1, NewGene);
  }

  return;
}


//---------------------------------AddNeuron------------------------------
//
//	crea un nuovo nodo al genoma esaminando il fenotipo. si splitta un link
//  esistente e vi si inserisce un nuovo nodo.
//------------------------------------------------------------------------
void CGenome::AddNeuron(float       MutationRate,
                        CInnovation  &innovations)
{
  //semplicemente esce senza far nulla, con probabilità 1-MutationRate
  if (RandFloat() > MutationRate) return;
  
  //se viene trovato un link valido nel cuale inserire il nuvo nodo
  //questa variabile assume valore true.
  bool bDone = false;

  //conterrà la posizione del link trovato in m_vecLinks
  int  ChosenLink = 0;

  while (!bDone)
  {
	  //si estra un link a caso fra tutti
	  ChosenLink = RandInt(0, NumGenes() - 1);

	  //si controlla che il link sia abilitato e che non abbia il bia come nodo di uscita
	  int FromNeuron = m_vecLinks[ChosenLink].FromNeuron;

	  if ((m_vecLinks[ChosenLink].bEnabled) &&
		  (m_vecNeurons[GetElementPos(FromNeuron)].NeuronType != bias))
	  {
		  bDone = true;
	  }
  }
  
  //disabilito il geno relativo al link scelto
  m_vecLinks[ChosenLink].bEnabled = false;

  //si tiene traccia del peso sinaptico originale in moda tale da riutilizzarlo per uno dei
  //due nuovi link, al fine di disturbare il meno possibile l'ottimizzazione fin qui raggiunta
  float OriginalWeight = m_vecLinks[ChosenLink].dWeight;

  //si identificano i nodi che sono connessi tramite il link scelto
  int from =  m_vecLinks[ChosenLink].FromNeuron;
  int to   =  m_vecLinks[ChosenLink].ToNeuron;

  //si calcola la posizione grafica del nuovo nodo
  float NewDepth = (m_vecNeurons[GetElementPos(from)].dSplitY + 
                     m_vecNeurons[GetElementPos(to)].dSplitY) /2.f;

  float NewWidth = (m_vecNeurons[GetElementPos(from)].dSplitX + 
                     m_vecNeurons[GetElementPos(to)].dSplitX) /2.f;

 


  /*'E possobile per il NEAT eseguire ritutamente la seguente procedura:
  
      1. Find a link. Lets say we choose link 1 to 5
      2. Disable the link,
      3. Add a new neuron and two new links
      4. The link disabled in Step 2 may be re-enabled when this genome
         is recombined with a genome that has that link enabled.
      5  etc etc

  Perciò qui si controlla se esiste gia una inovazione per il nuovo nodo (ai nuovi nodi nodi viene associato
  come numero identificativo il numero dell'innovazione). Perciò se nella rete è contemporaneamente presente
  sia il link da splittare che il nodo della innovazione allora significa che si deve creare una nuova innovazione.
  se l'innovazione esiste ma quel nodo non è presente nell'individuo in questione non si deve creare una nuova innovazione.
  se l'innovazione non è ancora stata creata va creata.*/


  //si controlla se l'innovazione data da questo nuovo nodo sia già stata creata
  int id = innovations.CheckInnovation(from,
	  to,
	  new_neuron);

  if (id >= 0)//se una rete nella popolazione ha già sviluppato l'innovazione
			  //di avere un nuovo nodo fra gli stessi due nodi: from e to
  {
    int NeuronID = innovations.GetNeuronID(id);//ID del neurone salvato nella lista delle innovazioni

    if (AlreadyHaveThisNeuronID(NeuronID))//se l'ID del neurone è già presente nella rete
    {									  //bisogna creare una nuova innovazione
      id = -1;
    }
  }
  
  if (id < 0)//l'innavozione o non esiste ancora, o è il caso speciale sopra menzionato; quindi va creata
  {
    //crea una nuova innovazione
    int NewNeuronID = innovations.CreateNewInnovation(from,
                                                      to,
                                                      new_neuron,
                                                      hidden,
                                                      NewWidth,
                                                      NewDepth);
    
    //crea il gene per il nuovo nodo e lo aggiunge al genoma dei nodi.
    m_vecNeurons.resize(m_vecNeurons.size()+1,SNeuronGene(hidden,
                                       NewNeuronID,
                                       NewDepth,
                                       NewWidth));
		


    //Due innovazioni new_link vanno create, poiché illink originale viene splittato.

    //-----------------------------------first link

    //fornisce il numero d'innovazione per il nuovo link, che anche il numero d'innovazione
    int idLink1 = innovations.NextNumber();//NextNumber() fornisce m_NextInnovationNum += 0 ma questo										   
										   //è gia stato incrementato in CreateNewInnovation().
		
    //crea la nuova innovazione
    innovations.CreateNewInnovation(from,
                                    NewNeuronID,
                                    new_link);

    //crea il gene del nuovo link e lo aggiunge al genoma dei link
    SLinkGene link1(from,
                        NewNeuronID,
                        true,
                        idLink1,
                        1.f);

    m_vecLinks.resize(m_vecLinks.size()+1, link1);


    //-----------------------------------second link

    //get the next innovation ID
    int idLink2 = innovations.NextNumber();
		
    //create the new innovation
    innovations.CreateNewInnovation(NewNeuronID,
                                    to,
                                    new_link);
		
    //create the new gene
    SLinkGene link2(NewNeuronID,
                        to,
                        true,
                        idLink2,
                        OriginalWeight);
    
	m_vecLinks.resize(m_vecLinks.size() + 1, link2);
  }

  else//non si deve creare una nuova innovazione
  {
    //l'innovazione esiste già quindi bisogna ricavare dalla lista delle innovazioni
	//le informazioni: per il nodo e i due link
    int NewNeuronID = innovations.GetNeuronID(id);
		

    int idLink1 = innovations.CheckInnovation(from, NewNeuronID, new_link);
    int idLink2 = innovations.CheckInnovation(NewNeuronID, to, new_link);


    //errore: questo non dovrebbe succedere poichè deve esistere l'innovazione nella lista
    if ( (idLink1 < 0) || (idLink2 < 0) )
    {
#ifdef VIEWER
		MessageBox(NULL, L"Error in CGenome::AddNeuron", L"Problem!", MB_OK);
		return;
#else
		cout << "Error in CGenome::AddNeuron" << endl;
		exit(0);
#endif // VIEWER


    }

    //si creano i geni dei nuovi link e si aggiungono al genoma dei link
    SLinkGene link1(from, NewNeuronID, true, idLink1, 1.f);
    SLinkGene link2(NewNeuronID, to, true, idLink2, OriginalWeight);

	m_vecLinks.resize(m_vecLinks.size() + 1, link1); 
	m_vecLinks.resize(m_vecLinks.size() + 1, link2); 


    //crea il gene del nuovo nodo e lo aggiunge al genoma dei nodi
    SNeuronGene NewNeuron(hidden, NewNeuronID, NewDepth, NewWidth);

	m_vecNeurons.resize(m_vecNeurons.size() + 1, NewNeuron);
  }

  return;
}


//--------------------------- AlreadyHaveThisNeuronID ----------------------
// 
//  controlla se un nodo avente numero identificativo ID è già presente
//  nel genoma.
//------------------------------------------------------------------------
bool CGenome::AlreadyHaveThisNeuronID(const int ID)
{
  for (size_t n=0; n<m_vecNeurons.size(); ++n)
  {
    if (ID == m_vecNeurons[n].iID)
    {
      return true;
    }
  }

  return false;
}


//------------------------------- MutateWeights---------------------------
//	itera sui geni dei link e ne muta il peso con probabilità mut_rate.
//
//	con probabilità prob_new_mut il peso viene rimpiazzato nel range ]-1,1[ 
//  con distribuzione a tenda centrata in 0.
//
//	con probabilità 1-prob_new_mut si perturba il peso.
//  dMaxPertubation è la massima perturbazione applicata.
//------------------------------------------------------------------------
void CGenome::MutateWeights(float mut_rate,
                            float prob_new_mut,
                            float MaxPertubation)
{
	for (size_t cGen=0; cGen<m_vecLinks.size(); ++cGen)
	{
		//muto il gene?
		if (RandFloat() < mut_rate)
		{
			//lo rimpiazzo o lo perturbo?
			if (RandFloat() < prob_new_mut)
			{
				//rimpiazza il peso sinaptico
				m_vecLinks[cGen].dWeight = RandomClamped();
			}

			else
			{
				//perturbo il peso sinaptico
				m_vecLinks[cGen].dWeight += RandomClamped() * MaxPertubation;                                            
			}
		}
	}

	return;
}

//-------------------- MutateActivationResponse---------------------------
//	itera sui geni dei nodi e ne perturba il parametro beta con probabilità mut_rate.
//------------------------------------------------------------------------
void CGenome::MutateActivationResponse(float mut_rate,
                                       float MaxPertubation)
{
  for (size_t cGen=0; cGen<m_vecNeurons.size(); ++cGen)
  {
	//muto il parametro beta?
    if (RandFloat() < mut_rate)
    {
      m_vecNeurons[cGen].dActivationResponse += RandomClamped() * MaxPertubation;
    }
  }
}


//------------------------- GetCompatibilityScore ------------------------
//
//  questa funzione ritorna la distanza, in termini di compatibilità,
//  fra il genoma a cui viene applicato e quello in argomentoi
//------------------------------------------------------------------------
float CGenome::GetCompatibilityScore(const CGenome &genome)
{
  //scorre il genoma dei link contanto i geni disjoint, excess e matching
  int	NumDisjoint = 0;
  int	NumExcess   = 0; 
  int	NumMatched  = 0;

  //tiene traccia della somma, dei moduli delle differenze, fra i pesi dei geni matching
  float	WeightDifference = 0.f;

  //tengono traccia della posizione in lettura per ognuno dei due genomi.
  int g1 = 0;
  int g2 = 0;


  //leggo fino a che non ho raggiunto la fine di entrambi i genomi
  while ( (g1 < m_vecLinks.size()) && (g2 < genome.m_vecLinks.size()) )
  {
    //si è raggiunta la fine di uno solo dei due genomi, quindi i geni rimanenti sono excess
	if ((g1 == m_vecLinks.size() - 1) ^ (g2 == genome.m_vecLinks.size() - 1))//^ è lo XOR
	{
		if (g1 == m_vecLinks.size() - 1)//è finito il genoma 1
		{
			++g2;
			++NumExcess;

			continue;// fa salatare la iterazione corrente del while
		}
		//vice versa (basterebbe un else ma così si sta sul sicuro)
		if (g2 == genome.m_vecLinks.size() - 1)
		{
			++g1;
			++NumExcess;

			continue;
		}
	}//end if excess
    
		
    //si ricavano i numeri d'innovazione dei geni correntemente in lettura
	//si sottolinea che i geni dei link nel genoma dei link sono ordinati
	//per numero d'innovazione crescente
    int id1 = m_vecLinks[g1].InnovationID;
    int id2 = genome.m_vecLinks[g2].InnovationID;

    //i numeri d'innovazione sono identici quindi i geni sono matching
    if (id1 == id2)
    {
      ++NumMatched;

      //carico la differenza dei pesi e incremento la posizinoe dei geni da leggere
      WeightDifference += fabs(m_vecLinks[g1++].dWeight - genome.m_vecLinks[g2++].dWeight);
    }

    //i numeri d'innovazione sono diversi quindi sono geni disjoint
    if (id1 < id2)
    {
      ++NumDisjoint;
      ++g1;
    }
    
    if (id1 > id2)
    {
      ++NumDisjoint;
      ++g2;
    }
		
  }//end while


  //viene ricavata la lunghezza del genoma più lungo
  int longest = genome.NumGenes();//identico a dire genome.m_vecLinks.size()
  
  if (NumGenes() > longest)
  {
    longest = NumGenes();
  }

  //pesi nel calcolo della somma che definisce la funzione di compatibilità.
  const float mDisjoint = 1.f;
  const float mExcess   = 1.f;
  const float mMatched  = 0.4f;
	
  //funzione di compatibilità
  float score = (mExcess * NumExcess/(float)longest) + 
                 (mDisjoint * NumDisjoint/(float)longest) + 
                 (mMatched * WeightDifference / NumMatched);

    
  return score;
}


//--------------------------- SortGenes ----------------------------------
//  ordina i geni per numero d'innovazione. modifica internamente il vector
//  m_vecLinks
//------------------------------------------------------------------------
void CGenome::SortGenes()
{
  sort(m_vecLinks.begin(), m_vecLinks.end());
}

//--------------------- CreateFromFile -----------------------------------
//
//  crea e restituisce un genoma, leggendolo da file
//------------------------------------------------------------------------
bool CGenome::CreateFromFile(string szFileName)
{
  ifstream in(szFileName);

  //controllo
  if (!in)
  {
#ifdef VIEWER
	  MessageBox(NULL, L"Cannot find genome file!", L"error", MB_OK);
#else
	  cout << "Cannot find genome file!" << endl;
#endif // VIEWER


    return false;
  }

  //ripulisce per sicurezza i vector del genoma in creazione
  m_vecNeurons.clear();
  m_vecLinks.clear();

  char buffer[100];
  int  iVal;

  //acquisisce il numero identificativo del genoma
  in >> buffer; in >> iVal;

  //acquisisce la profondità
  in >> buffer; in >> m_iNetDepth;

  //legge le info sui nodi e crea il genoma dei nodi
  int NumNeurons = 0;

  in >> buffer; in >> NumNeurons;

  m_vecNeurons.resize(NumNeurons);

  for (int n=0; n<NumNeurons; ++n)
  {
	  int    NeuronID, NeuronType;
	  bool   recurrent;
      float Activation, SplitX, SplitY;

    in >> buffer; in >> NeuronID;
    in >> buffer; in >> NeuronType;
	in >> buffer; in >> recurrent;
    in >> buffer; in >> Activation;
    in >> buffer; in >> SplitX;
    in >> buffer; in >> SplitY;

    //crea il gene del nodo e lo aggiunge al genoma 
    SNeuronGene gene((neuron_type)NeuronType,
                     NeuronID,
                     SplitY,
                     SplitX,
					 recurrent,
                     Activation);

    m_vecNeurons[n] = (gene);
    
  }//legge il nuovo nodo


  //legge le info sui link e crea il genoma dei link 
  int NumLinks = 0;
  in >> buffer; in >> NumLinks;

  int NextInnovationID = NumNeurons;

  m_vecLinks.resize(NumLinks);

  for (int l=0; l<NumLinks; ++l)
  {
    int    from, to, ID;
    bool   recurrent, enabled;
    float weight;

    in >> buffer; in >> ID;
    in >> buffer; in >> from;
    in >> buffer; in >> to;
    in >> buffer; in >> enabled;
    in >> buffer; in >> recurrent;
    in >> buffer; in >> weight;

	//crea il gene del link e lo aggiunge al genoma link
    SLinkGene LinkGene(from,
                       to,
                       enabled,
                       ID,//NextInnovationID++,
                       weight,
                       recurrent);


    m_vecLinks[l] = (LinkGene);


  }//next link

  return true;
}


//--------------------------------- Write --------------------------------
//
//  Scrive il genoma su streaming di output
//------------------------------------------------------------------------
bool CGenome::Write(ostream &stream)
{

	//controllo la validità dello streaming
	if (!stream) return false;

	vector<SNeuronGene>::iterator	curNeuron;
	vector<SLinkGene>::iterator	curLink;

	//id
	stream << "GenomeID: " << m_GenomeID << endl; 

	//profondità
	stream << "NetworkDepth: " << m_iNetDepth << endl;

	//nodi
	stream << "NumNeurons: " << m_vecNeurons.size();
  
	for (curNeuron = m_vecNeurons.begin(); curNeuron != m_vecNeurons.end(); ++curNeuron)
	{
    stream << *curNeuron;
	}

	//link
	stream << "\nNumLinks: " << m_vecLinks.size();
  
	for (curLink = m_vecLinks.begin(); curLink != m_vecLinks.end(); ++curLink)
	{
		stream << *curLink;
	}

	return true;

}
