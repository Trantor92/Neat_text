#include "phenotype.h"

//------------------------------------ Sigmoid function ------------------------
//  funzione di attivazione del neurone
//----------------------------------------------------------------------------
double CNeuralNet::Sigmoid(float netinput, float response)
{
	return ( 1 / ( 1 + exp(-netinput / response)));
}


//--------------------------------- constructor --------------------------
//
//------------------------------------------------------------------------
CNeuralNet::CNeuralNet(vector<SNeuron*> neurons,
                       int              depth):m_vecpNeurons(neurons),
                                               m_iDepth(depth)
{}


//--------------------------------- destructor ---------------------------
//
//------------------------------------------------------------------------
CNeuralNet::~CNeuralNet()
{
  //pulisce il vector che contiene i neuroni
  for (int i=0; i<m_vecpNeurons.size(); ++i)
  {
    if (m_vecpNeurons[i])
    {
      delete m_vecpNeurons[i];

      m_vecpNeurons[i] = NULL;
    }
  }
}

//----------------------------------Update--------------------------------
//	prende un array di double come input e lo asscoia alle attivazioni dei
//  neuroni di input. la funzione quindi agisce a step attraversando la tutta rete.
//  ad ogni step viene ricalcolato l'input netto di ogni neurone. Dopo un numero
//  di step pari alla profondità si ottiene l'attivazione finale dei neuroni di output (snapshot)
//
//	ritorna un vector delle attivazioni dei neuroni di output.
//------------------------------------------------------------------------
vector<double> CNeuralNet::Update(const vector<double> &inputs,
                                  const run_type        type)
{
  //crea un vector in cui mettere le attivazioni degli output
  vector<double>	outputs;

  //Se si è scelto snapshot, allora si itera percorrendo tutta la rete, 
  //un numero di volte pari alla profondità. Se si è scelto active si ritorna il 
  //valore di attivazione degli output dopo solo una passata.
  int FlushCount = 0;//numero delle passate attraverso la rete
  
  if (type == snapshot)
  {
    FlushCount = m_iDepth;
  }
  else
  {
    FlushCount = 1;
  }



  //si percorre l'intera rete il numero prefissato di volte
  for (int i=0; i<FlushCount; ++i)
  {
    //per sicurezza si ripulisce il vector delle attivazioni dei neuroni di output
    outputs.clear();
   
    //indice del neurone che si sta considerando
    int cNeuron = 0;

    //prima di tutto si setta come valore di attivazione dei neuroni di input
	//il valore passato alla funzione come inputs
    while (m_vecpNeurons[cNeuron]->NeuronType == input)
    {
      m_vecpNeurons[cNeuron]->dOutput = inputs[cNeuron];

      ++cNeuron;
    }


    //si setta l'attivazione del neurone di bias ad 1
    m_vecpNeurons[cNeuron++]->dOutput = 1;

    //si visita l'intera la rete un neurone alla volta
    while (cNeuron < m_vecpNeurons.size())
    {
      //conterrà l'input netto del neurone
      double sum = 0;

      //calcolo l'input netto del neurone. sommo le attivazioni di tutti i neuroni aventi link uscenti
	  //che entrano nel neurone in esame, moltiplicadole per il peso sinaptico della connessione
      for (int lnk=0; lnk<m_vecpNeurons[cNeuron]->vecLinksIn.size(); ++lnk)
      {
        //estraggo il peso del link entrante
        double Weight = m_vecpNeurons[cNeuron]->vecLinksIn[lnk].dWeight;

        //estraggo l'attivazione del neurone da cui esce questo link
        double NeuronOutput =
        m_vecpNeurons[cNeuron]->vecLinksIn[lnk].pIn->dOutput;

        //calcolo l'input netto
        sum += Weight * NeuronOutput;
      }

      //ora calcolo e assegno l'attivazione del neurone poichè ne conosco l'input netto
      m_vecpNeurons[cNeuron]->dOutput = 
      Sigmoid(sum, m_vecpNeurons[cNeuron]->dActivationResponse);
	

	  //se il neurone è di output allora salvo l'attivazione, che dopo la devo restituire
      if (m_vecpNeurons[cNeuron]->NeuronType == output)
      {
        outputs.push_back(m_vecpNeurons[cNeuron]->dOutput);
      }

      //next neuron
      ++cNeuron;
    }

  }//next iteration through the network


  //E' necessario ripulire le attivazioni dei neuroni nel caso in cui si sia scelta 
  //l'opzione snapshot in modo tale che quando viene richiamata non si risenta del calcolo precedente.
  //Se si sceglie active questa cosa non va fatta e ogni iterazioni gli input nuovi lavorano insieme alle
  //attivazioni gli input precedenti hanno causato.
  if (type == snapshot)
  {
    for (int n=0; n<m_vecpNeurons.size(); ++n)
    {
      m_vecpNeurons[n]->dOutput = 0;
    }
  }


  //ritorna le attivazioni dei neuroni di output
  return outputs;
}


//----------------------------- TidyXSplits -----------------------------
//
//  Questa funzione permette di disegnare nel modo corretto le reti neurali
//  prevenendo che i neuroni si sovrappongano in fase grafica
//-----------------------------------------------------------------------
void TidyXSplits(vector<SNeuron*> &neurons)
{
  //immagazzina gli indici dei neuroni che si trovano sul medesimo layer 
  vector<int>    SameLevelNeurons;

  //immagazzina i layer già controllati
  vector<double> DepthsChecked;


  //per ogni neurone trova tutti i neuroni che sono nel medesimo layer (dSplitY)
  for (int n=0; n<neurons.size(); ++n)
  {
    double ThisDepth = neurons[n]->dSplitY;

    //permette di controllare se questo layer è già stato messo a posto
    bool bAlreadyChecked = false;

    for (int i=0; i<DepthsChecked.size(); ++i)
    {
      if (DepthsChecked[i] == ThisDepth)
      {
        bAlreadyChecked = true;

        break;
      }
    }

    //viene aggiunta questo layer a quelle controllate, poichè
	//nel caso non sia stato fatto prima lo si sta per fare.
    DepthsChecked.push_back(ThisDepth);

    //se questo layer non è stato già controllato
    if (!bAlreadyChecked)
    {
      //pulisce il vector dei neuroni nel layer che si sta controllando e si aggiunge
	  //il neurone n
	  SameLevelNeurons.clear();
      SameLevelNeurons.push_back(n);
      
      //si trovano tutti i neuroni nel layer in esame
      for (int i=n+1; i<neurons.size(); ++i)
      {
        if (neurons[i]->dSplitY == ThisDepth)
        {
          //si aggiunge l'indice del neurone trovato
          SameLevelNeurons.push_back(i);
        }
      }

      //si calcola lo spaziamento che devono avere i neuroni nel layer
      double slice = 1.0/(SameLevelNeurons.size()+1);
  

      //si assegna la posizione nel layer ad ogni neurone che vi appartiene
      for (int i=0; i<SameLevelNeurons.size(); ++i)
      {
        int idx = SameLevelNeurons[i];

        neurons[idx]->dSplitX = (i+1) * slice;
      }
    }

  }//next neuron to check

}


//----------------------------- DrawNet ----------------------------------
//
//  crea la rappresentazione grafica della rete neurale sulla surface
//------------------------------------------------------------------------
void CNeuralNet::DrawNet(HDC &surface, int Left, int Right, int Top, int Bottom)
{
  //spessore dei bordi
  const int border = 10;
    
  //massimo spessore delle linee (connessioni)
  const int MaxThickness = 5;

  //si mettono a posto le posizioni dei neuroni
  TidyXSplits(m_vecpNeurons);

  //si assegnano le coordinate dei neuroni in funzione di quanto
  //è grande la surface grafica
  int spanX = Right - Left;
  int spanY = Top - Bottom - (2*border);

  for (int cNeuron=0; cNeuron<m_vecpNeurons.size(); ++cNeuron)
  {
    m_vecpNeurons[cNeuron]->iPosX = Left + spanX*m_vecpNeurons[cNeuron]->dSplitX;
    m_vecpNeurons[cNeuron]->iPosY = (Top - border) - (spanY * m_vecpNeurons[cNeuron]->dSplitY);
  }

  //si creano gli oggeti che serviranno a disegnare (pen and brush)
  HPEN GreyPen  = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
  HPEN RedPen   = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
  HPEN GreenPen = CreatePen(PS_SOLID, 1, RGB(0, 200, 0));
  HPEN OldPen   = NULL;

  HBRUSH RedBrush = CreateSolidBrush(RGB(255, 0, 0));
  HBRUSH BlueBrush = CreateSolidBrush(RGB(0, 0, 255));
  HBRUSH GreyBrush = CreateSolidBrush(RGB(200, 200, 200));
  HBRUSH GreenBrush = CreateSolidBrush(RGB(0, 255, 0));
  HBRUSH DarkGreyBrush = CreateSolidBrush(RGB(100, 100, 100));
  HBRUSH BlackBrush = CreateSolidBrush(RGB(0, 0, 0));

  HBRUSH OldBrush = NULL;

  OldPen =   (HPEN)  SelectObject(surface, RedPen);
  OldBrush = (HBRUSH)SelectObject(surface, GetStockObject(HOLLOW_BRUSH));


  //raggio dei neuroni (funzione della largezza della surface)
  int radNeuron = spanX/60;

  //ora che si hanno le coordinate di ogni neurone della reteno si può passare a disegnare.
  //Prima di tutto si itera sui neuroni e si disegnano i link.
  for (int cNeuron=0; cNeuron<m_vecpNeurons.size(); ++cNeuron)
  {
    //si prende la posizione del neurone come punto di partenza
	//della connessione
    int StartX = m_vecpNeurons[cNeuron]->iPosX;
    int StartY = m_vecpNeurons[cNeuron]->iPosY;

    //il nodo in esame è bias? nel caso va disegnato in verde
	bool bBias = false;

    if (m_vecpNeurons[cNeuron]->NeuronType == bias)
    {
      bBias = true;
    }

    //ora si itera sui link uscenti, tenendo traccia della posizione
	//de neurone che questo link connette
    for (int cLnk=0; cLnk<m_vecpNeurons[cNeuron]->vecLinksOut.size(); ++ cLnk)
    {
      int EndX = m_vecpNeurons[cNeuron]->vecLinksOut[cLnk].pOut->iPosX;
      int EndY = m_vecpNeurons[cNeuron]->vecLinksOut[cLnk].pOut->iPosY;

      //se il link è diretto da un layer inferiore ad uno superiore (forward) e non proviene dal bias
      if( (!m_vecpNeurons[cNeuron]->vecLinksOut[cLnk].bRecurrent) && !bBias)
      {
		//spessore del link proporzionale al peso sinaptico
        int thickness = (int)(fabs(m_vecpNeurons[cNeuron]->vecLinksOut[cLnk].dWeight));        
        
        Clamp(thickness, 0, MaxThickness);

        HPEN Pen;

        //crea una penna gialla per i pesi inibitori (<0)
        if (m_vecpNeurons[cNeuron]->vecLinksOut[cLnk].dWeight <= 0)
        {
          Pen  = CreatePen(PS_SOLID, thickness, RGB(240, 230, 170));
        }

		//crea una penna grigia per i pesi eccitatori (>0)
        else
        {
          Pen  = CreatePen(PS_SOLID, thickness, RGB(200, 200, 200));
        }
        
        HPEN tempPen = (HPEN)SelectObject(surface, Pen);
        
        //disegna il link utilizzando la penna creata
        MoveToEx(surface, StartX, StartY, NULL);
        LineTo(surface, EndX, EndY);

        SelectObject(surface, tempPen);

        DeleteObject(Pen);
      }

	  //se il link è diretto da un layer inferiore ad uno superiore (forward) e proviene dal bias
      else if( (!m_vecpNeurons[cNeuron]->vecLinksOut[cLnk].bRecurrent) && bBias)
      {
        SelectObject(surface, GreenPen);
        
        //disegna il link utilizzando la penna verde
        MoveToEx(surface, StartX, StartY, NULL);
        LineTo(surface, EndX, EndY);
      }

      //se il link è diretto da un layer superiore ad uno inferiore (backward)
      else
      {
		int thickness = (int)(fabs(m_vecpNeurons[cNeuron]->vecLinksOut[cLnk].dWeight));

		Clamp(thickness, 0, MaxThickness);

		HPEN Pen;

		//crea una penna rossa per i pesi inibitori (<0)
		if (m_vecpNeurons[cNeuron]->vecLinksOut[cLnk].dWeight <= 0)
		{
		  Pen = CreatePen(PS_SOLID, thickness, RGB(255, 0, 0));

		}

		//crea una penna blu per i pesi eccitatori (>0)
		else
		{
	      Pen = CreatePen(PS_SOLID, thickness, RGB(0, 0, 255));
		}


		HPEN tempPen = (HPEN)SelectObject(surface, Pen);

		//disegna il link utilizzando la penna creata
		MoveToEx(surface, StartX, StartY, NULL);
		LineTo(surface, EndX, EndY);

		SelectObject(surface, tempPen);

		DeleteObject(Pen);   
      }
    }
  }//ho disegnato tutti link

  //ora si disegnano i neuroni
  SelectObject(surface, RedBrush);
  SelectObject(surface, GetStockObject(BLACK_PEN));

  for (int cNeuron=0; cNeuron<m_vecpNeurons.size(); ++cNeuron)
  {
	  //quote in grigio
	  if (cNeuron < 7)
	  {
		  SelectObject(surface, GreyBrush);
	  }
	  //statistiche squadra in casa in blu
	  else if (cNeuron < 13)
	  {
		  SelectObject(surface, BlueBrush);
	  }
	  //statistiche squadra fuori casa in rosso
	  else if (cNeuron < 19)
	  {
		  SelectObject(surface, RedBrush);
	  }
	  //bias in verde
	  else if (cNeuron < 20)
	  {
		  SelectObject(surface, GreenBrush);
	  }
	  //gli output in grigio scuro
	  else if(cNeuron < 23)
	  {
		  SelectObject(surface, DarkGreyBrush);
	  }
	  //i neuroni hidden in nero
	  else
	  {
		  SelectObject(surface, BlackBrush);
	  }

    int x = m_vecpNeurons[cNeuron]->iPosX; 
    int y = m_vecpNeurons[cNeuron]->iPosY;

    //disegna il neurone come un cerchio colorato
    Ellipse(surface, x-radNeuron, y-radNeuron, x+radNeuron, y+radNeuron); 
  }

  //pulizia degli strumenti grafici
  SelectObject(surface, OldPen);
  SelectObject(surface, OldBrush);
  

  DeleteObject(RedPen);
  DeleteObject(GreyPen);
  DeleteObject(GreenPen);
  DeleteObject(OldPen);

  DeleteObject(RedBrush);
  DeleteObject(GreyBrush);
  DeleteObject(BlueBrush);
  DeleteObject(GreenBrush);
  DeleteObject(DarkGreyBrush);
  DeleteObject(BlackBrush);

  DeleteObject(OldBrush);
}




