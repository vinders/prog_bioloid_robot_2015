/* ************************************************************
VINDERS Romain
BIOLOID - SCORPION - Application de contrôle
Date : 02/04/2015
************************************************************ */
#include <iostream> 
#include <stdlib.h> 
#include <climits> 
#include <windows.h> 

using namespace std;
#include "bioloid_sequence.h" 
#include "bioloid_scorpion.h" 

#define NB_THREADS 2  //pour déclaration et fermeture


/* prototypes - application de contrôle */
void readKeyboard();
DWORD WINAPI runSequence(LPVOID);
DWORD WINAPI runAction(LPVOID);
void displayCommands();
int getSequence(int, int*);
void getAction(short*, short*);
void setAction(int, int);
void closeApplication(int);


/* structure données partagées (input) */
struct
{
	short front; //déplacement
	short left;
	short right;
	short attack; //attaque
	short head_up; //tête
	short head_down;
	short pincer_left; //pinces
	short pincer_right;
} pressedKeys;

/* variables de contrôle */
bool appContinue = true;
HANDLE hThreadArray[NB_THREADS];
Scorpion *robot;       //outil d'entrée-sortie du robot
HANDLE hMutexReadKeys; //mutex données partagées

/*----------------------------------------------------------------------------*/

int main()
{
	/* initialisation robot */
	robot = new Scorpion(140);

	/* création mutex données partagées */
	hMutexReadKeys = CreateMutex(NULL, FALSE, NULL); //mutex données partagées (input)
	if (hMutexReadKeys == NULL)
	{
		cout << "MAIN: Erreur creation mutex lecture" << endl;
		exit(1);
	}

	/* création des threads */
	hThreadArray[0] = CreateThread(NULL, 0, runSequence, NULL, 0, NULL);//séquences
	hThreadArray[1] = CreateThread(NULL, 0, runAction, NULL, 0, NULL);  //actions simples
	if (hThreadArray[0] == NULL || hThreadArray[1] == NULL)
	{
		cout << "MAIN: Erreur creation threads" << endl;
		appContinue = false;
		system("pause");
		closeApplication(1);
	}

	/* thread principal - lecture clavier */
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL); //priorité+1
	readKeyboard();

	/* attente fin threads et fermeture */
	closeApplication(0);
	return 0;
}

/*-----------------------------------------------------------------------------
THREAD LECTURE DU CLAVIER (INPUT)
-----------------------------------------------------------------------------*/
void readKeyboard()
{
	displayCommands(); //afficher mode d'emploi

	/* boucle de lecture du clavier */
	while (appContinue)
	{
		Sleep(30);
		if (GetAsyncKeyState(VK_ESCAPE)) //demande de fermeture
			appContinue = false;
		else
		{
			if (WaitForSingleObject(hMutexReadKeys, INFINITE) == WAIT_OBJECT_0) //mutex input
			{
				/* lire et actualiser input clavier */
				pressedKeys.front = GetKeyState(VK_UP) & 0x8000;
				pressedKeys.left = GetKeyState(VK_LEFT) & 0x8000;
				pressedKeys.right = GetKeyState(VK_RIGHT) & 0x8000;
				pressedKeys.attack = GetKeyState(VK_SPACE) & 0x8000;
				pressedKeys.head_up = GetKeyState(0x5A) & 0x8000; //Z
				pressedKeys.head_down = GetKeyState(0x53) & 0x8000; //S
				pressedKeys.pincer_left = GetKeyState(0x51) & 0x8000; //Q
				pressedKeys.pincer_right = GetKeyState(0x44) & 0x8000; //D

				if (!ReleaseMutex(hMutexReadKeys)) //libération mutex input
				{
					cout << "READKEYBOARD: Erreur liberation mutex(readkeys)" << endl;
					appContinue = false;
					closeApplication(1);
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
THREAD LANCEMENT DE SEQUENCES
-----------------------------------------------------------------------------*/
DWORD WINAPI runSequence(LPVOID lpParam)
{
	UNREFERENCED_PARAMETER(lpParam); //aucun paramètre de thread
	int seqType = 0;
	int transition = 0;

	/* boucle lancement séquence (selon input) */
	while (appContinue)
	{
		Sleep(30);

		/* récupérer demande(s) de séquence(s) */
		seqType = getSequence(seqType, &transition);

		/* transition vers séquence */
		if (transition)
		{
			Sequence::doTransition(robot, transition);
			transition = 0;
		}
		/* lancer séquence demandée */
		if (seqType == SEQ_ATTACK) //séquence d'attaque (complète)
		{
			Sequence::attack(robot);
			seqType = 0;
		}
		else
		{
			if (seqType >= SEQ_WALK_FWD) //séquence de marche (une étape)
				Sequence::walk(robot, seqType);
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------------
THREAD LANCEMENT D'ACTIONS SIMPLES
-----------------------------------------------------------------------------*/
DWORD WINAPI runAction(LPVOID lpParam)
{
	UNREFERENCED_PARAMETER(lpParam); //aucun paramètre de thread
	short actionHead = 0;
	short actionPincer[2] = { 0, 0 };
	short lastActionPincer[2] = { 0, 0 };

	/* boucle d'actionnement (selon input) */
	while (appContinue)
	{
		Sleep(30);

		/* récupérer demande(s) d'action(s) */
		lastActionPincer[0] = actionPincer[0]; //mémoriser précédentes
		lastActionPincer[1] = actionPincer[1];
		getAction(&actionHead, actionPincer); //récupérer actions

		/* déplacer tête selon demande */
		if (actionHead)
		{
			if (actionHead > 0)
				setAction(HEAD_ROTATE, 500);
			else
				setAction(HEAD_ROTATE, 700);
			actionHead = 0; //fin action tête
		}

		/* déplacer pince gauche selon demande */
		if (actionPincer[0] != lastActionPincer[0])
		{
			if (actionPincer[0])
				setAction(PINCER_L, 400);
			else
				setAction(PINCER_L, 600);
		}

		/* déplacer pince droite selon demande */
		if (actionPincer[1] != lastActionPincer[1])
		{
			if (actionPincer[1])
				setAction(PINCER_R, 600);
			else
				setAction(PINCER_R, 400);
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------------
AFFICHAGE MODE D'EMPLOI
-----------------------------------------------------------------------------*/
void displayCommands()
{
	cout << "------------------------------" << endl;
	cout << "DOSSIER BIOLOID - Scorpion" << endl;
	cout << "par VINDERS R. et TOMBEURDEHAVAY H., 2222" << endl;
	cout << "------------------------------" << endl;
	cout << "\t<Haut/Gauche/Droite>   Deplacer" << endl;
	cout << "\t<Z/S>    Incliner tete" << endl;
	cout << "\t<Q>      Fermer pince gauche" << endl;
	cout << "\t<D>      Fermer pince droitee" << endl;
	cout << "\t<ESPACE> Attaquer" << endl << endl;
	cout << "Appuyez sur ESC pour arreter le programme." << endl;
}

/*-----------------------------------------------------------------------------
DETERMINER DEMANDE DE SEQUENCE (SELON INPUT)
-----------------------------------------------------------------------------*/
int getSequence(int lastSeqType, int *pTransition)
{
	int seqType = 0;
	int turn = 0;

	if (WaitForSingleObject(hMutexReadKeys, INFINITE) == WAIT_OBJECT_0) //mutex input
	{
		/* vérifier demande d'attaque */
		if (pressedKeys.attack) //prioritaire
		{
			seqType = SEQ_ATTACK;
		}
		else
		{
			/* vérifier demande de déplacement */
			if (pressedKeys.front != 0) //avancer
			{
				seqType = SEQ_WALK_FWD;
			}
			if ((turn = pressedKeys.left + pressedKeys.right) != 0) //tourner
			{
				/* s'assurer qu'une seule des deux touches soit pressée */
				if (pressedKeys.left == turn)
					seqType = SEQ_WALK_LEFT;
				else if (pressedKeys.right == turn)
					seqType = SEQ_WALK_RIGHT;
			}
		}

		if (!ReleaseMutex(hMutexReadKeys)) //libération mutex input
		{
			cout << "GETSEQUENCE: Erreur liberation mutex(readkeys)" << endl;
			appContinue = false;
		}

		/* vérifier nécessité de transition */
		if (lastSeqType >= SEQ_WALK_FWD) //fin de marche
		{
			if (seqType == 0 || seqType == SEQ_ATTACK)
				*pTransition = TR_WALK_END;
		}
		else if (!lastSeqType && seqType >= SEQ_WALK_FWD) //début de marche
		{
			*pTransition = TR_WALK_START;
		}
	}
	return seqType;
}

/*-----------------------------------------------------------------------------
DETERMINER DEMANDE D'ACTION (SELON INPUT)
-----------------------------------------------------------------------------*/
void getAction(short *pHead, short *pPincer)
{
	if (WaitForSingleObject(hMutexReadKeys, INFINITE) == WAIT_OBJECT_0) //mutex input
	{
		/* vérifier demande déplacement tête */
		if (pressedKeys.head_up)
		{
			if (!pressedKeys.head_down)
				*pHead = 1;
		}
		else
		{
			if (pressedKeys.head_down)
				*pHead = -1;
		}

		/* vérifier demande déplacement pinces */
		if (pressedKeys.pincer_left) //pince gauche
			pPincer[0] = 1;
		else
			pPincer[0] = 0;
		if (pressedKeys.pincer_right) //pince droite
			pPincer[1] = 1;
		else
			pPincer[1] = 0;

		if (!ReleaseMutex(hMutexReadKeys)) //libération mutex input
		{
			cout << "GETACTION: Erreur liberation mutex(readkeys)" << endl;
			appContinue = false;
		}
	}
}

/*-----------------------------------------------------------------------------
EFFECTUER ACTION (AVEC MUTEX SCORPION)
-----------------------------------------------------------------------------*/
void setAction(int motor, int val)
{
	if (WaitForSingleObject(robot->hMutexOutput, INFINITE) == WAIT_OBJECT_0)//mutex IO scorp.
	{
		robot->moveMotor(motor, val, 0); //effectuer action

		if (!ReleaseMutex(robot->hMutexOutput)) //libération mutex IO scorpion
		{
			cout << "SETACTION: Erreur liberation mutex(output)" << endl;
			appContinue = false;
		}
	}
}

/*-----------------------------------------------------------------------------
FERMETURE APPLICATION
-----------------------------------------------------------------------------*/
void closeApplication(int retVal)
{
	/* fermeture threads */
	for (int i = 0; i < NB_THREADS; i++)
	{
		if (hThreadArray[i] != NULL)
		{
			WaitForSingleObject(hThreadArray[i], 2000);
			CloseHandle(hThreadArray[i]);
		}
	}

	/* fermeture mutex et robot */
	CloseHandle(hMutexReadKeys);
	delete robot;
	robot = NULL;

	system("pause");
	exit(retVal);
}

