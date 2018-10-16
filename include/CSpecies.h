#ifndef CSPECIES_H
#define CSPECIES_H

//-----------------------------------------------------------------------
//
//  Name: CSpecies.h
//
//	Desc: Classe che implementa il concetto di speciazione con condivisione
//  della fitness
//
//-----------------------------------------------------------------------

#include <vector>
#include <math.h>
#include <iomanip>
#include <iostream>

#include "genotype.h"

using namespace std;


//------------------------------------------------------------------------
//
//  classe per gestire i genomi all'interno di una specie
//------------------------------------------------------------------------
class CSpecies
{

private:

  //copia locale del miglior membro della specie e riferimento per la funzione
  //di compatibilità
  CGenome           m_Leader;
  
  //vector di puntatori ai genomi dei membri di questa specie
  vector<CGenome*>  m_vecMembers;
  
  //numero idenficativo  della specie
  int               m_iSpeciesID;
	
  //migliori fitness, mai raggiunta, dalla specie
  float            m_dBestFitness;
  
  //numero di generazioni dall'ultimo miglioramento di fitness (aumento di m_dBestFitness).
  //Questa informazione serve per eliminare le specie che non migliorano più
  int               m_iGensNoImprovement;

  //età della specie
  int               m_iAge;
  
  //numero di figli concessi alla specie in fase di riproduzione
  float            m_dSpawnsRqd;


public:

  //-------- constructor ------------------------------------
  CSpecies(CGenome &FirstOrg, int SpeciesID);

  //questa funzione calcola la fitness modificata di ogni individuo 
  //nella specie, attraverso la condivisione all'interno della nicchi evolutiva.
  //premia le specie giovani e penalizza le specie vecchie
  void    AdjustFitnesses();
  
  //aggiunge un nuono individuo alla specie
  void    AddMember(CGenome& new_org, bool is_preliminar = false);

  //funzioni necessarie al corretto funzionamento della classificazione in specie
  void    Purge();
  void Purge_specializzata();
  void	  Purge_preliminar();

  //calcola quanti figli sono concessi a questa specie. funzione delle fitness modificate
  //dei membri
  void    CalculateSpawnAmount();

  //estrae un individuo dalla specie per partecipare alla riproduzione. l'estrazione avviene
  //in modo casuale secondo una distribuzione di probabilità specificata dal parametro
  //CParams::dSurvivalRate
  CGenome Spawn();




  
  //overload dell'operatore '<' in modo tale da ordinare le specie in base
  //alla fitness dell'individuo migliore
  friend bool operator<(const CSpecies &lhs, const CSpecies &rhs)
  {
    return lhs.m_dBestFitness > rhs.m_dBestFitness;
  }

  //overload << per lo stream di output della specie
  friend ostream& operator<<(ostream &os, const CSpecies &rhs)
  {
    os << "\n\n------------------------------ Species " << rhs.m_iSpeciesID << " --------------------------------"
       << "\nAge: " << rhs.m_iAge << "  Gens No improvement: " << rhs.m_iGensNoImprovement
       << " Leader ID: " << rhs.m_Leader.ID();

    os << "\n\nBestFitness: " << rhs.m_dBestFitness << "\n\nMembers\n";

    for (int i=0; i<rhs.m_vecMembers.size(); ++i)
    {
      os << "\nGenomeID: " << setw(3) << rhs.m_vecMembers[i]->ID()
         << "  Fitness: "  << setw(3) << rhs.m_vecMembers[i]->Fitness()
         << "  Adjusted: " << setw(3) << rhs.m_vecMembers[i]->GetAdjFitness()
         << "  Spawn: "    << setw(3) << rhs.m_vecMembers[i]->AmountToSpawn();

    }

    os << "\n\nAmount to spawn: " << rhs.m_dSpawnsRqd;

    return os;
  }


  //------------ accessor methods ----------------------------------------- 
  CGenome			Leader()const { return m_Leader; }

  void				SetLeader(CGenome new_leader) { m_Leader = new_leader; }
  void				SetLeader() { m_Leader = *m_vecMembers[0]; }

  float			 SpeciesLeaderFitness()const { return m_Leader.Fitness(); }


  vector<CGenome*>	GenomesOfMembers()const { return m_vecMembers; }
  int				NumMembers()const { return m_vecMembers.size(); }


  int				ID()const { return m_iSpeciesID; }

  float			BestFitness()const { return m_dBestFitness; }

  int				Age()const { return m_iAge; }

  float			NumToSpawn()const { return m_dSpawnsRqd; }


  int				GensNoImprovement()const { return m_iGensNoImprovement; }
  void				GensNoImprovement(int no_improvement) { m_iGensNoImprovement = no_improvement; }

};

#endif