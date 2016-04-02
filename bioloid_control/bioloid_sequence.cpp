/* ************************************************************
VINDERS Romain
BIOLOID - SCORPION - Classe outil séquence (marche / attaque)
Date : 02/04/2015
************************************************************ */
#include <iostream> 
#include <climits> 
#include <string.h> 
#include <windows.h> 

using namespace std; 
#include "bioloid_sequence.h" 
#include "bioloid_scorpion.h" 

/* valeurs particulières */
#define LEG_LALL_ROTATE -1
#define LEG_RALL_ROTATE -2

/* variable de contrôle */
extern bool appContinue;


/* initialisation variables statiques */
int Sequence::_seqStep = 0;              //étape courante
short Sequence::_steps[4][6] = {         //valeurs de fin d'étapes ([étape][patte])
        {400, 620, 620, 620, 620, 620},
        {400, 400, 620, 400, 400, 400},
        {400, 400, 400, 620, 400, 400},
		{620, 620, 620, 620, 620, 400}
    };
short Sequence::_stepMoves[12][2][2] = { //valeurs de déplacements d'étapes
        {{LEG_L1_BEND,420},   {LEG_R3_BEND,600}},  //[partie][côté]{moteur, valeur}
        {{LEG_L1_ROTATE,400}, {LEG_R3_ROTATE,620}},
        {{LEG_L1_BEND,520},   {LEG_R3_BEND,500}},

        {{LEG_L2_BEND,420},   {0,0}},
        {{LEG_L2_ROTATE,400}, {LEG_RALL_ROTATE,400}},
        {{LEG_L2_BEND,520},   {0,0}},

        {{LEG_L3_BEND,420},   {LEG_R1_BEND,600}},
        {{LEG_L3_ROTATE,400}, {LEG_R1_ROTATE,620}},
        {{LEG_L3_BEND,520},   {LEG_R1_BEND,500}},

        {{0,0},               {LEG_R2_BEND,600}},
        {{LEG_LALL_ROTATE,620}, {LEG_R2_ROTATE,620}},
        {{0,0},               {LEG_R2_BEND,500}}
    };


/*-----------------------------------------------------------------------------
  SEQUENCE COMPLETE - ATTAQUE
 -----------------------------------------------------------------------------*/
void Sequence::attack(Scorpion *robot)
{
    if (WaitForSingleObject(robot->hMutexOutput,INFINITE) == WAIT_OBJECT_0) //mutex IO scorp.
    {
        /* 1 - incliner */
        robot->setBasePosition(false);
        robot->moveMotor(TAIL_BASE, 700, 0);
        robot->moveMotor(LEG_R1_BEND, 640, 0);
        robot->moveMotor(LEG_R2_BEND, 570, 0);
        robot->moveMotor(LEG_L1_BEND, 380, 0);
        robot->moveMotor(LEG_L2_BEND, 450, 150);
        
        /* 2 - queue en retrait */
        robot->moveMotor(TAIL_MID, 740, 0);
        robot->moveMotor(TAIL_END, 600, 150);
        
        /* 3 - attaque */
        robot->setSpeed(160);
        robot->moveMotor(TAIL_BASE, 800, 0);
        robot->moveMotor(TAIL_MID, 780, 0);
        robot->setSpeed(140);
        robot->moveMotor(TAIL_END, 700, 350);
        
        /* 4 - retour état initial */
        robot->setBasePosition(true);
        
        if (!ReleaseMutex(robot->hMutexOutput) ) //libération mutex IO scorpion
        {
            cout << "SEQUENCE(attack): Erreur liberation mutex(output)" << endl;
            appContinue = false;
            return;
        }
    }
}

/*-----------------------------------------------------------------------------
  ETAPE DE SEQUENCE - MARCHE
 -----------------------------------------------------------------------------*/
void Sequence::walk(Scorpion *robot, int type) 
{
    int lastSeqStep;
    int waitEnd;
    int j;
    
    if (WaitForSingleObject(robot->hMutexOutput,INFINITE) == WAIT_OBJECT_0) //mutex IO scorp.
    {
        lastSeqStep = _seqStep - 1;
        if (lastSeqStep < 0)
            lastSeqStep = 3;
        
        /* 1 - configurer position pré-requise */
        if (type != SEQ_WALK_LEFT) //côté gauche (si avant/droite)
        {
            robot->moveMotor(LEG_L1_ROTATE, _steps[lastSeqStep][0], 0);
            robot->moveMotor(LEG_L2_ROTATE, _steps[lastSeqStep][1], 0);
            robot->moveMotor(LEG_L3_ROTATE, _steps[lastSeqStep][2], 0);
        }
        if (type != SEQ_WALK_RIGHT) //côté droit (si avant/gauche)
        {
            robot->moveMotor(LEG_R1_ROTATE, _steps[lastSeqStep][3], 0);
            robot->moveMotor(LEG_R2_ROTATE, _steps[lastSeqStep][4], 0);
            robot->moveMotor(LEG_R3_ROTATE, _steps[lastSeqStep][5], 0);
        }
        
        /* 2 - effectuer étape actuelle déplacement (en 3 parties) */
        for (j = 0; j < 3; j++)
        {
            //côté gauche : si vers avant/droite (bloqué si on tourne à gauche)
            if (type == SEQ_WALK_RIGHT || type == SEQ_WALK_FWD) 
            {
                if (_stepMoves[_seqStep*3 + j][0][0]) //partie d'étape non vide
                {
                    if (_stepMoves[_seqStep*3 + j][0][0] == LEG_LALL_ROTATE) //rotation totale
                    {
                        robot->moveMotor(LEG_L3_ROTATE,_stepMoves[_seqStep*3+j][0][1], 0);
                        robot->moveMotor(LEG_L2_ROTATE,_stepMoves[_seqStep*3+j][0][1], 0);
                        robot->moveMotor(LEG_L1_ROTATE,_stepMoves[_seqStep*3+j][0][1], 300);
                    }
                    else //mouvement d'une seule patte
                    {
                        if (_seqStep==0 || (_seqStep==1 && j!=1) || type!=SEQ_WALK_FWD)
                            waitEnd = 150; //partie bloquante
                        else
                            waitEnd = 0; //non bloquante (l'autre côté bloquera)
                        robot->moveMotor(_stepMoves[_seqStep*3 + j][0][0], 
                                         _stepMoves[_seqStep*3 + j][0][1], waitEnd);
                    }
                }
            }
            //côté droit : si vers avant/gauche (bloqué si on tourne à droite)
            if (type == SEQ_WALK_LEFT || type == SEQ_WALK_FWD) 
            {
                if (_stepMoves[_seqStep*3 + j][1][0]) //partie d'étape non vide
                {
                    if (_stepMoves[_seqStep*3 + j][1][0] == LEG_RALL_ROTATE) //rotation totale
                    {
                        robot->moveMotor(LEG_R3_ROTATE,_stepMoves[_seqStep*3+j][1][1], 0);
                        robot->moveMotor(LEG_R2_ROTATE,_stepMoves[_seqStep*3+j][1][1], 0);
                        robot->moveMotor(LEG_R1_ROTATE,_stepMoves[_seqStep*3+j][1][1], 300);
                    }
                    else //mouvement d'une seule patte
                    {
                        if (_seqStep==2 || (_seqStep==3 && j!=1) || type!=SEQ_WALK_FWD)
                            waitEnd = 150; //partie bloquante
                        else
                            waitEnd = 0; //non bloquante (l'autre côté bloquera)
                        robot->moveMotor(_stepMoves[_seqStep*3 + j][1][0], 
                                         _stepMoves[_seqStep*3 + j][1][1], waitEnd);
                    }
                }
            }
        }
        
        if (!ReleaseMutex(robot->hMutexOutput) ) //libération mutex IO scorpion
        {
            cout << "SEQUENCE(walk): Erreur liberation mutex(output)" << endl;
            appContinue = false;
            return;
        }
        
        /* 3 - incrémenter numéro d'étape */
        _seqStep++;
        if (_seqStep > 3)
            _seqStep = 0;
    }
}

/*-----------------------------------------------------------------------------
  TRANSITION AVANT/APRES SEQUENCE
 -----------------------------------------------------------------------------*/
void Sequence::doTransition(Scorpion *robot, int type)
{
    if (WaitForSingleObject(robot->hMutexOutput,INFINITE) == WAIT_OBJECT_0) //mutex IO scorp.
    {
        /* choix transition */
        switch (type)
        {
            case TR_WALK_START: //début marche
                _seqStep = 0;
                robot->moveMotor(LEG_L1_BEND, 420, 0);
                robot->moveMotor(LEG_R2_BEND, 600, 100);
                robot->moveMotor(LEG_L1_ROTATE, 510, 0);
                robot->moveMotor(LEG_R1_ROTATE, 620, 100);
                robot->moveMotor(LEG_R2_ROTATE, 620, 150);
                robot->moveMotor(LEG_L1_BEND, 520, 0);
                robot->moveMotor(LEG_R2_BEND, 500, 150);
                break;
            case TR_WALK_END: //fin marche
                robot->setBasePosition(true);
                break;
        }
        
        if (!ReleaseMutex(robot->hMutexOutput) ) //libération mutex IO scorpion
        {
            cout << "SEQUENCE(transition): Erreur liberation mutex(output)" << endl;
            appContinue = false;
            return;
        }
    }
}