/*
 * Oled1306.cpp library for Oled1306OLED display SSD1306 applications
 * Created by Sachi Gerlitz, 10-VII-2024 ver 1
 * 
 * in this file
 *  constructor:  Oled1306;
 *  methods:      begin; PopQueueDisplayOLED; PushQueueOLED; DisplayMemory; DisplayMix; DisplayLine; SetTimeToRow2;
 *                DrawLinePattern; DrawRecPattern; clear; show; InQueueOLED; getVersion;
 *                PeekQueueOLED; DisplayProg; SetCharsToRow;
 * 
 * V2 24-XII-2024
 * 
 * Display      +---------------------------------------+
 *              | line 0 up to 11 chars                 |
 *              | line 1 wide up to 7 chars OR 2 wraped |
 *              |                        lines 11 chars |
 *              | line 2 up to 11 chars                 |
 *              +---------------------------------------+
 * Common settings
 *  <scroll>    action               
 *    4         display freeze
 *    3         display freeze
 *    2         display scroll left
 *    1         display scroll right
 *    0         clear display
 *
 *  style                 Line 0      Line 1      line 2   Display Scroll  
 *    0     default       payload0    payload1    time      on      set
 *                        max 11 c.   max 7 c.
 *    1     IP            "Device IP" 2 lines IP  time      on      set
 *                                    max 22 c.
 *    2     long payload  PayLoad0    PayLoad1    time      on      set
 *                        max 11 c.   max 7 c.
 *    3     clear display   off       off         off       off     stop
 *
 *    4     full wide     payload0    payload1    payload2  set     set
 *                        max 11 c.   max 7 c.
 *    5     full narrow   payload0    payload1    payload2  set     set
 *                        max 11 c.   2 narrow lines
 *    6     full wide NS  payload0    payload1    payload2  set     stop
 *                        max 11 c.   max 7 c.
 *    7     full nar NS   payload0    payload1    payload2  set     stop
 *                        max 11 c.   2 narrow lines
 * 
 */

#include  "Oled_1306.h"

TimePack  _SysClock_Oled ;
Utilities _RunUtil_Oled(_SysClock_Oled);            // Utilities instance
                                                    // DS1306 Screen declaration
#define SCREEN_WIDTH    128                         // OLED display width, in pixels
#define SCREEN_HEIGHT   64                          // OLED display height, in pixels
#define OLED_RESET      -1                          // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS  0x3C                        //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLEDSTARTDELAY  500                         // 500mS delay for stability
Adafruit_SSD1306 OLED_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // display instance

OledStackDef  OLEDqueue;                            // actual stack
cppQueue ShowOLED (sizeof(OledStackDef), OLEDQUEUELEN,  IMPLEMENTATION); // Input instantiate queue

//****************************************************************************************/
Oled_1306::Oled_1306(bool activate) {
  /*
   * Constructor for OLED 1306 library
   * <activate>   - 0 - disbale library (1306 does not exist)
   *              - 1 - activate library
   */
  _activate = activate;
}     // end of Clock 

//****************************************************************************************/
bool Oled_1306::begin(TimePack _SysClock, uint8_t option){
  /*
   * method to initiate D1306 OLED display
   * <_SysClock>- system clock
   * <option>   - test pattern option:  0 - do not run test
   *                                    1 - line pattern test
   *                                    2 - Rectangular test pattern
   *                                    3 - TBD
   * returns  1 - OK
   *          0 - Error in starting Oled
   */
  static const char Mname[] PROGMEM = "Oled1306::begin:";
  static const char E0[] PROGMEM = "ERROR setup: SSD1306 allocation failed\nERROR setup: Processing stops!";
  static const char L0[] PROGMEM = "SSD1306 allocation successful. Version";
  
  if ( !_activate ) return 0;                 // the library not activated
  delay(OLEDSTARTDELAY);                      // stability delay
  if(!OLED_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    _RunUtil_Oled.InfoStamp(_SysClock,Mname,E0,1,1); 
    return  false;                            // error starting OLED
  } else {
    #if _LOGGMEOLED==1
      _RunUtil_Oled.InfoStamp(_SysClock,Mname,L0,1,0); Serial.print(getVersion());  Serial.print(F(" -END\n"));
    #endif //_LOGGMEOLED
    delay(OLEDSTARTDELAY);                    // stability delay
  }   // end of Oled initiaion

  // init tests
  switch ( option ) {
    case  0:                                  // do nothing
      break;
    case  1:                                  // line patern for OLED
      DrawLinePattern(PostTestDelay);
      break;
    case  2:                                  // Rectangular patern for OLED
      DrawRecPattern(PostTestDelay);
      delay(PostTestDelay);  
      break;
    default:                                  // error
      break;
  }   // end of test pattern

  // init display
  OLED_display.clearDisplay();                // Clear display buffer
                                              // initial message: style-3 lines, no scroll, flush stack
  PushQueueOLED(6,4,1,M01,nullptr,M05,nullptr,M06,nullptr);
  return  true;
}     // end of begin 

//****************************************************************************************/
uint8_t Oled_1306::PopQueueDisplayOLED(TimePack SysClock) {
  /*
   * method to pop a record from the queue stack and display on OLED @<style>
   * it returns indication about the current record content, so timing can be controlled
   * Theory of oprations
   * for a recork in the stak; (1) pop (2) display (3) push back to implement scrolling
   * in case of non empty stack, the scrolling is cancelled (otherwise, wirred order in the FIFO stack)
   * <_Stack.scroll> sets the mechanism for scrolling: 3 is a still display before scroll, creating a push for 2 (scrolling) and
   * later 1. 4 means still display w/o scrolling
   * 
   * returns      0 - stack is empty
   *              1 - record found and diplayed
   *              2 - record found, displayed and scrolled
   *              3 - display cleared
   *
   * if there is a record on stack queue, it check the scroll indicator: (push to complete the scroll action)
   *  <scroll>    action                post action         push new record
   *    4         display freeze        -                   No
   *    3         display freeze        set <scroll> to 2   Yes
   *    2         display scroll left   set <scroll> to 1   Yes
   *    1         display scroll right  set <scroll> to 0   Yes
   *    0         clear display         -                   No
   *
   *  style                 Line 0      Line 1      line 2   Display Scroll  
   *    0     default       payload0    payload1    time      On      set
   *                        max 11 c.   max 7 c.
   *    1     IP            "Device IP" 2 lines IP  time      On      set
   *                                    max 22 c.
   *    2     long payload  PayLoad0    PayLoad1    time      On      set
   *                        max 11 c.   max 7 c.
   *    3     clear display   off       off         off       off     stop
   *
   *    4     full wide     payload0    payload1    payload2  On      set
   *                        max 11 c.   max 7 c.
   *    5     full narrow   payload0    payload1    payload2  On      set
   *                        max 11 c.   2 narrow lines
   *    6     full wide NS  payload0    payload1    payload2  On      stop
   *                        max 11 c.   max 7 c.
   *    7     full nar NS   payload0    payload1    payload2  On      stop
   *                        max 11 c.   2 narrow lines
   *  
   */
  if ( !_activate ) return 0;                               // the library not activated
  TimePack _SysClock = SysClock;
  OledStackDef _Stack;
                                                            // retrieve from stack
  if (ShowOLED.isEmpty()) {                                 // queue is empty
    _Stack.RC=0;
    return  _Stack.RC; 
  }   // end stack is empty
  ShowOLED.pop(&_Stack);                                    // retieve (pop) data to be displayed
                                                            // display
  switch ( _Stack.style ) {                                 // select style to display
    case  0:                                                // default: top,middle,time
      DisplayMemory(_SysClock,1,_Stack.payload0,_Stack.payload1);
      break;
    case  1:                                                // IP head, 2 lines IP, time
      DisplayMix(_SysClock,3,M03,_Stack.payload1);
      break;
    case  2:                                                // top,2 middle, time
      DisplayMemory(_SysClock,3,_Stack.payload0,_Stack.payload1);
      break;
    case  3:                                                // clear display
      _Stack.scroll = 0;
      break;
    case  4:                                                // top,2 middle, bottom
    case  6:                                                // top,2 middle, bottom
      DisplayMemory(_SysClock,0,_Stack.payload0,_Stack.payload1,_Stack.payload2);
      break;
    case  5:                                                // top,2 middle, bottom
    case  7:                                                // top,2 middle, bottom
      DisplayMemory(_SysClock,2,_Stack.payload0,_Stack.payload1,_Stack.payload2);
      break;
    default:                                                // error
      break;
  }   // end of style switch
                                                            // scrolling mechanism and push back
  switch ( _Stack.scroll ) {                                // select scroll mode
    case  4:                                                // display no scroll
      OLED_display.stopscroll();
      _Stack.RC = 1;
      break;
    case  3:                                                // still display before scroll
      _Stack.scroll--;
      if ( ShowOLED.peek(&_Stack) ) {;                     // stack not empty!
      } else {                                             // stack empty - can push to scroll
        PushQueueOLED(_Stack.style,_Stack.scroll,0,nullptr,_Stack.payload0,nullptr,_Stack.payload1,nullptr,_Stack.payload2);
      }
      OLED_display.stopscroll();
      _Stack.RC = 1;
      break;
    case  2:                                                // scroll left
      _Stack.scroll--;
      OLED_display.startscrollleft  (0x00, 0x0F);
      if ( ShowOLED.peek(&_Stack) ) {;                     // stack not empty!
      } else {                                             // stack empty - can push to scroll
        PushQueueOLED(_Stack.style,_Stack.scroll,0,nullptr,_Stack.payload0,nullptr,_Stack.payload1,nullptr,_Stack.payload2);
      }
      _Stack.RC = 2;
      break;
    case  1:                                                // scroll right
      _Stack.scroll--;
      OLED_display.startscrollright (0x00, 0x0F);
      if ( ShowOLED.peek(&_Stack) ) {;                     // stack not empty!
      } else {                                             // stack empty - can push to scroll
        PushQueueOLED(_Stack.style,_Stack.scroll,0,nullptr,_Stack.payload0,nullptr,_Stack.payload1,nullptr,_Stack.payload2);
      }
      _Stack.RC = 2;
      break;
    case  0:                                                // clear display
      OLED_display.clearDisplay();
      OLED_display.display();
      _Stack.RC = 3;
      break;
    default:                                                // error
      OLED_display.stopscroll();
      _Stack.RC = 1;
      break;
  }   // end of scroll selection

  #if _DEBUGOLED==1
    Serial.print("ACK 00:00:00 PopQueueDisplayOLED style="); Serial.print(_Stack.style); Serial.print(" scroll="); Serial.print(_Stack.scroll); 
    Serial.print(" Stack:["); Serial.print(_Stack.payload0); Serial.print("] ["); Serial.print(_Stack.payload1); 
    Serial.print("] ["); Serial.print(_Stack.payload2); Serial.print("] RC="); Serial.print(_Stack.RC); Serial.print("-END\n");
  #endif  //_DEBUGOLED
  return  _Stack.RC;
}   // end of PopQueueDisplayOLED

//****************************************************************************************/
bool Oled_1306::PushQueueOLED(uint8_t style, uint8_t scroll, bool flush,
                      const char* P0_pgm, char* P0_dyn, 
                      const char* P1_pgm, char* P1_dyn, 
                      const char* P2_pgm, char* P2_dyn) {
  /*
   * method to push parameters into FIFO stack
   *
   * style      - style {0,1,2,3,4,5,6,7}
   * scroll     - controls the scrolling of the display of this record {0,1,2,3,4}
   * flush      - if set, clears the stack queue before push, eliminating previous entries
   * P0_pgm / P0_dyn is payload to line 0
   * P1_pgm / P1_dyn is payload to line 1
   * P2_pgm / P2_dyn is payload to line 2
   * each payload field get on of {Px_pgm,Px_dyn}, one of each of the pairs must exist
   * returns  - 1 - for successful push
   *            0 - for error or function in active
   */
  static const char ArgumetError[] PROGMEM = "ERROR!";
  static const char ArgumetTimestamp[] PROGMEM = "Timestamp";
  if ( !_activate ) return 0;                               // the library not activated
  OledStackDef _Stack;
  _Stack.RC = 0;
  _Stack.scroll = scroll;
  _Stack.style = style;
  char* pntr;
                                                            // select and load argument 0
  if ( P0_dyn==nullptr ) {                                  // reg mem not provided
    if ( P0_pgm==nullptr ) strcpy_P(_Stack.payload0,ArgumetError); // error - no arguments provided
    else strcpy_P(_Stack.payload0,P0_pgm);                  // PROG MEM provided
  } else {                                                  // regular mem provided
    pntr = P0_dyn;
    for ( uint8_t ii=0;ii<PAYLOADMAXLEN;ii++){
      _Stack.payload0[ii] = 0x00;
      if ( *pntr == 0x00 ) break;
      _Stack.payload0[ii] = *pntr++;
    } // end of copy loop
  } // end of argument 0 selection

                                                            // select and load argument 1
  if ( P1_dyn==nullptr ) {                                  // reg mem not provided
    if ( P1_pgm==nullptr ) strcpy_P(_Stack.payload1,ArgumetError);  // error - no arguments provided
    else strcpy_P(_Stack.payload1,P1_pgm);                  // PROG MEM provided
  } else {                                                  // regular mem provided
    pntr = P1_dyn;
    for ( uint8_t ii=0;ii<PAYLOADMAXLEN;ii++){
      _Stack.payload1[ii] = 0x00;
      if ( *pntr == 0x00 ) break;
      _Stack.payload1[ii] = *pntr++;
    } // end of copy loop
  } // end of argument 1 selection

                                                            // select and load argument 2
  if (style==0 || style==1 || style==2 ) {                  // for third line time stamp - set on display time
    strcpy_P(_Stack.payload2,ArgumetTimestamp);             // indicate timestamp
  } else if ( P2_dyn==nullptr ) {                           // reg mem not provided
    if ( P2_pgm==nullptr ) strcpy_P(_Stack.payload2,ArgumetError);  // error - no arguments provided
    else strcpy_P(_Stack.payload2,P2_pgm);                  // PROG MEM provided
  } else {                                                  // regular mem provided
    pntr = P2_dyn;
    for ( uint8_t ii=0;ii<PAYLOADMAXLEN;ii++){
      _Stack.payload2[ii] = 0x00;
      if ( *pntr == 0x00 ) break;
      _Stack.payload2[ii] = *pntr++;
    } // end of copy loop
  } // end of argument 2 selection

                                                            // push into stack queue
  if ( flush )  ShowOLED.flush();                           // reset (flash) the stack
  if ( !ShowOLED.push(&_Stack) ) {                          // error on queue
    return false;
  }   
  return  true; 
} // end of PushQueueOLED

//****************************************************************************************/
void Oled_1306::DisplayMemory( TimePack _SysClock, uint8_t type, char* row0_str, 
                            char* row1_str, char* row2_str) {
  /*
   * method to display 3 lines from regular on OLED row, with optional parameters
   * display format:    type A                    type B
   *                    slim top line             slim top line
   *                    wide middle line          slim two middle line (one line with wrap)
   *                    slim bottom line          slim bottom line
   *      
   * <_SysClock>  - system clock structure
   * <type>       - 0 - type A  3 free lines
   *                1 - type A  2 free lines (top and middle) and time display on bottom line
   *                2 - type B  3 free lines
   *                3 - type B  2 free lines (top and wrap middle line) and time display on bottom line
   * <row0_str>   - pointer to data for row 0
   * <row1_str>   - pointer to data for row 1
   * <row2_str>   - pointer to data for row 2 (optional argument for type {1,3})
   */
  uint8_t len0=0;
  uint8_t len1=0;
  uint8_t len2=0;
  if ( row0_str!=nullptr ) len0 = strlen(row0_str);
  if ( len0>PAYLOADMAXLEN ) return;                       // error
  if ( row1_str!=nullptr )  len1 = strlen(row1_str);
  if ( len1>PAYLOADMAXLEN ) return;                       // error
  if ( row2_str!=nullptr )  len2 = strlen(row2_str);
  if ( len2>PAYLOADMAXLEN ) return;                       // error
  
  OLED_display.clearDisplay();                            // Clear display buffer
  OLED_display.setTextColor(WHITE);
  OLED_display.setFont();                                 // return to dealt font
  if ( len0>0 ) DisplayLine( OLED_Start_row_0,len0,OLED_CharSize_row_0,row0_str);

  switch ( type ) {
    case  0:                                              // 0 - 3 provided lines size 2,3,2
      if ( len1>0 ) DisplayLine( OLED_Start_row_1,len1,OLED_CharSize_row_1,row1_str);
      if ( len2>0 ) DisplayLine( OLED_Start_row_2,len2,OLED_CharSize_row_2,row2_str);
      break;
    case  1:                                              // 1 - 2 provided lines, line 3 is time, size 2,3,2
      if ( len1>0 ) DisplayLine( OLED_Start_row_1,len1,OLED_CharSize_row_1,row1_str);
      SetTimeToRow2(_SysClock);
      break;
    case  2:                                              // 0 - 3 provided lines size 2,2,2
      if ( len1>0 ) DisplayLine( OLED_Start_row_3,len1,OLED_CharSize_row_3,row1_str);
      if ( len2>0 ) DisplayLine( OLED_Start_row_2,len2,OLED_CharSize_row_2,row2_str);
      break;
    case  3:                                              // 1 - 2 provided lines, line 3 is time, size 2,2,2
      if ( len1>0 ) DisplayLine( OLED_Start_row_3,len1,OLED_CharSize_row_3,row1_str);
      SetTimeToRow2(_SysClock);
      break;
    case  4:
    default:
      break;
  } // end of type 
  OLED_display.display();                                 // display
}   // end of DisplayMemory

//****************************************************************************************/
void Oled_1306::DisplayMix( TimePack _SysClock, uint8_t type, const char* row0_cst, 
                            char* row1_str) {
  /*
   * method to display 3 lines from regular on OLED row, with optional parameters
   * display format:    type A                    type B
   *                    slim top line             slim top line
   *                    wide middle line          slim two middle line (one line with wrap)
   *                    slim bottom line          slim bottom line
   *      
   * <_SysClock>  - system clock structure
   * <type>       - 1 - type A  2 free lines (top and middle) and time display on bottom line
   *                3 - type B  2 free lines (top and wrap middle line) and time display on bottom line
   * <row0_cst>   - pointer to data for row 0 (prog mem)
   * <row1_str>   - pointer to data for row 1 (reg mem)
   */
  uint8_t len0=0;
  uint8_t len1=0;
  #if _DEBUGOLED==1
  
  #endif  //_DEBUGOLED==1
  if ( row0_cst!=nullptr ) len0 = strlen_P(row0_cst);
  if ( len0>PAYLOADMAXLEN ) return;                       // error
  if ( row1_str!=nullptr )  len1 = strlen(row1_str);
  if ( len1>PAYLOADMAXLEN ) return;                       // error
  
  OLED_display.clearDisplay();                            // Clear display buffer
  OLED_display.setTextColor(WHITE);
  OLED_display.setFont();                                 // return to dealt font
  if ( len0>0 ) DisplayLine( OLED_Start_row_0,len0,OLED_CharSize_row_0,row0_cst);

  switch ( type ) {                                       // select option for middle row
    case  1:                                              // wide middle row
      if ( len1>0 ) DisplayLine( OLED_Start_row_1,len1,OLED_CharSize_row_1,row1_str);
      SetTimeToRow2(_SysClock);
      break;
    case  3:                                              // slim 2 middle rows
      if ( len1>0 ) DisplayLine( OLED_Start_row_3,len1,OLED_CharSize_row_3,row1_str);
      SetTimeToRow2(_SysClock);
      break;

    case  0:
    case  2:
    case  4:
    default:
      break;
  } // end of type 
    
  SetTimeToRow2(_SysClock);                               // time to bottom row
  OLED_display.display();                                 // display
}   // end of DisplayMix

//****************************************************************************************/
void Oled_1306::DisplayLine( uint8_t start, uint8_t length, uint8_t size,
                            const char* row_cst, char* row_dyn) {
  /*
   * method to display a line on OLED with optional PROG MEM or regular memory
   */
  static const char ArgumetError[] PROGMEM = "ERROR";
  char* pntr;
  OLED_display.setTextSize(size);                           // value {1,...,8}
  OLED_display.setCursor(0,start);                          // point to row start
  if ( row_dyn==nullptr ) {                                 // reg mem not provided
    char  buffer[PAYLOADMAXLEN];
    pntr = &buffer[0];
    if ( row_cst==nullptr ) {                               // error - no arguments provided
      strcpy_P(buffer,ArgumetError);
    } else {                                                // PROG MEM provided
      strcpy_P(buffer,row_cst);
    }
  } else {                                                  // regular mem provided
    pntr = row_dyn;
  } // end of argument selection
  for (uint8_t ii=0;ii<length;ii++) OLED_display.print(*pntr++); 
} // end of DisplayLine

//****************************************************************************************/
void Oled_1306::SetTimeToRow2(TimePack _SysClock) {
  /*
   * method to set system time to last row (2)
   */
  OLED_display.setTextColor(WHITE);
  OLED_display.setFont();                                 // return to dealt font
  OLED_display.setTextSize(OLED_CharSize_row_2);          // value {1,...,8}
  OLED_display.setCursor(0,OLED_Start_row_2);             // point to row start
  OLED_display.print(F("T "));
  OLED_display.print(_SysClock.clockHour/10);
  OLED_display.print(_SysClock.clockHour%10);
  OLED_display.print(F(":"));
  OLED_display.print(_SysClock.clockMin/10);
  OLED_display.print(_SysClock.clockMin%10);
  OLED_display.print(F(":"));
  OLED_display.print(_SysClock.clockSec/10);
  OLED_display.print(_SysClock.clockSec%10);
    
}   // end of SetTimeToRow2

//****************************************************************************************/
void Oled_1306::DrawLinePattern(uint16_t PostDelay) {
  /*
   * method to draw test patern of lines
   * <PostDelay>  - the post test delay [mS] before return
   */
  int16_t i;

  OLED_display.clearDisplay();                            // Clear display buffer

  for(i=0; i<OLED_display.width(); i+=4) {
    OLED_display.drawLine(0, 0, i, OLED_display.height()-1, WHITE);
    OLED_display.display();                               // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<OLED_display.height(); i+=4) {
    OLED_display.drawLine(0, 0, OLED_display.width()-1, i, WHITE);
    OLED_display.display();
    delay(1);
  }
  delay(250);

  OLED_display.clearDisplay();

  for(i=0; i<OLED_display.width(); i+=4) {
    OLED_display.drawLine(0, OLED_display.height()-1, i, 0, WHITE);
    OLED_display.display();
    delay(1);
  }
  for(i=OLED_display.height()-1; i>=0; i-=4) {
    OLED_display.drawLine(0, OLED_display.height()-1, OLED_display.width()-1, i, WHITE);
    OLED_display.display();
    delay(1);
  }
  delay(250);

  OLED_display.clearDisplay();

  for(i=OLED_display.width()-1; i>=0; i-=4) {
    OLED_display.drawLine(OLED_display.width()-1, OLED_display.height()-1, i, 0, WHITE);
    OLED_display.display();
    delay(1);
  }
  for(i=OLED_display.height()-1; i>=0; i-=4) {
    OLED_display.drawLine(OLED_display.width()-1, OLED_display.height()-1, 0, i, WHITE);
    OLED_display.display();
    delay(1);
  }
  delay(250);

  OLED_display.clearDisplay();

  for(i=0; i<OLED_display.height(); i+=4) {
    OLED_display.drawLine(OLED_display.width()-1, 0, 0, i, WHITE);
    OLED_display.display();
    delay(1);
  }
  for(i=0; i<OLED_display.width(); i+=4) {
    OLED_display.drawLine(OLED_display.width()-1, 0, i, OLED_display.height()-1, WHITE);
    OLED_display.display();
    delay(1);
  }

  delay(PostDelay);                                         // Pause for <PostDelay> mS
}     // end of DrawLinePattern

//****************************************************************************************/
void Oled_1306::DrawRecPattern(uint16_t PostDelay) {
  /*
   * method to draw test patern of rectangulars
   * <PostDelay>  - the post test delay [mS] before return
   */
  int16_t i;
  OLED_display.clearDisplay();                            // Clear display buffer
  for(int16_t i=0; i<OLED_display.height()/2; i+=2) {
    OLED_display.drawRect(i, i, OLED_display.width()-2*i, OLED_display.height()-2*i, WHITE);
    OLED_display.display();                               // Update screen with each newly-drawn rectangle
    delay(20);
  }
  delay(PostDelay);                                       // Pause for <PostDelay> mS
}     // end of DrawRecPattern

//****************************************************************************************/
void Oled_1306::clear() {
  /*
   * method to clear OLED 
   */
  if ( !_activate ) return;                               // the library not activated
  OLED_display.clearDisplay();                            // Clear display buffer
  OLED_display.display();                                 // display
}     // end of show

//****************************************************************************************/
void Oled_1306::show() {
  /*
   * method to display OLED post printing
   */
  if ( !_activate ) return;                               // the library not activated
  OLED_display.display();                                 // display
}     // end of show

//****************************************************************************************/
uint8_t Oled_1306::InQueueOLED() {
  /*
   * method to return how many records are in the queue stack
   */
  if ( !_activate ) return 0;                               // the library not activated
  //Serial.print(F("Oled1306::info number of records stored in the queue=")); Serial.print(ShowOLED.getCount()); Serial.print(F("\n"));
  //Serial.print(F("Oled1306::info number of records left  empty in the queue=")); Serial.print(ShowOLED.getRemainingCount()); Serial.print(F("\n")); 
  return ShowOLED.getCount();
}     // end of InQueueOLED

// ****************************************************************************************/
const   char* Oled_1306::getVersion() {
    /*
     * method to return the lib's version
     */
    return  Oled_1306_Version;
}   // end of getVersion

//****************************************************************************************/
//****************************************************************************************/
uint8_t Oled_1306::PeekQueueOLED() {
  /*
   * method to peek into the stack queue
   *
   * returns      0 - stack is empty
   *              1 - the record's style
   */
  if ( !_activate ) return 0;                               // the library not activated
  OledStackDef _Stack;
                                                            // retrieve from stack
  if (ShowOLED.isEmpty()) {                                 // queue is empty
    return 0; 
  }   // end stack is empty
  
  ShowOLED.peek(&_Stack);                                    // retieve (pop) data to be displayed
  #if _DEBUGOLED==1
    Serial.print("ACK 00:00:00 PeekQueueOLED style="); Serial.print(_Stack.style); Serial.print(" scroll="); Serial.print(_Stack.scroll); 
    Serial.print(" Stack:["); Serial.print(_Stack.payload0); Serial.print("] ["); Serial.print(_Stack.payload1); 
    Serial.print("] ["); Serial.print(_Stack.payload2); Serial.print("] -END\n");
  #endif  //_DEBUGOLED
  return  _Stack.style;
}     // end of PeekQueueOLED

//****************************************************************************************/
void Oled_1306::DisplayProg( TimePack _SysClock, uint8_t type, const char* row0_const, 
                            const char* row1_const, const char* row2_const) {
  /*
   * method to display 3 lines from PROG mem on OLED row, with optional parameters
   * display format:    type A                    type B
   *                    slim top line             slim top line
   *                    wide middle line          slim two middle line (one line with wrap)
   *                    slim bottom line          slim bottom line
   *      
   * <_SysClock>  - system clock structure
   * <type>       - 0 - type A  3 free lines
   *                1 - type A  2 free lines (top and middle) and time display on bottom line
   *                2 - type B  3 free lines
   *                3 - type B  2 free lines (top and wrap middle line) and time display on bottom line
   * <row0_const> - pointer to PROG MEM data for row 0
   * <row1_const> - pointer to PROG MEM data for row 1
   * <row2_const> - pointer to PROG MEM data for row 2 (optional argument for type {1,3})
   */
  uint8_t len0=0;
  uint8_t len1=0;
  uint8_t len2=0;
  if ( row0_const!=nullptr ) len0 = strlen(row0_const);
  if ( len0>PAYLOADMAXLEN ) return;                       // error
  if ( row1_const!=nullptr )  len1 = strlen(row1_const);
  if ( len1>PAYLOADMAXLEN ) return;                       // error
  if ( row2_const!=nullptr )  len2 = strlen(row2_const);
  if ( len2>PAYLOADMAXLEN ) return;                       // error
  
  OLED_display.clearDisplay();                            // Clear display buffer
  OLED_display.setTextColor(WHITE);
  OLED_display.setFont();                                 // return to dealt font
  if ( len0>0 ) DisplayLine( OLED_Start_row_0,len0,OLED_CharSize_row_0,row0_const);

  switch ( type ) {
    case  0:                                              // 0 - 3 provided lines size 2,3,2
      if ( len1>0 ) DisplayLine( OLED_Start_row_1,len1,OLED_CharSize_row_1,row1_const);
      if ( len2>0 ) DisplayLine( OLED_Start_row_2,len2,OLED_CharSize_row_2,row2_const);
      break;
    case  1:                                              // 1 - 2 provided lines, line 3 is time, size 2,3,2
      if ( len1>0 ) DisplayLine( OLED_Start_row_1,len1,OLED_CharSize_row_1,row1_const);
      SetTimeToRow2(_SysClock);
      break;
    case  2:                                              // 0 - 3 provided lines size 2,2,2
      if ( len1>0 ) DisplayLine( OLED_Start_row_3,len1,OLED_CharSize_row_3,row1_const);
      if ( len2>0 ) DisplayLine( OLED_Start_row_2,len2,OLED_CharSize_row_2,row2_const);
      break;
    case  3:                                              // 1 - 2 provided lines, line 3 is time, size 2,2,2
      if ( len1>0 ) DisplayLine( OLED_Start_row_3,len1,OLED_CharSize_row_3,row1_const);
      SetTimeToRow2(_SysClock);
      break;
    case  4:
    default:
      break;
  } // end of type 
  OLED_display.display();                                 // display
}   // end of DisplayProg

//****************************************************************************************/
void Oled_1306::SetCharsToRow(char* buffer, uint8_t row) {
  /*
   * method to display chars on OLED row
   * <buffer> - pointer to chars to display. must end with 0x00
   *            length is limitted to <PAYLOADMAXLEN> (overflow might occur)
   * <row>    - row number {0,1,2,3}
   *            (row 3 is actually row 1 half size font)
   */
  char* pntr;
  if ( strlen(buffer)>PAYLOADMAXLEN ) return;             // error
  
  OLED_display.setTextColor(WHITE);
  OLED_display.setFont();                                 // return to dealt font
  switch ( row ) {
    case  0:
      OLED_display.setTextSize(OLED_CharSize_row_0);      // value {1,...,8}
      OLED_display.setCursor(0,OLED_Start_row_0);         // point to row start
      break;
    case  1:
      OLED_display.setTextSize(OLED_CharSize_row_1);      // value {1,...,8}
      OLED_display.setCursor(0,OLED_Start_row_1);         // point to row start
      break;
    case  2:
      OLED_display.setTextSize(OLED_CharSize_row_2);      // value {1,...,8}
      OLED_display.setCursor(0,OLED_Start_row_2);         // point to row start
      break;
    case  3:
      OLED_display.setTextSize(OLED_CharSize_row_3);      // value {1,...,8}
      OLED_display.setCursor(0,OLED_Start_row_3);         // point to row start
      break;
    default:                                              //error
      return;
  }   // end of row select
  pntr = buffer;
  for (uint8_t ii=0;ii<strlen(buffer);ii++) OLED_display.print(*pntr++);    
}   // end of SetCharsToRow

//****************************************************************************************/
//****************************************************************************************/
