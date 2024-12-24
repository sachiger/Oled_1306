/*
 * OledConfig.h  is a configuraiton file for Oled_1306.h library 
 *
 * Created by Sachi Gerlitz, 24-XII-2024 ver 2
 */

#ifndef OledConfig_h
  #define OledConfig_h
  #define Oled_1306_Version "2.0.2"
  //
  // display definitions
  // row 0
  #define OLED_Start_row_0    0       // start of row in line
  #define OLED_CharSize_row_0 2       // character size for row
  // row 1
  #define OLED_Start_row_1    16      // start of row in line
  #define OLED_CharSize_row_1 3       // character size for row
  // row 2
  #define OLED_Start_row_2    48      // start of row in line
  #define OLED_CharSize_row_2 2       // character size for row
  // row 3  - this is actually row 1 but half size fornt
  #define OLED_Start_row_3    16      // start of row in line
  #define OLED_CharSize_row_3 2       // character size for row
  
  #define PostTestDelay  1000         // how many mS to delay post pattern test

  // common messages
  static const char M00[] PROGMEM = "1234567890";
  static const char M01[] PROGMEM = "Prog start";
  static const char M02[] PROGMEM = "WIFI ct";
  static const char M03[] PROGMEM = "Device IP:";
  static const char M04[] PROGMEM = "IP 192.168.7.11";
  static const char M05[] PROGMEM = "Hello";
  static const char M06[] PROGMEM = "wait forIP";
#endif  //OledConfig_h