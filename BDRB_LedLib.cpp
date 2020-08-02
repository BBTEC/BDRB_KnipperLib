//============================================================================================
// HEADER
// 
// Title: BDRB_LedLib  
// Author: Bob Derboven
// Initial Date: 26/07/2020
// Date Last Change: 26/07/2020
// Version: 1.0
//
// Description: 
//
// Bibliotheek met LED functies klasse 

// Documentatie:
// LedLib:
// =============
#include "BDRB_LedLib.h"
#include <BDRB_StateMachine.h>


BDRB_StateMachine myLedStateMachine;
//StateMachineBDRB myLedStateMachine;

  // State declarations
struct strctLedStates
  {
    const int UNDEFINED = 0;            //==> MOET ALTIJD AANWEZIG ZIJN
    const int INITIAL = 1;              //==> MOET ALTIJD AANWEZIG ZIJN
    const int SELF_CHECK = 2;           //==> MOET ALTIJD AANWEZIG ZIJN
    const int STOPPED_ON_ERROR = 3;     //==> MOET ALTIJD AANWEZIG ZIJN
    
    //USER STATES:
    const int STANDBY = 10;
    const int PULSE1 = 20;
    const int PULSE2 = 21;
    const int PULSE3 = 22;
    const int PULSE4 = 23;
    const int FADING_IN = 30;
    const int FADING_OUT = 31; 
    const int FADING_OFF = 32;   
  };
strctLedStates myLedStates;

//********************************************************************************************
//* Constructor routine
//********************************************************************************************
BDRB_LedLib::BDRB_LedLib(byte LEDPIN1)
{
  if(LEDPIN1 > 0)
    {
      _LedPin1 = LEDPIN1;
      pinMode(_LedPin1, OUTPUT);
    }

  //Activeer StateMachine
  myLedStateMachine.Initialise(); //Initialiseer StateMachine
  myLedStateMachine.Debug(true); //Activeer debug messages van de StateMachineBDRB klasse.
  myLedStateMachine.Run(true); //Laat statemachine draaien
  //Serial.println("statemachine zou geïnitialiseerd moeten zijn");

//  while(myLedStateMachine.State()!= myLedStates.INITIAL) 
//    {
//      myLedStateMachine.Initialise(); //Initialiseer StateMachine
//      myLedStateMachine.Debug(true); //Activeer debug messages van de StateMachineBDRB klasse.
//      myLedStateMachine.Run(true); //Laat statemachine draaien  
//    }
  
}

//********************************************************************************************
//* Routine die statemachine aandrijft - moet cyclisch aangeroepen worden
//********************************************************************************************
void BDRB_LedLib::RunLedLib()
{
  _StateMachineLED();
}

//********************************************************************************************
//* Flankdetectie functie
//********************************************************************************************
bool BDRB_LedLib::_FlankDetectie(bool Signaal,enmDetectieType DetType, bool &HulpMerker)
  {
    bool _Result = false;
    switch (DetType)
      {
               
        case _RISING:
          //Serial.println("FLANKDETECTIE? Signaal= " + String(Signaal) + " // Hulpmerker= " + String(HulpMerker));
          if (Signaal && !HulpMerker)
            {
              _Result = true;
              Serial.println("POS FLANK");
            }
          else
            {
              _Result = false;
              //Serial.println("HulpMerker status = " + String(HulpMerker) + " VS _MH_GoKnipper = " + String(_MH_GoKnipper) );
            }
          break;
          
        case _FALLING:
          if (!Signaal && HulpMerker)
            {
              _Result = true;
              Serial.println("NEG FLANK");
            }
          else
            {
              _Result = false;
            }
          break;
          
        case _CHANGE:
          if ((Signaal && !HulpMerker) || (!Signaal && HulpMerker))
            {
              _Result = true;
              Serial.println("POS/NEG FLANK");
            }
          else
            {
              _Result = false;
            }
          break;
          
        default:
          _Result = false;
          break;
      }

      HulpMerker = Signaal;
      return _Result;
  }

//********************************************************************************************
//* Routine die led symmetrisch doet knipperen   __|--|__|--|__|--|__
//********************************************************************************************
void BDRB_LedLib::Knipper(unsigned long T_OnOff)
{
   _GoKnipper = true;
   _timPulse1 = T_OnOff;
   _timPulse2 = T_OnOff;
   _timPulse3 = 0;
   _timPulse4 = 0;
   //Serial.println("Knipper Sym functie aangeroepen");
}

//********************************************************************************************
//* Routine die led assymmetrisch doet knipperen  ______|--|______|--|______|--|______
//********************************************************************************************
void BDRB_LedLib::Knipper(unsigned long T_On, unsigned long T_Off)
{
   _GoKnipper = true;
   _timPulse1 = T_On;
   _timPulse2 = T_Off;
   _timPulse3 = 0;
   _timPulse4 = 0;
   //Serial.println("Knipper 1xAssy functie aangeroepen");
}

//********************************************************************************************
//* Routine die led dubbel assymmetrisch doet knipperen  ______|--|___|-|______|--|___|-|____
//********************************************************************************************
void BDRB_LedLib::Knipper(unsigned long T_On, unsigned long T_Off, unsigned long T_On2, unsigned long T_Off2)
{
   _GoKnipper = true;
   _timPulse1 = T_On;
   _timPulse2 = T_Off;
   _timPulse3 = T_On2;
   _timPulse4 = T_Off2;
   //Serial.println("Knipper 2xAssy functie aangeroepen");
}

//********************************************************************************************
//                                   /|          /|          /|          /|          /|
//                                  / |         / |         / |         / |         / |
//                                 /  |        /  |        /  |        /  |        /  |
//* Routine die led laat infaden  /   |_______/   |_______/   |_______/   |_______/   |_______
//                                     < Toff>
//********************************************************************************************
void BDRB_LedLib::FadeIn(int T_FadeIn, unsigned long T_Off)
{
   _GoFade = true;
   
   if(T_FadeIn > int(_MinPulseTime))
    {
      _timFadeIn = T_FadeIn;   
    }
   else
    {
      _timFadeIn = _MinPulseTime; 
    }
   _timFadeOut = 0;
   _timOff = T_Off;
}
//********************************************************************************************
//                                |\          |\          |\          |\          |\       
//                                | \         | \         | \         | \         | \
//                                |  \        |  \        |  \        |  \        |  \
//* Routine die led laat uitfaden |   \_______|   \_______|   \_______|   \_______|   \_______
//                                     < Toff>
//********************************************************************************************
void BDRB_LedLib::FadeOut(int T_FadeOut, unsigned long T_Off)
{
   _GoFade = true;
   
   if(T_FadeOut > int(_MinPulseTime))
    {
      _timFadeOut = T_FadeOut;   
    }
   else
    {
      _timFadeOut = _MinPulseTime; 
    }
   _timFadeIn = 0;
   _timOff = T_Off; 
}
//********************************************************************************************
//                                   / \             / \             / \             / \         
//                                  /   \           /   \           /   \           /   \     
//                                 /     \         /     \         /     \         /     \
//* Routine die led laat faden    /       \_______/       \_______/       \_______/       \
//                                         < Toff>
//********************************************************************************************
void BDRB_LedLib::FadeInOut(int T_Fade, unsigned long T_Off)
{
   _GoFade = true;
   
   if(T_Fade > int(_MinPulseTime))
    {
      _timFadeIn = T_Fade; 
      _timFadeOut = T_Fade;   
    }
   else
    {
      _timFadeIn = _MinPulseTime; 
      _timFadeOut = _MinPulseTime;
    }  
   _timOff = T_Off;  
}
//********************************************************************************************
//                                   / \                / \                / \                       
//                                  /    \             /    \             /    \                
//                                 /       \          /       \          /       \         
//* Routine die led laat faden    /          \_______/          \_______/          \_______
//                                            < Toff>
//********************************************************************************************
void BDRB_LedLib::FadeInOut(int T_FadeIn,int T_FadeOut, unsigned long T_Off)
{
   _GoFade = true;
   
   if(T_FadeIn > int(_MinPulseTime))
    {
      _timFadeIn = T_FadeIn;   
    }
   else
    {
      _timFadeIn = _MinPulseTime; 
    }
   
   if(T_FadeOut > int(_MinPulseTime))
    {
      _timFadeOut = T_FadeOut;   
    }
   else
    {
      _timFadeOut = _MinPulseTime; 
    }
    
   _timOff = T_Off;    
}
//********************************************************************************************
//* Statemachine
//********************************************************************************************
void BDRB_LedLib::_StateMachineLED() 
{
      myLedStateMachine.Run(true); //Aanroepen van de StateMachine RUN routine
      
      //Flankdetectie op knippervraag
      _FP_GoKnipper = _FlankDetectie(_GoKnipper, _RISING, _MH_GoKnipper );
      _GoKnipper = false;

      //Flankdetectie op fadevraag
      _FP_GoFade = _FlankDetectie(_GoFade, _RISING, _MH_GoFade );
      //Serial.println("_FP_GoFade = " + String(_FP_GoFade));
      _GoFade = false;
      
      //================================================================================
      //= STATE MACHINE
      //================================================================================

       //------------------------------------------------------------------------------
       // State:   INITIAL
       //------------------------------------------------------------------------------
        if (myLedStateMachine.State()== myLedStates.INITIAL)
          {
            //**************************************************************************
            //* ACTIE:
            //* - Nothing
            //* - 
            //*
            //* TRANSITIE:
            //* - Ga onvoorwaardelijk naar standby
            //*
            //************************************************************************** 

            //* ACTIONS ON ENTRY
            if (myLedStateMachine.StatePulse())
              {
                Serial.println("ENTER STATE INITIAL");
                myLedStateMachine.SetStateTimer(100); //[ms]
              }

            //* CONTINUOUS ACTIONS
            analogWrite(_LedPin1, 255);


            //* TRANSITIONS
            if (not myLedStateMachine.StatePulse() && myLedStateMachine.StatetimerElapsed())
              {     
                myLedStateMachine.SetState(myLedStates.STANDBY);
              }
          }
      
       //------------------------------------------------------------------------------
       // State:   STANDBY
       //------------------------------------------------------------------------------
        if (myLedStateMachine.State()== myLedStates.STANDBY)
          {
            //**************************************************************************
            //* ACTIE:
            //* - Nothing
            //* - 
            //*
            //* TRANSITIE:
            //* - Nothing
            //*
            //************************************************************************** 

            //* ACTIONS ON ENTRY
            if (myLedStateMachine.StatePulse())
              {
                //Nothng
              }

            //* CONTINUOUS ACTIONS
            analogWrite(_LedPin1, 0);
            

            //* TRANSITIONS
            if (not myLedStateMachine.StatePulse())
              {
                if(_FP_GoKnipper || _MH_GoKnipper)
                  {
                    myLedStateMachine.SetState(myLedStates.PULSE1);
                  }
                 else if(_FP_GoFade || _MH_GoFade)
                  {                
                    if (_timFadeIn > 0) 
                      {
                        myLedStateMachine.SetState(myLedStates.FADING_IN);
                                    
                      }
                    else if (_timFadeOut > 0)
                      {
                       myLedStateMachine.SetState(myLedStates.FADING_OUT);    
                      }
                    else
                      {
                        //Nothing
                      }
                  }   
              }
          }
       //------------------------------------------------------------------------------
       // State:   PULSE1
       //------------------------------------------------------------------------------
        if (myLedStateMachine.State()== myLedStates.PULSE1)
          {
            //**************************************************************************
            //* ACTIE:
            //* - Nothing
            //* - 
            //*
            //* TRANSITIE:
            //* - Nothing
            //*
            //************************************************************************** 

            //* ACTIONS ON ENTRY
            if (myLedStateMachine.StatePulse())
              {
                if (_timPulse1 >= _MinPulseTime)
                  {
                    myLedStateMachine.SetStateTimer(_timPulse1-1); //[ms]
                  }
                else
                  {
                    myLedStateMachine.SetStateTimer(_MinPulseTime-1); //[ms]
                  }
              }

            //* CONTINUOUS ACTIONS
            analogWrite(_LedPin1, 255);

            //* TRANSITIONS
            if (not myLedStateMachine.StatePulse())
              {
                if (myLedStateMachine.StatetimerElapsed()&& _MH_GoKnipper)
                  {
                    myLedStateMachine.SetState(myLedStates.PULSE2); 
                  }
                else if (! _MH_GoKnipper)
                  {
                    myLedStateMachine.SetState(myLedStates.STANDBY);
                  }
                
              }
          }
        //------------------------------------------------------------------------------
        // State:   PULSE2
        //------------------------------------------------------------------------------
        if (myLedStateMachine.State()== myLedStates.PULSE2)
          {
            //**************************************************************************
            //* ACTIE:
            //* - Nothing
            //* - 
            //*
            //* TRANSITIE:
            //* - Nothing
            //*
            //************************************************************************** 

            //* ACTIONS ON ENTRY
            if (myLedStateMachine.StatePulse())
              {
                if (_timPulse2 >= _MinPulseTime)
                  {
                    myLedStateMachine.SetStateTimer(_timPulse2-1); //[ms]
                  }
                else
                  {
                    myLedStateMachine.SetStateTimer(_MinPulseTime-1); //[ms]
                  }
              }

            //* CONTINUOUS ACTIONS
            analogWrite(_LedPin1, 0);

            //* TRANSITIONS
            if (not myLedStateMachine.StatePulse())
              {
                if (myLedStateMachine.StatetimerElapsed()&& _MH_GoKnipper)
                  {
                    if (_timPulse3 > 0)
                      {
                        myLedStateMachine.SetState(myLedStates.PULSE3);  
                      }
                    else
                      {
                        myLedStateMachine.SetState(myLedStates.PULSE1); 
                      }
                  }
                else if (! _MH_GoKnipper)
                  {
                    myLedStateMachine.SetState(myLedStates.STANDBY);
                  }
                
              }
          }
        //------------------------------------------------------------------------------
        // State:   PULSE3
        //------------------------------------------------------------------------------
        if (myLedStateMachine.State()== myLedStates.PULSE3)
          {
            //**************************************************************************
            //* ACTIE:
            //* - Nothing
            //* - 
            //*
            //* TRANSITIE:
            //* - Nothing
            //*
            //************************************************************************** 

            //* ACTIONS ON ENTRY
            if (myLedStateMachine.StatePulse())
              {
                if (_timPulse3 >= _MinPulseTime)
                  {
                    myLedStateMachine.SetStateTimer(_timPulse3-1); //[ms]
                  }
                else
                  {
                    myLedStateMachine.SetStateTimer(_MinPulseTime-1); //[ms]
                  }
              }

            //* CONTINUOUS ACTIONS
            analogWrite(_LedPin1, 255);
            //* TRANSITIONS
            if (not myLedStateMachine.StatePulse())
              {
                if (myLedStateMachine.StatetimerElapsed()&& _MH_GoKnipper)
                  {
                    myLedStateMachine.SetState(myLedStates.PULSE4);  
                  }
                else if (! _MH_GoKnipper)
                  {
                    myLedStateMachine.SetState(myLedStates.STANDBY);
                  }
                
              }
          }
        //------------------------------------------------------------------------------
        // State:   PULSE4
        //------------------------------------------------------------------------------
        if (myLedStateMachine.State()== myLedStates.PULSE4)
          {
            //**************************************************************************
            //* ACTIE:
            //* - Nothing
            //* - 
            //*
            //* TRANSITIE:
            //* - Nothing
            //*
            //************************************************************************** 

            //* ACTIONS ON ENTRY
            if (myLedStateMachine.StatePulse())
              {
                if (_timPulse4 >= _MinPulseTime)
                  {
                    myLedStateMachine.SetStateTimer(_timPulse4-1); //[ms]
                  }
                else
                  {
                    myLedStateMachine.SetStateTimer(_MinPulseTime-1); //[ms]
                  }
              }

            //* CONTINUOUS ACTIONS          
            analogWrite(_LedPin1, 0);

            //* TRANSITIONS
            if (not myLedStateMachine.StatePulse())
              {
                if (myLedStateMachine.StatetimerElapsed()&& _MH_GoKnipper)
                  {
                    myLedStateMachine.SetState(myLedStates.PULSE1);  
                  }
                else if (! _MH_GoKnipper)
                  {
                    myLedStateMachine.SetState(myLedStates.STANDBY);
                  }
                
              }
          }
       //------------------------------------------------------------------------------
       // State:   FADING_IN
       //------------------------------------------------------------------------------
        if (myLedStateMachine.State()== myLedStates.FADING_IN)
          {
            //**************************************************************************
            //* ACTIE:
            //* - Deze stap genereert een stijgende zaagtand
            //* - 
            //*
            //* TRANSITIE:
            //* - Nothing
            //*
            //************************************************************************** 

            //* ACTIONS ON ENTRY
            if (myLedStateMachine.StatePulse())
              {
                analogWrite(_LedPin1, 0);
                myLedStateMachine.SetStateTimer(_timFadeIn-1);
              }

            //* CONTINUOUS ACTIONS
          
            analogWrite(_LedPin1, map(int(myLedStateMachine.ElapsedStateTime()),0,_timFadeIn,0,255));
            //Serial.println("analog val = " + String(map(int(myLedStateMachine.ElapsedStateTime()),0,_timFadeIn,0,255)));

            //* TRANSITIONS
            if (not myLedStateMachine.StatePulse())
              {
                if (myLedStateMachine.StatetimerElapsed())
                  {
                    if (_timFadeOut > 0)
                      {
                        myLedStateMachine.SetState(myLedStates.FADING_OUT); 
                      }
                    else if(_timOff > 0)
                      {
                       myLedStateMachine.SetState(myLedStates.FADING_OFF); 
                      }
                    else
                      {
                       myLedStateMachine.SetState(myLedStates.STANDBY);  
                      }
                  }
              }
          }
       //------------------------------------------------------------------------------
       // State:   FADING_OUT
       //------------------------------------------------------------------------------
        if (myLedStateMachine.State()== myLedStates.FADING_OUT)
          {
            //**************************************************************************
            //* ACTIE:
            //* - Deze stap genereert een dalende zaagtand
            //* - 
            //*
            //* TRANSITIE:
            //* - Nothing
            //*
            //************************************************************************** 

            //* ACTIONS ON ENTRY
            if (myLedStateMachine.StatePulse())
              {
                analogWrite(_LedPin1, 255);
                myLedStateMachine.SetStateTimer(_timFadeOut-1);
              }

            //* CONTINUOUS ACTIONS
          
            analogWrite(_LedPin1, map(int(myLedStateMachine.ElapsedStateTime()),0,_timFadeOut,255,0));
            //Serial.println("analog val = " + String(map(int(myLedStateMachine.ElapsedStateTime()),0,_timFadeIn,255,0)));

            //* TRANSITIONS
            if (not myLedStateMachine.StatePulse())
              {
                if (myLedStateMachine.StatetimerElapsed())
                  {
                    if (_timOff > 0)
                      { 
                        myLedStateMachine.SetState(myLedStates.FADING_OFF); 
                      }
                    else if(_timFadeIn > 0)
                      {
                       myLedStateMachine.SetState(myLedStates.FADING_IN); 
                      }
                    else
                      {
                       myLedStateMachine.SetState(myLedStates.STANDBY);  
                      }
                  }
              }
          }
        //------------------------------------------------------------------------------
       // State:   FADING_OFF
       //------------------------------------------------------------------------------
        if (myLedStateMachine.State()== myLedStates.FADING_OFF)
          {
            //**************************************************************************
            //* ACTIE:
            //* - Deze stap genereert een stijgende zaagtand
            //* - 
            //*
            //* TRANSITIE:
            //* - Nothing
            //*
            //************************************************************************** 

            //* ACTIONS ON ENTRY
            if (myLedStateMachine.StatePulse())
              {
                myLedStateMachine.SetStateTimer(_timOff-1);
                analogWrite(_LedPin1, 0);
              }

            //* CONTINUOUS ACTIONS

            //* TRANSITIONS
            if (not myLedStateMachine.StatePulse())
              {
                if (myLedStateMachine.StatetimerElapsed())
                  {
                    myLedStateMachine.SetState(myLedStates.STANDBY);  
                  }
              }
          }
          _GoKnipper = false;
          _GoFade = false;
}
