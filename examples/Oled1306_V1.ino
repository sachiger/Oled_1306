/*
 * Unit test for <Oled1306.h> lib
 * Ver 1 10-VII-2024
 * converted V0 to stack structure
 * to simulate real time time interrupt is used
 * Queue doesn't work with timer interrupt!!!!
 */
static const char Version[] PROGMEM = "V1 10.VII.2024";
//                                                  0           1                     2             3             4             5               6               7
static const char SWapplication[8][18] PROGMEM ={"EmailTest", "Shades Controller", "Gray Water", "MQTTTester", "WebTester", "IR controller", "IRcontrol V1","Nixie Controller"};
#define   CurrentApp      5         // IR controlled unit test

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

//------------------------------------------------------------------
#include <Ticker.h>                     // libraries/Ticker
Ticker timer;
volatile uint8_t interrupts;
volatile long curr_time;
#define PeriodToActivate  10            // time to activate in .5Sec units

//****************************************************************************************/
// ISR to Fire when Timer is triggered
void ICACHE_RAM_ATTR onTime() {
  /*
   *  ISR to have non blocking 500mS interrupt
   */
  uint8_t  ScrollControl;
	interrupts++;
  if ( interrupts>=PeriodToActivate ) {     // trigger activity
    interrupts = 0;
    //Serial.print("Oled time gap: "); Serial.print(millis()-curr_time); Serial.print("mS\n");
    curr_time = millis();                   // timrstamp renewbool
    //
    //ScrollControl = RunOled.PopQueueOLED(SysClock);
    //Serial.print(F("ScrollControl=")); Serial.print(ScrollControl); Serial.print(F(" -END\n"));
  }
	timer1_write(2500000);                    // Re-Arm the timer as using TIM_SINGLE: 12us
} // end of ISR
//****************************************************************************************/
void setup() {
  uint8_t  ScrollControl;
  //
  // 1. set the environment
  //
  static const char Mname[] PROGMEM = "setup:";
  Serial.begin(BAUDRATE);                   // Serial monitor setup
  delay(3000);
  Serial.print("\n\n\nInit Test. version: "); Serial.print(Version); Serial.print("\n\n");
  SysClock = RunClock.begin(SysClock);
  RunUtil.begin(LEDPIN);

                                              // Initialize Ticker every 0.5s by ISR
	timer1_attachInterrupt(onTime);             // Add ISR Function
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
	//  Dividers:
	//  TIM_DIV1 = 0,                           // 80MHz (80 ticks/us - 104857.588 us max)
	//  TIM_DIV16 = 1,                          // 5MHz (5 ticks/us - 1677721.4 us max)
	//  TIM_DIV256 = 3                          // 312.5Khz (1 tick = 3.2us - 26843542.4 us max)
	//  Reloads:
	//  TIM_SINGLE	0                           // on interrupt routine you need to write a new value to start the timer again
  //  TIM_LOOP	1                             // on interrupt the counter will start with the same value again
	                                            // Arm the Timer for 0.5s Interval
	timer1_write(2500000);                      // 2500000 / 5 ticks per us from TIM_DIV16 == 500,000 us interval 
  curr_time = millis();

  RunOled.begin(SysClock,2);                    // begin with test
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" Peek=")); Serial.print(RunOled.PeekQueueOLED()); Serial.print(F(" -END\n"));
  ScrollControl = RunOled.PopQueueOLED(SysClock);       // display initial message by <.begin>
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" ScrollControl=")); Serial.print(ScrollControl); Serial.print(F(" -END\n"));
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" waiting in queue ")); Serial.print(RunOled.InQueueOLED()); Serial.print(F(" -END\n"));

  //
  // 2. test push/pop style 1 (IP)
  //
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Start Test 2 -END\n"));
  #define DelayDisplay  3000                  // mS gap between displays
  static const char M0[] PROGMEM = "Test push:";
  static const char M1[] PROGMEM = "Last line";
  static const char M2[] PROGMEM = "Don't!";
  static char X0[] PROGMEM = "IP 192.168.007.011";
  static char X1[] PROGMEM = "Hello";
  RunOled.PushQueueOLED(SysClock,1,3,0,M0,nullptr,nullptr,X0,M1,nullptr);
  for (uint8_t ii=0;ii<4;ii++) {
    RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" waiting in queue ")); Serial.print(RunOled.InQueueOLED()); Serial.print(F(" -END\n"));
    delay(DelayDisplay);
    RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" Peek=")); Serial.print(RunOled.PeekQueueOLED()); Serial.print(F(" -END\n"));
    ScrollControl = RunOled.PopQueueOLED(SysClock);
    RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(ii); Serial.print(F(" ScrollControl=")); Serial.print(ScrollControl); Serial.print(F(" -END\n"));
  }
  //
  // 3. test push/pop style 0 (wide)
  // 
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Start Test 3 -END\n"));
  RunOled.PushQueueOLED(SysClock,0,3,0,M0,nullptr,nullptr,X1,M1,nullptr);
  for (uint8_t ii=0;ii<4;ii++) {
    RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" waiting in queue ")); Serial.print(RunOled.InQueueOLED()); Serial.print(F(" -END\n"));
    delay(DelayDisplay);
    RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" Peek=")); Serial.print(RunOled.PeekQueueOLED()); Serial.print(F(" -END\n"));
    ScrollControl = RunOled.PopQueueOLED(SysClock);
    RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(ii); Serial.print(F(" ScrollControl=")); Serial.print(ScrollControl); Serial.print(F(" -END\n"));
  }
  //
  // 4. test forced push/pop style 0 (wide)
  // 
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Start Test 4 -END\n"));
  RunOled.PushQueueOLED(SysClock,0,3,0,M0,nullptr,M2,nullptr,M1,nullptr);       // set to full scroll
  for (uint8_t ii=0;ii<2;ii++) {                                                // empty only 2 entries from stack
    RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" waiting in queue ")); Serial.print(RunOled.InQueueOLED()); Serial.print(F(" -END\n"));
    delay(DelayDisplay);
    RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" Peek=")); Serial.print(RunOled.PeekQueueOLED()); Serial.print(F(" -END\n"));
    ScrollControl = RunOled.PopQueueOLED(SysClock);
    RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(ii); Serial.print(F(" ScrollControl=")); Serial.print(ScrollControl); Serial.print(F(" -END\n"));
  }

  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Start Test 2 part B -END\n"));
  delay(DelayDisplay);
  RunOled.PushQueueOLED(SysClock,6,4,1,M0,nullptr,nullptr,X1,M1,nullptr);
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" waiting in queue ")); Serial.print(RunOled.InQueueOLED()); Serial.print(F(" -END\n"));
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" Peek=")); Serial.print(RunOled.PeekQueueOLED()); Serial.print(F(" -END\n"));
  ScrollControl = RunOled.PopQueueOLED(SysClock);
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" waiting in queue ")); Serial.print(RunOled.InQueueOLED()); Serial.print(F(" -END\n"));
  delay(DelayDisplay);
  RunOled.clear();
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F(" Peek=")); Serial.print(RunOled.PeekQueueOLED()); Serial.print(F(" -END\n"));
  //
  // end
  //
  RunUtil.InfoStamp(SysClock,Mname,nullptr,1,0); Serial.print(F("Last test completed"));  Serial.print(F(" -END\n"));

} // end of setup

//****************************************************************************************/
void loop() {
  ;
} // end of loop

//****************************************************************************************/
//****************************************************************************************/
