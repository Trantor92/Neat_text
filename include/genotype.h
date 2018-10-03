#ifndef NEATGENOTYPE_H
#define NEATGENOTYPE_H

//-----------------------------------------------------------------------
//
//  Name: genotype.h
//
//	Desc: Definizione del genoma nell'implementazione dell'algoritmo NEAT.
//
//-----------------------------------------------------------------------

#include <vector>

#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\phenotype.h"
#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\utils.h"
#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\CInnovation.h"
#include "C:\Users\Adele\Desktop\C++_Code\Neat_text\include\genes.h"


using namespace std;


class Cga;
class CInnovation;


//------------------------------------------------------------------------
//
// Definizione della classe CGenome. Un genoma consiste essenzialmente di un
// vector di geni di link, di un vector di geni di nodi ed una fitness.
//------------------------------------------------------------------------
class CGenome 
{

private:
	
  //numero identificativo del genoma
  int                     m_GenomeID;

  //vector di tutti i nodi che compongono il genoma
  vector<SNeuronGene>     m_vecNeurons;

  //vector di tutti i nodi che compongono il genoma
  vector<SLinkGene>       m_vecLinks;

  //puntatore al fenotipo ad esso associato
  CNeuralNet*             m_pPhenotype;

  //profontità del fenotipo
  int                     m_iNetDepth;

  //fitness
  double                  m_dFitness;

  //fitness modificata, in accordo con la specie in cui il genoma viene posto
  double                  m_dAdjustedFitness;

  //numero di figli concessi a questo individuo
  double                  m_dAmountToSpawn;

  //traccia locale del numero di nodi di input e di output
  int                     m_iNumInputs,
                          m_iNumOutPuts;

  //tiene traccia della specie in cui questo genoma viene posto
  //(solo a scopo grafico nel riquadro delle informazioni generali)
  int                     m_iSpecies;



  //ritorna vero esiste già, nel genoma, un link fra questi due nodi
  bool    DuplicateLink(int NeuronIn, int NeuronOut);

  //dato il numero identificativo di un nodo, questa funzione restituisce il suo indice
  //nel vector dei nodi m_vecNeurons
  int     GetElementPos(int neuron_id);

  //ritorna vero se esiste già nel genoma un nodo avente questo numero identificativo
  //see CGenome::AddNeuron
  bool    AlreadyHaveThisNeuronID(const int ID);
 

public:

	//-------------- constructors --------------------------------------------
	CGenome(); 
  
	//questo cosntruttore crea un genoma minimo, ovvero nessun nodo hidden e
	//strato di input completamente connesso a quello di output
	CGenome(int id, int inputs, int outputs);

	//questo costruttore crea un genoma a partire dai vector dei nodi e dei link
	CGenome(int                 id,
			vector<SNeuronGene> neurons,
			vector<SLinkGene>   genes,
			int                 inputs,
			int                 outputs);
	 
	//costruttore per copia
	CGenome(const CGenome& g);

	~CGenome();


	//overload operatore di assegnazione '='
	CGenome& operator =(const CGenome& g);

	//crea il fenotipo a partire da genoma e ne calcola la profondità
	CNeuralNet*	    CreatePhenotype();

	CNeuralNet*		CreatePhenotype(int ID1, int ID2);//crea un fenotipo aggiungendo genoma un link fra i nodi specificati
	                                                  //dai numeri identificativi. controlla se questa aggiunta rende
	                                                  //la rete ciclica.


	//calcella il fenotipo
	void            DeletePhenotype();

	//inizializza i pesi a valori piccoli ]-1,1[. 
	void            InitializeWeights();

	//aggiunge un link al genoma con probabilità MutationRate
	void            AddLink(double      MutationRate,
                            CInnovation &innovation,
                            int         NumTrysToAddLink);

	//aggiunge un nodo al genoma con probabilità MutationRate
	void			AddNeuron(double      MutationRate,
							  CInnovation &innovation);

	//questa funziona muta i pesi
	void            MutateWeights(double  mut_rate,
                                  double  prob_new_mut,
                                  double  dMaxPertubation);

	//questa funzione muta il parametro beta della funzione di attivazione
	void            MutateActivationResponse(double mut_rate,
                                             double MaxPertubation);

	//funzione di compatibilità, necessaria alla speciazione. 
	//calcola la distanza fra questo e il genoma in argomento 
	double          GetCompatibilityScore(const CGenome &genome);

	//ordina i geni dei link per numero d'innovazione crescente
	void            SortGenes();

	//scrive su file il genoma
	bool			Write(ostream &file);

	//crea un genoma CGenome da file
	bool			CreateFromFile(const char* szFileName);

	//overload '<' usato per ordinare i genomi, da quello a più alta fitness a quello a più bassa.
	friend bool operator<(const CGenome& lhs, const CGenome& rhs)
	{
		return (lhs.m_dFitness > rhs.m_dFitness);//invertendo così loperatore < si ottiene che in posizione 0
		                                         //si ha la massima fitness
	}


	//funzioni che calcolano la profondita del phenotipo
	int CalculateDepthPhenotype( vector<SNeuron*> vecNeurons);

	int CalculateDepthPhenotype_rec(int nd, vector<SNeuron*> vecNeurons, int it_rec);//funzione implementata all'interno di
																				     //CalculateDepthPhenotype() in modo
																					 //ricorsivo

	
	//---------------------------------accessor methods-------------------------------
	int	    ID()const{return m_GenomeID;}
	void    SetID(const int val){m_GenomeID = val;}

	int     Depth(){return m_iNetDepth;}
	void    SetDepth(int val){m_iNetDepth = val;}

	int     NumGenes()const{return m_vecLinks.size();}
	int     NumNeurons()const{return m_vecNeurons.size();}
	int     NumInputs()const{return m_iNumInputs;}
	int     NumOutputs()const{return m_iNumOutPuts;}

	double  AmountToSpawn()const{return m_dAmountToSpawn;}
	void    SetAmountToSpawn(double num){m_dAmountToSpawn = num;}
  
	void    SetFitness(const double num){m_dFitness = num;}
	void    SetAdjFitness(const double num){m_dAdjustedFitness = num;}
	double  Fitness()const{return m_dFitness;}
	double  GetAdjFitness()const{return m_dAdjustedFitness;}

	int     GetSpecies()const{return m_iSpecies;}
	void    SetSpecies(int spc){m_iSpecies = spc;}

	CNeuralNet* Phenotype(){return m_pPhenotype;}
  
	double  SplitY(const int val)const{return m_vecNeurons[val].dSplitY;}

	vector<SLinkGene>	  LinkGenes()const{return m_vecLinks;}
	vector<SNeuronGene> NeuronGenes()const{return m_vecNeurons;}

	 vector<SLinkGene>::iterator StartOfGenes(){return m_vecLinks.begin();}
	 vector<SLinkGene>::iterator EndOfGenes(){return m_vecLinks.end();}
};

#endif