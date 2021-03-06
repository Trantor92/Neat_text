// NEAT_text.cpp: definisce il punto di ingresso dell'applicazione console.
//

#ifdef _WIN32
#define VIEWER
#endif // _WIN32



#ifdef VIEWER
#include <windows.h>   
#include "resource.h"
#endif // VIEWER


#ifdef _OPENMP
#include <omp.h>
#endif // 


#include "utils.h"
#include "CController.h"
#include "CParams.h"







/////////////////////// GLOBALI ////////////////////////////////////

#ifdef VIEWER
//nomi delle finestre grafiche
LPCWSTR		szApplicationName = L"NEAT Aciclico (Calvanese)";
LPCWSTR		szWindowClassName = L"Brains";
LPCWSTR		szInfoWindowClassName = L"Info Window";

//handle della finestra principale
HWND g_hwndMain = NULL;

//handle della finestra figlia che mostra le migliori topologie
HWND g_hwndInfo = NULL;
#endif


//classe che gestisce i processi
CController*	g_pController = NULL;

//classe che gestisce i parametri
CParams   g_Params;




//---------------------------- Cleanup ----------------------------------
//
//	pulisce la memoria quando il programma si arresta
//-----------------------------------------------------------------------
void Cleanup()
{
	if (g_pController)

		delete g_pController;
}

#ifdef VIEWER
//-----------------------------------WinProc-----------------------------
//
//  definizione della callback associata alla finestra principale
//-----------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	//dimensioni della finestra
	static int cx_mainClient, cy_mainClient;

	//servono per creare il back buffer della finestra ovvero il frame che viene disegnato
	static HDC		hdcBackBuffer;
	static HBITMAP	hBitmap;
	static HBITMAP	hOldBitmap;


	switch (msg)
	{
	case WM_CREATE://inizializzazione della finestra
	{
		//eventuale assegnazione del seed al generatore random
		if (CParams::is_Random)
			srand((unsigned)time(NULL));

		//prende le dimensioni della finestra
		RECT rect;
		GetClientRect(hwnd, &rect);

		cx_mainClient = rect.right;
		cy_mainClient = rect.bottom;

		//inizializza il gestore della simulazione (Classe CController) tramite costruttore
		g_pController = new CController(hwnd, cx_mainClient, cy_mainClient);

		//crea la surface su cui disegnare il back buffer
		hdcBackBuffer = CreateCompatibleDC(NULL);

		HDC hdc = GetDC(hwnd);

		hBitmap = CreateCompatibleBitmap(hdc,
			cx_mainClient,
			cy_mainClient);
		ReleaseDC(hwnd, hdc);

		hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);
	}

	break;

	//in caso di premuta di un tasto della tastiera
	case WM_KEYUP:
	{
		switch (wparam)
		{

		case VK_ESCAPE: //in caso questo si il tasto ESC spegne l'applicazione
		{
			PostQuitMessage(0);
		}

		break;



		}//end WM_KEYUP switch
	}

	break;

	//caso il cui si debba ridisegnare la finestra
	case WM_PAINT:
	{
		PAINTSTRUCT ps;

		BeginPaint(hwnd, &ps);

		//rimepie il back buffer di bianco
		BitBlt(hdcBackBuffer,
			0,
			0,
			cx_mainClient,
			cy_mainClient,
			NULL,
			NULL,
			NULL,
			WHITENESS);

		//disegna la finestra principale
		g_pController->Render(hdcBackBuffer);

		//esecuzione dellla operazione di blit, ovvero copia del back buffer nel front buffer di ps
		BitBlt(ps.hdc, 0, 0, cx_mainClient, cy_mainClient, hdcBackBuffer, 0, 0, SRCCOPY);

		EndPaint(hwnd, &ps);
	}

	break;

	//chiusura della finestra
	case WM_DESTROY:
	{
		SelectObject(hdcBackBuffer, hOldBitmap);

		//ripulisce il back buffer 
		DeleteDC(hdcBackBuffer);
		DeleteObject(hBitmap);

		//spegne l'intera applicazione
		PostQuitMessage(0);
	}

	break;

	default:break;

	}//end switch

	 // default msg handler 
	return (DefWindowProc(hwnd, msg, wparam, lparam));

}//end WinProc


 //-----------------------------------InfoWinProc-----------------------------
 //
 //callback associata alla finestra figlia
 //-----------------------------------------------------------------------
LRESULT CALLBACK InfoWindowProc(HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	//dimensioni della finestra
	static int cx_infoClient, cy_infoClient;

	switch (msg)
	{
	case WM_CREATE:
	{

		//prende le dimensioni della finestra
		RECT rect;
		GetClientRect(hwnd, &rect);

		cx_infoClient = rect.right;
		cy_infoClient = rect.bottom;
	}

	break;

	case WM_PAINT://caso il cui si debba ridisegnare la finestra
	{
		PAINTSTRUCT ps;

		BeginPaint(hwnd, &ps);

		//disegna la finestra figlia
		g_pController->RenderNetworks(ps.hdc);

		EndPaint(hwnd, &ps);
	}

	break;

	default:break;

	}//end switch

	 // default msg handler 
	return (DefWindowProc(hwnd, msg, wparam, lparam));

}//end WinProc


 //---------------------------------CreateInfoWindow---------------------------
 //
 //crea ed inzializza la finestra figlia
 //
 //----------------------------------------------------------------------------
void CreateInfoWindow(HWND hwndParent)
{
	//crea e registra la finestra figlia
	WNDCLASSEX wcInfo = { sizeof(WNDCLASSEX),
		CS_HREDRAW | CS_VREDRAW,
		InfoWindowProc,
		0,
		0,
		GetModuleHandle(NULL),
		LoadIcon(wcInfo.hInstance, MAKEINTRESOURCE(IDC_MYICON)),
		NULL,
		(HBRUSH)(GetStockObject(WHITE_BRUSH)),
		NULL,
		L"Info",
		NULL };

	RegisterClassEx(&wcInfo);

	//crea funzione chiama la callback in WM_CREATE
	g_hwndInfo = CreateWindow(L"Info",
		L"Best four phenotypes",
		WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
		GetSystemMetrics(SM_CXSCREEN) / 2,
		GetSystemMetrics(SM_CYSCREEN) / 2 - CParams::InfoWindowHeight / 2,
		CParams::InfoWindowWidth,
		CParams::InfoWindowHeight,
		hwndParent,
		NULL,
		wcInfo.hInstance,
		NULL);

	//stampa a schermo e aggiornamento
	ShowWindow(g_hwndInfo, SW_SHOWDEFAULT);
	UpdateWindow(g_hwndInfo);

	//passa le informazioni della finestra al gestore della simulazione
	g_pController->PassInfoHandle(g_hwndInfo);

	return;
}

//-----------------------------------WinMain-----------------------------------------
//	Punto d'ingresso della windows application
//-----------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE hprevinstance,
	LPSTR lpcmdline,
	int ncmdshow)
{

	WNDCLASSEX winclass;
	HWND	   hwnd;
	MSG		   msg;

	//caricamento dei parametri dal file params.ini
	if (!g_Params.Initialize())
	{
		return false;
	}

	//caratteristiche della finestra principale
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(IDC_MYICON));
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = NULL;
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = szWindowClassName;
	winclass.hIconSm = LoadIcon(hinstance, MAKEINTRESOURCE(IDC_MYICON));


	//registrazione della finestra principale
	if (!RegisterClassEx(&winclass))
	{
		MessageBox(NULL, L"Error Registering Class!", L"Error", 0);
		return 0;
	}

	//creazione della finestra principale, questa funzione chiama la callback in WM_CREATE
	if (!(hwnd = CreateWindowEx(NULL,
		szWindowClassName,
		szApplicationName,
		WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
		GetSystemMetrics(SM_CXSCREEN) / 2 - CParams::WindowWidth,
		GetSystemMetrics(SM_CYSCREEN) / 2 - CParams::WindowHeight / 2,
		CParams::WindowWidth,
		CParams::WindowHeight,
		NULL,
		NULL,
		hinstance,
		NULL)))
	{
		MessageBox(NULL, L"Error Creating Window!", L"Error", 0);
		return 0;
	}


	//tiene una traccia globale del puntatore alla finestra principale
	g_hwndMain = hwnd;

	//crea la finestra figlia
	CreateInfoWindow(hwnd);

	//stampa a schermo e aggiornamento della finestra principale
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);//chiama le callback in WM_PAINT


					   //creazione delle directory che conterranno i file riguardanti i migliori 4 individui
	char percorso[] = "";
	string cartella;

	for (int i = 0; i < 1/*4*/; i++)
	{
		cartella = "Member_" + itos(i);

		crea_cartella(percorso, cartella);
	}

	//creazione dei file delle prestazioni per i migliori 4 individui
	ofstream prestazioni0("Member_0\\prestazioni_0.txt");
	/*ofstream prestazioni1("Member_1\\prestazioni_1.txt");
	ofstream prestazioni2("Member_2\\prestazioni_2.txt");
	ofstream prestazioni3("Member_3\\prestazioni_3.txt");*/


	if (!prestazioni0)
	{
		MessageBox(NULL, L"Non ho aperto prestazioni0.txt", L"ERROR", MB_OK);
		return 0;
	}
	else
	{
		prestazioni0 << "Best Fitness\t";

		if (CParams::ModAddestramento > MODO_GLOBALE)
			prestazioni0 << "Fit%\tFitness Batch\tFit% Batch\tsoglia_addestramento";

		prestazioni0 << endl;
	}

	/*
	if (!prestazioni1)
	{
		MessageBox(NULL, L"Non ho aperto prestazioni1.txt", L"ERROR", MB_OK);
		return 0;
	}
	else
	{
		//prestazioni1 << "Best train Fitness\tFit%\t1_Msqe\t2_Msqe\t3_Msqe\tTest Fitness of Best\tFit%" << endl;
		prestazioni1 << "Best Fitness\tFit%\tFitness Batch\tFit% Batch" << endl;
	}

	if (!prestazioni2)
	{
		MessageBox(NULL, L"Non ho aperto prestazioni2.txt", L"ERROR", MB_OK);
		return 0;
	}
	else
	{
		prestazioni2 << "Best train Fitness\tFit%\t1_Msqe\t2_Msqe\t3_Msqe\tTest Fitness of Best\tFit%" << endl;
	}

	if (!prestazioni3)
	{
		MessageBox(NULL, L"Non ho aperto prestazioni3.txt", L"ERROR", MB_OK);
		return 0;
	}
	else
	{
		prestazioni3 << "Best train Fitness\tFit%\t1_Msqe\t2_Msqe\t3_Msqe\tTest Fitness of Best\tFit%" << endl;
	}

	*/

	//serve nel message loop per capire quando uscire dal programma
	bool bDone = FALSE;


	HACCEL hAccelTable = LoadAccelerators(hinstance, MAKEINTRESOURCE(IDC_NEATTEXT));



	while (!bDone)
	{

		// Ciclo di messaggi principale:
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				//Stop loop se è un messaggio di chiusura
				bDone = TRUE;
			}

			else if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		//return (int)msg.wParam;

		//esecuzione di una generazione con la funzinoe Update di CController
		if (!g_pController->Update(prestazioni0/*, prestazioni1, prestazioni2, prestazioni3*/))
		{
			//c'è un problema, chiudi l'applicazione
			bDone = TRUE;
		}

		//chiama le callback in WM_PAINT ed entrambe le finestre vengono aggiornate
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);

	}//end while


	 //cancella tutto
	Cleanup();
	UnregisterClass(szWindowClassName, winclass.hInstance);

	return 0;

}//end WinMain

#else

main()
{

	//caricamento dei parametri dal file params.ini
	if (!g_Params.Initialize())
	{
		return false;
	}

	//-------------
	//eventuale assegnazione del seed al generatore random
	if (CParams::is_Random)
		srand((unsigned)time(NULL));

	//inizializza il gestore della simulazione (Classe CController) tramite costruttore
	g_pController = new CController();


	//creazione delle directory che conterranno i file riguardanti i migliori 4 individui
	char percorso[] = "";
	string cartella;

	for (int i = 0; i < 1/*4*/; i++)
	{
		cartella = "Member_" + itos(i);

		crea_cartella(percorso, cartella);
	}

	//creazione dei file delle prestazioni per i migliori 4 individui
	ofstream prestazioni0("Member_0\\prestazioni_0.txt");


	if (!prestazioni0)
	{
		MessageBox(NULL, L"Non ho aperto prestazioni0.txt", L"ERROR", MB_OK);
		return 0;
	}
	else
	{
		prestazioni0 << "Best Fitness\t";

		if (CParams::ModAddestramento > MODO_GLOBALE)
			prestazioni0 << "Fit%\tFitness Batch\tFit% Batch\tsoglia_addestramento";

		prestazioni0 << endl;
	}

	//serve nel message loop per capire quando uscire dal programma
	bool bDone = FALSE;

	while (!bDone)
	{
		//esecuzione di una generazione con la funzinoe Update di CController
		if (!g_pController->Update(prestazioni0/*, prestazioni1, prestazioni2, prestazioni3*/))
		{
			//c'è un problema, chiudi l'applicazione
			bDone = TRUE;
		}

	//cancella tutto
	Cleanup();

	return 0;
}

#endif 
