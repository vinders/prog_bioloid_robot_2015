/* ************************************************************
VINDERS Romain & TOMBEUR DE HAVAY Hubert
BIOLOID - SCORPION - Classe entrée-sortie scorpion
Date : 02/04/2015
************************************************************ */
#ifndef BIOLOID_SCORPION_H
#define BIOLOID_SCORPION_H

#pragma once

#include "dynamixel.h"
#include <windows.h>

#pragma comment(lib, "dynamixel.lib")


/* numéros des moteurs */
#define PINCER_L            1
#define PINCER_R            2
#define TAIL_BASE           16
#define TAIL_MID            17
#define TAIL_END            18
#define LEG_L1_ROTATE       3
#define LEG_L2_ROTATE       7
#define LEG_L3_ROTATE       11
#define LEG_R1_ROTATE       4
#define LEG_R2_ROTATE       8
#define LEG_R3_ROTATE       12
#define LEG_L1_BEND         5
#define LEG_L2_BEND         9
#define LEG_L3_BEND         13
#define LEG_R1_BEND         6
#define LEG_R2_BEND         10
#define LEG_R3_BEND         14
#define HEAD_ROTATE         15

/* table adresse de contrôle */
#define P_GOAL_POSITION_L       30
#define P_GOAL_POSITION_H       31
#define P_PRESENT_POSITION_L    36
#define P_PRESENT_POSITION_H    37
#define P_PRESENT_SPEED_L       38
#define P_PRESET_SPEED_H        39
#define P_MOVING                46

/* configuration de la connexion */
#define DEFAULT_PORTNUM    5 //COM5
#define DEFAULT_BAUDNUM    1 //1Mbps


/* classe IO moteurs du scorpion */
class Scorpion
{
  private:
    int _speed; //vitesse des moteurs

  public:
    HANDLE hMutexOutput; //mutex IO scorpion
    
    /* constructeur / destructeur */
    Scorpion(int);
    ~Scorpion();

    /* méthodes du scorpion */
    void setSpeed(int);
    int  getSpeed();
    void setBasePosition(bool);
    void moveMotor(int, int, int);
    //void waitMovementEnd(int);
};

#endif

