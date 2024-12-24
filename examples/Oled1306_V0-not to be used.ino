/*
 * Unit test for <Oled1306.h> lib
 * Ver 0 06-VII-2024
 * to do:
 */
static const char Version[] PROGMEM = "V0 06.VII.2024";
//                                                  0           1                     2             3             4             5               6               7
static const char SWapplication[8][18] PROGMEM ={"EmailTest", "Shades Controller", "Gray Water", "MQTTTester", "WebTester", "IR controller", "IRcontrol V1","Nixie Controller"};
#define   CurrentApp      0         // Email unit test

#define   BAUDRATE    115200
#define   LOGGME      true            // enable progress of new log on serial monitor
#define   DEBUGON     false           // debug prints
const int LEDPIN  = D4;               // LED on GPIO2
#include <Clock.h>                    // self generated master clock lib
TimePack  SysClock ;
Clock     RunClock(SysClock);         // clock instance

#include <Utilities.h>                // self generated utilities lib
Utilities RunUtil(SysClock);          // Utilities instance
#include  <WifiNet.h>
ManageWifi  SysWifi;
WifiNet   RunWifi(SysWifi);           // Wifi instance

//------------------------------------------------------------------

#include <Oled1306.h>                   // HTML screens generating lib
OledStackDef OledStack;
Oled1306 RunOled(OledStack);            // Oled instance

//****************************************************************************************/
void setup() {
  //
  // 1. set the environment
  //
  static const char Mname[] PROGMEM = "setup:";
  Serial.begin(BAUDRATE);     // Serial monitor setup
  delay(3000);
  Serial.print("\n\n\nInit Test. version: "); Serial.print(Version); Serial.print("\n\n");
  SysClock = RunClock.begin(SysClock);
  RunUtil.begin(LEDPIN);

  RunOled.begin(SysClock,2);                    // begin with test
  //
  // 2. test user input functionality
  //
  static const char LW[] PROGMEM = "enter text to test waitForUserInput ";
  char  buffer[100];
  RunUtil.waitForUserInput(SysClock,buffer,LW); // print user input
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(buffer);  Serial.print(F(" -END\n"));

  //
  // 3. simple test for <SetTimeToRow2> and <SetCharsToRow>
  //
  RunOled.clear();
  static const char L3[] PROGMEM = "enter text to test IP ";
  RunUtil.waitForUserInput(SysClock,buffer,L3); // print user input
  RunOled.SetTimeToRow2(SysClock);
  static const char M0[] PROGMEM = "This is L0";
  static const char M1[] PROGMEM = "IP 192.168.7.11";
  char  buf[20];
  strcpy_P(buf,M0);
  RunOled.SetCharsToRow(buf,0);
  strcpy_P(buf,M1);
  RunOled.SetCharsToRow(buf,3);
  RunOled.show();
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Test 1 completed"));  Serial.print(F(" -END\n"));
  
  //
  // 4. test for <DisplayProg> options
  //
  RunUtil.waitForUserInput(SysClock);           // press to start test
  static const char M2[] PROGMEM = "Test 2 str";
  static const char M3[] PROGMEM = "Hello";
  static const char M4[] PROGMEM = "Last line!";
  RunOled.DisplayProg(SysClock,0,M2,M3,M4);
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Test 2 completed"));  Serial.print(F(" -END\n"));
  RunUtil.waitForUserInput(SysClock);           // press to start test
  static const char M5[] PROGMEM = "Test 3 str";
  static const char M6[] PROGMEM = "IP 123.456.789.012";
  RunOled.DisplayProg(SysClock,2,M5,M6,M4);
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Test 3 completed"));  Serial.print(F(" -END\n"));
  RunUtil.waitForUserInput(SysClock);           // press to start test
  RunOled.DisplayProg(SysClock,3,M5,M6);
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Test 4 completed"));  Serial.print(F(" -END\n"));
  
  //
  // 5. test for <DisplayMemory> options
  //
  RunUtil.waitForUserInput(SysClock);           // press to start test
  static char X2[] PROGMEM = "Test 5 mem";
  static char X3[] PROGMEM = "Ready";
  static char X4[] PROGMEM = "BottomLine";
  RunOled.DisplayProg(SysClock,0,X2,X3,X4);
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Test 5 completed"));  Serial.print(F(" -END\n"));
  RunUtil.waitForUserInput(SysClock);           // press to start test
  static char X5[] PROGMEM = "Test 6 mem";
  static char X6[] PROGMEM = "IP 192.168.007.011";
  RunOled.DisplayProg(SysClock,2,X5,X6,X4);
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Test 6 completed"));  Serial.print(F(" -END\n"));
  RunUtil.waitForUserInput(SysClock);           // press to start test
  RunOled.DisplayProg(SysClock,3,X5,X6);
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Test 7 completed"));  Serial.print(F(" -END\n"));
  //
  // 6. test <DisplayMix>
  //
  RunUtil.waitForUserInput(SysClock);           // press to start test
  RunOled.DisplayMix(SysClock,3,M5,X6);
  //
  // 7. test <PopStack>
  //
  static const char L4[] PROGMEM = "enter text push clear screan ";
  RunUtil.waitForUserInput(SysClock,buffer,L4); // print user input
  OledStackDef Stack;
                                                // clear display
  Stack = RunOled.PushStack(SysClock,3,Stack,M5,nullptr,nullptr,X6,M4,nullptr);
  Stack = RunOled.PopStack(SysClock,Stack);
                                                // default ( two lines and time)
  static const char L6[] PROGMEM = "enter text to show default ";
  RunUtil.waitForUserInput(SysClock,buffer,L6); // print user input
  static const char M7[] PROGMEM = "default 0";
  static const char M8[] PROGMEM = "Ping!";
  static const char M9[] PROGMEM = "Done it";
  Stack = RunOled.PushStack(SysClock,0,Stack,M7,nullptr,M8,nullptr,M9,nullptr);
  Stack = RunOled.PopStack(SysClock,Stack);
                                                // show IP
  static const char L7[] PROGMEM = "enter text to show IP ";
  RunUtil.waitForUserInput(SysClock,buffer,L7); // print user input
  static const char IPdemo[] PROGMEM = "192.168.7.011";
  strcpy_P(SysWifi.DeviceIP,IPdemo);
  Stack = RunOled.PushStack(SysClock,1,Stack,M5,nullptr,nullptr,SysWifi.DeviceIP,M4,nullptr);
  Stack = RunOled.PopStack(SysClock,Stack);
  //
  // 7. test push
  //
  static const char L5[] PROGMEM = "enter short text (<6) to test push and pop ";
  RunUtil.waitForUserInput(SysClock,buffer,L5); // print user input
  RunUtil.InfoStamp(SysClock,Mname,nullptr,0,0); Serial.print(F("the user input=")); Serial.print(buffer); Serial.print(F(" -END\n"));
                                                // show user string in middle row
  static const char M10[] PROGMEM = "user input:"; 
  //                                         line 0      line 1         line 2
  Stack = RunOled.PushStack(SysClock,0,Stack,M10,nullptr,nullptr,buffer,nullptr,nullptr);
  Stack = RunOled.PopStack(SysClock,Stack);


  static const char L8[] PROGMEM = "enter text to test push and pop ";
  RunUtil.waitForUserInput(SysClock,buffer,L8); // print user input
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("the user input=")); Serial.print(buffer); Serial.print(F(" -END\n"));
                                                // show user string in middle row
  //                                         line 0      line 1         line 2
  Stack = RunOled.PushStack(SysClock,2,Stack,M10,nullptr,nullptr,buffer,nullptr,nullptr);
  Stack = RunOled.PopStack(SysClock,Stack);

  //
  // end
  //
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Last test completed"));  Serial.print(F(" -END\n")

} // end of setup

//****************************************************************************************/
void loop() {
  ;
} // end of loop

//****************************************************************************************/
//****************************************************************************************/
