#include "CSpecies.h"


//----------------------- constructor ------------------------------------
//
//  crea una nuova specie con il numero identificativo passato come argomento.
//  Uns copia del genoma inizializzante è tenuta in m_Leader ed il suo puntatore viene
//  aggiunto al vector dei membri.
//------------------------------------------------------------------------
CSpecies::CSpecies(CGenome  &FirstOrg,
                   int      SpeciesID):m_iSpeciesID(SpeciesID),
                                       m_dBestFitness(FirstOrg.Fitness()),
                                       m_iGensNoImprovement(0),
                                       m_iAge(0),
                                       m_Leader(FirstOrg),
                                       m_dSpawnsRqd(0)
                                     
{
  m_vecMembers.push_back(&FirstOrg);
  
  m_Leader = FirstOrg;
}

//------------------------ AddMember -------------------------------------
//
//  questa funzione aggiunge un nuovo membri alla specie e aggiorna le variabili
//  ad essa relative di conseguenza. la flag is_preliminar serve per il corretto conteggio
//  di m_iGensNoImprovement, il suo utilizzo è in Cga::Speciate() dove si esegue una speciazione
//  preliminare al fine di un corretto funzionamento di Cga::ResetAndKill()
//------------------------------------------------------------------------
void CSpecies::AddMember(CGenome &NewMember, bool is_preliminar)
{
 
	//la fitness del nuovo membri è migliore della miglior fitness attuale della specie?
	if (NewMember.Fitness() > m_dBestFitness)
	{
		m_dBestFitness = NewMember.Fitness();

		if (!is_preliminar)
		{
			m_iGensNoImprovement = 0;
		}
		else
		{
			m_iGensNoImprovement = -1;
		}
		m_Leader = NewMember;
	}

	m_vecMembers.push_back(&NewMember);
}

//-------------------------- Purge ---------------------------------------
//
//  questa funzione cancella i membri delle specie (mantenendo l'informazione del genoma
//  Leader) e aggiorna le variabili interne della specie per tenere conto che è trascorsa una
//  generazione.
//------------------------------------------------------------------------
void CSpecies::Purge()
{
  m_vecMembers.clear();

  //update age etc
  ++m_iAge;

  ++m_iGensNoImprovement;

  m_dSpawnsRqd = 0;
}

//questa funzione serve per il corretto funzionamento di Cga::Speciate()
void CSpecies::Purge_preliminar()
{
	m_vecMembers.clear();
}

//--------------------------- AdjustFitness ------------------------------
//
//  questa funzione costruisce la fitness modificata di ogni membro della specie
//  esaminando la fitness dei membri e l'età della specie. Si penalizzano le specie vecchie
//  e si premiano quelle giovani.
//  Quindi si esegue la condivisione della fitness all'interno della specie che servirà per calcolare il numero di
//  figli che essa può generare. Questo assicura che nessuna specie diventi troppo grande.
//------------------------------------------------------------------------
void CSpecies::AdjustFitnesses()
{

  for (int gen=0; gen<m_vecMembers.size(); ++gen)
  {
    double fitness = m_vecMembers[gen]->Fitness();

    //Si aumenta la fitness degli individui appartenenti a specie giovani
    if (m_iAge < CParams::iYoungBonusAgeThreshhold)
    {
      fitness *= CParams::dYoungFitnessBonus;
    }

    //Si penalizza la fitness degli individui appartenenti a specie vecchie
    if (m_iAge > CParams::iOldAgeThreshold)
    {
      fitness *= CParams::dOldAgePenalty;
    }

    //si esegue la condivisione della fitness, calcolo della fitness modificata
    double AdjustedFitness = fitness/m_vecMembers.size();

    m_vecMembers[gen]->SetAdjFitness(AdjustedFitness);
  }
}

//------------------------ CalculateSpawnAmount --------------------------
//
//  calcola il numero di figli concessi alla specie.
//  Semplicemente somma e arrotonda il numero di figli concessi ai membri.
//------------------------------------------------------------------------
void CSpecies::CalculateSpawnAmount()
{
	for (int gen=0; gen<m_vecMembers.size(); ++gen)
	{
		m_dSpawnsRqd += m_vecMembers[gen]->AmountToSpawn();//m_dSpawnsRqd viene settato a zero in Purge()
														   //dentro Cga::ResetAndKill()

	}
}

//------------------------ Spawn -----------------------------------------
//
//  Ritorna un genoma casuale della specie, fra i migliori.
//  Il parametro CParams::dSurvivalRate permette la scelta di due differenti distribuzioni di probabilità
//  per l'estrazione dell'indice del genoma in m_vecMembers. 
//  >0: distribuzione uniforme a gradino.
//  =0: distribuzione lineare.
//
//  L'applicazione di queste distribuzione nell'estrazione dei migliori individui
//  a senso solo se prima i genomi sono stati messi in ordine di fitness
//  decrescente i membri della specie.
//------------------------------------------------------------------------
CGenome CSpecies::Spawn()
{
  //conterrà il genoma estratto
  CGenome baby;

  int Num_Members = m_vecMembers.size();
  
  //quale distribuzione di probabilità devo applicare
  if (CParams::dSurvivalRate != 0)//a gradino
  {
	  if (Num_Members == 1)//nel caso in cui la specie è formata da un solo individuo, a prescindere dal
	  {                    //valore CParams::dSurvivalRate viene restituito l'univo membro

		  baby = *m_vecMembers[0];
	  }
	  else
	  {
		  //indice che rappresenta il limite del gradino
		  int MaxIndexSize = (int) (CParams::dSurvivalRate * Num_Members)-1;

		  if (MaxIndexSize > Num_Members - 1)//facendo così generalizzo a qualsiasi CParams::dSurvivalRate
		  MaxIndexSize = Num_Members - 1;

		  //estraggo l'indice con distribuzione uniforma nel range definito dal gradino
	      int TheOne = RandInt(0, MaxIndexSize);

	      baby = *m_vecMembers[TheOne];
	  }
  }
  else //lineare
  {
	  //distribuzione lineare decrescente tale P(Num_Members)=0
	  int TheOne = (int)(m_vecMembers.size()*fabs(RandomClamped()));

	  baby = *m_vecMembers[TheOne];
  }


  return baby;
}


