#ifndef BDRB_LedLib_h
#define BDRB_LedLib_h

#if (ARDUINO >=100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class BDRB_LedLib
{
//============================================================================================
// DECLARATIESECTIE
  //------------------------------------------------------------------------------------------
    public:
  //------------------------------------------------------------------------------------------
      //*******************
      // CONSTRUCTOR
      //*******************
      BDRB_LedLib(byte LEDPIN1 = 0);
  
      //*******************
      // PUBLIC MEMBERS
      //*******************
      void RunLedLib();                                                                                 //Methode om de statemachine van de LedLib draaiende te houde, moet cyclisch aangeroepen worden.
      void Knipper(unsigned long T_OnOff);                                                              //Knipper Symmetrisch
      void Knipper(unsigned long T_On, unsigned long T_Off);                                            //Knipper 1x Assymetrisch
      void Knipper(unsigned long T_On, unsigned long T_Off, unsigned long T_On2, unsigned long T_Off2); //Knipper 2x Assymetrisch
      void FadeIn(int T_FadeIn, unsigned long T_Off);                                     //Fade On
      void FadeOut(int T_FadeOut, unsigned long T_Off);                                   //Fade Off
      void FadeInOut(int T_Fade, unsigned long T_Off);                                    //Fade OnOff Symmetrisch
      void FadeInOut(int T_FadeIn,int T_FadeOut, unsigned long T_Off);          //Fade OnOff Assymetrisch
      
      void Initialise();                                  //Functie die statemachine initialiseert
      bool Run(bool RunStateMachine = true);              //Als deze routine wordt aangeroepen, draait de statemachine

      enum enmDetectieType
        {
          _RISING,
          _FALLING,
          _CHANGE
        };
      
  //------------------------------------------------------------------------------------------
    private:
  //------------------------------------------------------------------------------------------
      byte _LedPin1 = 0;
      void _StateMachineLED();
      bool _FlankDetectie(bool Signaal,enmDetectieType DetType, bool& HulpMerker);

      bool _GoKnipper = false;
      bool _FP_GoKnipper = false;
      bool _MH_GoKnipper = false;

      bool _GoFade = false;
      bool _FP_GoFade = false;
      bool _MH_GoFade = false;
      
      const unsigned long _MinPulseTime = 5UL; //[ms]
      unsigned long _timPulse1 = 0;
      unsigned long _timPulse2 = 0;
      unsigned long _timPulse3 = 0;
      unsigned long _timPulse4 = 0;
      unsigned int _timFadeIn = 0;
      unsigned int _timFadeOut = 0;
      unsigned long _timOff = 0;

      int _FadeVal = 0;
      

};
#endif
