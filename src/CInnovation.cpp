#include "CInnovation.h"

//---------------------------------- constructor -------------------------
//
//  dati i genomi iniziali, questo costruttore genera la corrispettiva lista
//  iniziale delle innovazioni.
//------------------------------------------------------------------------
CInnovation::CInnovation(vector<SLinkGene>   start_genes,
                         vector<SNeuronGene> start_neurons)
{
	m_NextNeuronID			= 0;
	m_NextInnovationNum		= 0;

	//aggiunge le innvazioni dei nodi
	for (int nd=0; nd<start_neurons.size(); ++nd)
	{
		 m_vecInnovs.push_back(SInnovation(start_neurons[nd],
							   m_NextInnovationNum++,
							   m_NextNeuronID++));
	 }

	 //aggiunge le innovazioni dei link
	 for (int cGen = 0; cGen<start_genes.size(); ++cGen) 
		{
			SInnovation NewInnov(start_genes[cGen].FromNeuron,
								 start_genes[cGen].ToNeuron,
								 new_link,
								 m_NextInnovationNum);

			m_vecInnovs.push_back(NewInnov);

			++m_NextInnovationNum;
		}
}

//---------------------------CheckInnovation------------------------------
//
//	controlla se innovazione in argomento è gia presente nella lista. 
//  se così ritorna il numero d'innovazione corrispondente, altrimenti -1.
//------------------------------------------------------------------------
int CInnovation::CheckInnovation(int in, int out, innov_type type)
{
	//itera su tutte le innovazioni presenti nella lista cerca una corrispondenza
	//esatta dei tre argomenti.
	for (int inv=0; inv<m_vecInnovs.size(); ++inv)
	{
		if ((m_vecInnovs[inv].NeuronIn == in)   && 
			(m_vecInnovs[inv].NeuronOut == out) && 
			(m_vecInnovs[inv].InnovationType == type))
		{			
			//trovata una corrispondenza quindi ne ritorna il numero d'innovazione
			return m_vecInnovs[inv].InnovationID;
		}
	}
	
	//se non si è trovata una corrispondenza allora ritorna -1
	return -1;
}

//--------------------------CreateNewInnovation---------------------------
//
//	crea una nuova innovazione, la aggiunge alla lista e ne restituisce
//  il numero d'innovazione
//------------------------------------------------------------------------
int CInnovation::CreateNewInnovation(int in, int out, innov_type type)
{
	SInnovation new_innov(in, out, type, m_NextInnovationNum);
	
	if (type == new_neuron)
	{
		new_innov.NeuronID = m_NextNeuronID;
		
		++m_NextNeuronID;
	}

	m_vecInnovs.push_back(new_innov);

	++m_NextInnovationNum;
	
	return (m_NextNeuronID-1);
}

//------------------------------------------------------------------------
//
//  come sopra ma questa aggiunge anche la posizione del nodo 
//  nel caso in cui sia una innvazione di nuovo nodo.
//------------------------------------------------------------------------
int CInnovation::CreateNewInnovation(int          from,
                                     int          to,
                                     innov_type   InnovType,
                                     neuron_type  NeuronType,
                                     double       x,
                                     double       y)
{ 

	SInnovation new_innov(from, to, InnovType, m_NextInnovationNum, NeuronType, x, y);
	
	if (InnovType == new_neuron)
	{
		new_innov.NeuronID = m_NextNeuronID;
		
		++m_NextNeuronID;
	}

	m_vecInnovs.push_back(new_innov);

	++m_NextInnovationNum;
	
	return (m_NextNeuronID-1);

}


//------------------------------- CreateNeuronFromID -----------------------
//
//  datone il numero d'identificazione di un nodo, questa funzione
//  lo restituisce leggendolo dalla lista 
//------------------------------------------------------------------------
SNeuronGene CInnovation::CreateNeuronFromID(int NeuronID)
{
  SNeuronGene temp(hidden,0,0,0);

  for (int inv=0; inv<m_vecInnovs.size(); ++inv)
  {
	if (m_vecInnovs[inv].NeuronID == NeuronID)
    {
      temp.NeuronType = m_vecInnovs[inv].NeuronType;
      temp.iID      = m_vecInnovs[inv].NeuronID;
      temp.dSplitY  = m_vecInnovs[inv].dSplitY;
      temp.dSplitX  = m_vecInnovs[inv].dSplitX;

      return temp;
    }
  }

  return temp;
}


//--------------------------------- Write --------------------------------
//
//  scrive la lista dellle innovazioni in uno stream di output
//------------------------------------------------------------------------ 
bool CInnovation::Write(const char* szFileName, const int gen)
{
	 ofstream stream(szFileName);
  
	if (!stream) return false;//error

	vector<SInnovation>::iterator curInnov;

	stream << "\n\n--------------------------------------- Generation: " << gen 
		   <<    " -------------------------------g-------\n";
	
	for (curInnov = m_vecInnovs.begin(); curInnov != m_vecInnovs.end(); ++curInnov)
	{
		stream << "\nInnovationID: " << setw(3) << curInnov->InnovationID
			   << "   in: " << setw(3) << curInnov->NeuronIn
               << "  out: " << setw(3) << curInnov->NeuronOut <<  "   NeuronID: " 
			   << setw(3) << curInnov->NeuronID << "   Type: ";


		if (curInnov->InnovationType == new_link)
		{
    		stream << "new_link";
		}
		else
		{
			stream << "NewNeuron";
		}

		stream<<"   NeuronType: " << curInnov->NeuronType;
	
	}//next innovation
  
	stream<< "\n\n";
  
	return true;
}


