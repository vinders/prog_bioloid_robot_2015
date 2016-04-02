/* ************************************************************
VINDERS Romain & TOMBEUR DE HAVAY Hubert
BIOLOID - SCORPION - Classe entrée-sortie scorpion
Date : 02/04/2015
************************************************************ */
#include <iostream>
#include <stdlib.h>
using namespace std; 
#include "bioloid_scorpion.h"


/*-----------------------------------------------------------------------------
  CONSTRUCTEUR - INITIALISATION ROBOT + MUTEX
 -----------------------------------------------------------------------------*/
Scorpion::Scorpion(int initSpeed)
{
    /* créer mutex d'accès aux moteurs du scorpion */
    hMutexOutput = CreateMutex(NULL, FALSE, NULL);
    if (hMutexOutput == NULL)
    {
        cout << "SCORPION: Erreur creation mutex pour ecriture" << endl;
        system("pause");
        exit(1);
    }
    
    /* configuration connexion robot */
    if (dxl_initialize(DEFAULT_PORTNUM, DEFAULT_BAUDNUM) == 0)
    {
        cout << "SCORPION: Erreur ouverture USB2Dynamixel!" << endl;
        CloseHandle(hMutexOutput);
        system("pause");
        exit(1);
    }
    _speed = initSpeed; //vitesse initiale
    
    /* mise en position de départ */
    if (WaitForSingleObject(hMutexOutput, INFINITE) != WAIT_OBJECT_0) //mutex IO scorp.
    {
        cout << "SCORPION: Erreur obtention mutex" << endl;
        CloseHandle(hMutexOutput);
        system("pause");
        exit(1);
    }
    setBasePosition(true); //initialiser position robot
    if (!ReleaseMutex(hMutexOutput) ) //libération mutex IO scorpion
    {
        cout << "SCORPION: Erreur liberation mutex" << endl;
        CloseHandle(hMutexOutput);
        system("pause");
        exit(1);
    }
}

/*-----------------------------------------------------------------------------
  DESTRUCTEUR - FERMER ACCES ROBOT + FERMER MUTEX
 -----------------------------------------------------------------------------*/
Scorpion::~Scorpion()
{
    dxl_terminate();
    CloseHandle(hMutexOutput);
}

/*-----------------------------------------------------------------------------
  SETTER / GETTER VITESSE
 -----------------------------------------------------------------------------*/
void Scorpion::setSpeed(int newSpeed)
{
    _speed = newSpeed;
}
int Scorpion::getSpeed()
{
    return _speed;
}

/*-----------------------------------------------------------------------------
  PLACER ROBOT EN POSITION INITIALE
 -----------------------------------------------------------------------------*/
void Scorpion::setBasePosition(bool wait)
{
    /* queue */
    moveMotor(TAIL_BASE, 680, 0);
    moveMotor(TAIL_MID, 760, 0);
    moveMotor(TAIL_END, 720, 0);
	if (wait)
		Sleep(250);
    
    /* rotation pattes */
    moveMotor(LEG_L1_ROTATE, 390, 0);
    moveMotor(LEG_R1_ROTATE, 630, 0);
    moveMotor(LEG_L2_ROTATE, 510, 0);
    moveMotor(LEG_R2_ROTATE, 510, 0);
    moveMotor(LEG_L3_ROTATE, 630, 0);
    moveMotor(LEG_R3_ROTATE, 390, 0);
    
    /* pli pattes */
    moveMotor(LEG_L3_BEND, 520, 0);
    moveMotor(LEG_R3_BEND, 500, 0);
    moveMotor(LEG_L2_BEND, 520, 0);
    moveMotor(LEG_R2_BEND, 500, 0);
    moveMotor(LEG_L1_BEND, 520, 0);
    moveMotor(LEG_R1_BEND, 500, 0);
}

/*-----------------------------------------------------------------------------
  DEPLACEMENT D'UN MOTEUR
 -----------------------------------------------------------------------------*/
void Scorpion::moveMotor(int motor, int position, int wait)
{
    /* déplacer moteur */
    dxl_write_word(motor, P_PRESENT_SPEED_L, getSpeed()); //vitesse
    dxl_write_word(motor, P_GOAL_POSITION_L, position);   //position
    
    /* attente optionnelle */
    if (wait)
        Sleep(wait);
}

/*-----------------------------------------------------------------------------
  ATTENTE DE FIN DE DEPLACEMENT
 -----------------------------------------------------------------------------*/
/*void Scorpion::waitMovementEnd(int motor)
{
    int moving, commStatus;
    do
    {
        if (!ReleaseMutex(hMutexOutput) ) //libération mutex IO scorpion
        {
            cout << "WAITMOV: Erreur liberation mutex(output)" << endl;
            return;
        }
        
        Sleep(20); //attente
        
        if (WaitForSingleObject(hMutexOutput, INFINITE) != WAIT_OBJECT_0) //mutex IO
        {
            cout << "WAITMOV: Erreur obtention mutex(output)" << endl;
            return;
        }
        
        /* vérifier fin mouvement *-/
        moving = dxl_read_byte(motor, P_MOVING);
        commStatus = dxl_get_result();
        if (commStatus != COMM_RXSUCCESS)
            moving = 2; //erreur : forcer fin attente

    }
    while (moving == 1); //attendre fin mouvement
}*/

