/*
 * Oled1306.h library for OLED display SSD1306 applications
 * Created by Sachi Gerlitz, 10-VII-2024 ver 1
 * 
 * in this file
 *  constructor:  Oled1306;
 *  methods:      begin; PopQueueDisplayOLED; PushQueueOLED; DisplayMemory; DisplayMix; DisplayLine; SetTimeToRow2;
 *                DrawLinePattern; DrawRecPattern; clear; show; InQueueOLED; getVersion;
 *                PeekQueueOLED; DisplayProg; SetCharsToRow;
 * 
 * 24-XII-2024  V2  [rewrite for platformIO]
 *
 */
#ifndef Oled1306_h
  #define Oled1306_h

  #include  "Arduino.h"
  #include  "stdlib.h"
  #include  "Adafruit_GFX.h"
  #include  "Adafruit_SSD1306.h"                      // https://github.com/adafruit/Adafruit_SSD1306
  #include  "cppQueue.h"                              // https://github.com/SMFSW/Queue/tree/master

  #include  "Clock.h"
  #include  "Utilities.h"
  #include  "WifiNet.h"

  #include  "OledConfig.h"                            // configuration by application file

  #ifndef _LOGGMEOLED                                 // enable logging print
    #define _LOGGMEOLED 1
  #endif  //_LOGGMEOLED
  #ifndef _DEBUGOLED                                  // endable debug prints
    #define _DEBUGOLED    0
  #endif  //_DEBUGOLED

  /****************************************************************************************/
  /*
  * common definitions
  */
  // stack definitions
  #define   PAYLOADMAXLEN 20                // Max payload length for display
  #define   OLEDQUEUELEN  8                 // Length of OLED display queue
  #define  IMPLEMENTATION  FIFO
  struct OledStackDef {
    char    payload0[PAYLOADMAXLEN];        // payload to line 0
    char    payload1[PAYLOADMAXLEN];        // payload to line 1
    char    payload2[PAYLOADMAXLEN];        // payload to line 2
    uint8_t style;                          // display style {0,1,2,3,4,5,6,7}
    uint8_t scroll;                         // status of scroll {4-no scroll;3-still;2-left;1-right;0-clear display}
    uint8_t RC;                             // 1-OK 0-Error
  } ; // end of OledStackDef

  
  /****************************************************************************************/
  class Oled_1306
  {
    public:
      Oled_1306(bool activate);		                    // constructor
      bool begin(TimePack SysClock, uint8_t option);
      uint8_t PopQueueDisplayOLED(TimePack _SysClock);
      bool PushQueueOLED(uint8_t style,  uint8_t scroll, bool flush,
                      const char* P0_pgm, char* P0_dyn, 
                      const char* P1_pgm, char* P1_dyn, 
                      const char* P2_pgm=nullptr, char* P2_dyn=nullptr);      
      void DisplayMemory(TimePack _SysClock, uint8_t type, char* row0_str=nullptr, 
                    char* row1_str=nullptr, char* row2_str=nullptr);
      void DisplayMix(TimePack _SysClock, uint8_t type, const char* row0_cst, 
                            char* row1_str);  
      void DisplayLine(uint8_t start, uint8_t length, uint8_t size,
                    const char* row_cst=nullptr, char* row_dyn=nullptr);
      void SetTimeToRow2(TimePack _SysClock);
      void DrawLinePattern(uint16_t PostDelay);
      void DrawRecPattern(uint16_t PostDelay);
      void clear();
      void show();
      uint8_t InQueueOLED();
      const   char* getVersion();
      
      uint8_t PeekQueueOLED();
      void DisplayProg(TimePack _SysClock, uint8_t type, const char* row0_const=nullptr, 
                    const char* row1_const=nullptr, const char* row2_const=nullptr);
      void SetCharsToRow(char* buffer, uint8_t row);
    private:
      OledStackDef _Stack;
      bool _activate;
  };

#endif   //Oled1306_h
/****************************************************************************************/
;
