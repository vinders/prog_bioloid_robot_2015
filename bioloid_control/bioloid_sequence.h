/* ************************************************************
VINDERS Romain
BIOLOID - SCORPION - Classe outil séquence (marche / attaque)
Date : 02/04/2015
************************************************************ */
#ifndef BIOLOID_SEQUENCE_H
#define BIOLOID_SEQUENCE_H

#include "bioloid_scorpion.h" 

/* numéros de séquences */
#define TR_WALK_START 1
#define TR_WALK_END 2
#define SEQ_ATTACK 1
#define SEQ_WALK_FWD 2
#define SEQ_WALK_LEFT 4
#define SEQ_WALK_RIGHT 8


/* exécution de séquences - classe outil (non instanciable) */
class Sequence
{
  private:
    Sequence() {} //bloquer instanciation
    
  public:
    static int _seqStep;               //étape courante
    static short _steps[4][6];         //valeurs de fin d'étapes
    static short _stepMoves[12][2][2]; //valeurs de déplacements d'étapes
    
    /* méthodes de séquences */
    static void attack(Scorpion*);
    static void walk(Scorpion*, int);
    static void doTransition(Scorpion*, int);
};

#endif
