#ifndef GENES_H
#define GENES_H

//-----------------------------------------------------------------------
//
//  Name: genes.h
//
//	Desc: definizioni dei geni per i nodi e per i link utilizzati nell'approccio NEAT.
//        Queste strutture sono utlizzate per costruire il genoma.
//
//-----------------------------------------------------------------------


//------------------------------------------------------------------------
//  enumerazioni delle tipologie di nodo presenti
//------------------------------------------------------------------------
enum neuron_type
{
  input,
  hidden,
  output,
  bias,
  none
};


//------------------------------------------------------------------------
//  Questa struttura definisce il gene di un nodo
//------------------------------------------------------------------------
struct SNeuronGene
{  
	//numero identificativo del nodo
	int         iID;

	//il suo tipo
	neuron_type NeuronType;

	//is it recurrent
	bool        bRecurrent;

	//parametro beta della funzione di attivazione, setta la pendenza
	float		dActivationResponse;

	//posizione nella griglia grafica; serve per disegnarlo.
	float		dSplitY, dSplitX;


	//------ constructor ----------------

	SNeuronGene() {};

	SNeuronGene(neuron_type type,
              int         id,
              float      splitY,
              float      splitX,
			  bool       r = false,
              float      act = 1.f):iID(id),
                                   NeuronType(type),
                                   dSplitY(splitY),
                                   dSplitX(splitX),
								   bRecurrent(r),
                                   dActivationResponse(act){}

	//overload << for streaming
	friend ostream& operator <<(ostream &os, const SNeuronGene &rhs)
	{
	  os << "\nNeuron: " << rhs.iID << "  Type: " << rhs.NeuronType
		 << "  Recurrent: " << rhs.bRecurrent << "  Activation: " << rhs.dActivationResponse
		 << "  SplitX: " << rhs.dSplitX << "  SplitY: " << rhs.dSplitY;
    
		 return os;
	} 
};


//------------------------------------------------------------------------
//  Questa struttura definisce il gene di un link
//------------------------------------------------------------------------
struct SLinkGene
{
	//numeri identificativi dei due nodi che il link connette	
	int     FromNeuron,
	        ToNeuron;
	
	//il suo peso sinaptico
	float	dWeight;

	//flag che indica se questo gene è abilitato o soppresso
	bool    bEnabled;
	
	//il suo numero d'innovazione
	int     InnovationID;

	//il link è ricorrente nel senso che punta un layer inferiore?
	bool bRecurrent;

	//------------ constructors -----------------------
	SLinkGene(){}
  
	SLinkGene(int    in,
            int    out,
            bool   enable,
            int    tag,
            float weight,
		    bool rec = false):bEnabled(enable),
					  InnovationID(tag),
                      FromNeuron(in),
                      ToNeuron(out),
                      dWeight(weight),
		              bRecurrent(rec){}



	//overload '<' utlizzato nell'ordinamento. si utilizza il numero di innovazione
	//per mettere in ordine i geni dei link
	friend bool operator<(const SLinkGene& lhs, const SLinkGene& rhs)
	{
		return (lhs.InnovationID < rhs.InnovationID);
	}


	//overload << for streaming
	 friend ostream& operator <<(ostream &os, const SLinkGene &rhs)
	{
	 os << "\nInnovID: " << rhs.InnovationID << "  From: " << rhs.FromNeuron
		<< "  To: " << rhs.ToNeuron << "  Enabled: " << rhs.bEnabled 
        << "  Recurrent: " << rhs.bRecurrent << "  Weight: " << rhs.dWeight;

		return os;
	}
};
                                         
#endif 