/***************************************************
  This is a library for the Adafruit 1.8" SPI display. This library has
  been modified from the original version to work with a non-Adafruit
  1.8in display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ---->  http://www.adafruit.com/products/358

The 1.8" TFT shield
  ---->  http://www.adafruit.com/products/358

The 1.44" TFT breakout
  ---->  http://www.adafruit.com/products/358

as well as Adafruit raw 1.8" TFT display
  ---->  http://www.adafruit.com/products/358


  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
  
 ****************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <EEPROM.h>
//#include <Fonts/FreeSerif9pt7b.h>
//#include <Fonts/FreeMono9pt7b.h>
//#include <Fonts/FreeSans9pt7b.h>
//#include <Fonts/Picopixel.h>
//#include <Fonts/Tiny3x3a2pt7b.h>

// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_RST    7//D6 //7 // TFT PIN 1 you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_CS     8//D8 //9 // TFT PIN 2
#define TFT_DC     9//D2 //8 // TFT PIN 3
enum UPDATESCREEN { FULL, PARTITIAL };
enum DEVICEMODES { CAMERA, FLASH };
enum SENSORMODES { LIGHT_POS, LIGHT_NEG, SOUND };
static const char *SENSORMODE_STRING[] = {
    "LIGHT+", "LIGHT-", "SOUND"
};
enum EDITMODES { NONE, LEVEL, F_DELAY, S_DELAY, ON, SENSOR, DEVMODE, MODE };
enum countMode { ROTATE, LIMITED };
DEVICEMODES CurrDeviceMode;
DEVICEMODES OldCurrDeviceMode;
EDITMODES CurrEditMode;
EDITMODES OldCurrEditMode;
//SENSORMODES CurrSensorMode;
EDITMODES ModeChange;
struct MyObject {
  int Level;
  unsigned long FirstDelay;
  unsigned long SecondDelay;  
  unsigned long OnTime;
  SENSORMODES SensorMode;
};

MyObject Values[2];
MyObject OldValues;
const int lightSensorPin = A0;    // select the input pin for the light sensor
const int soundSensorPin = A1;    // select the input pin for the sound input
const int modePin = A2;    // select the input pin for the sound input
const int button1Pin = 6;
const int button2Pin = 5;
const int button3Pin = 12;
const int output1Pin = 3;
const int output2Pin = 4;
const int laserPin = 2;

unsigned long button1High;
unsigned long button2High;
unsigned long button3High;

int sensorValue = 0;  // variable to store the value coming from the sensor
int sensorValueOld = 0;  // variable to store the value coming from the sensor

//int LevelOld=0;
int plotSensVal = 0;
int plotLevelVal = 0;
int plotLevelValOld = 0;
int viewStart = 0;
int viewHeight = 0;
int sensLevel = 100;
int button1State = 0;
int button2State = 0;
int button3State = 0;
int button1StateOld = 0;
int button2StateOld = 0;
int button3StateOld = 0;
#define TEXT_DELAY  (500);
unsigned long startTrigger = 0;
unsigned long triggerDelay = 0;
String strValue = "";
char arChar[10] ="";
int invertDetection = 1;

#define BUTTON_INTERVAL 1000
#define SECOND_RELAY_DELAY (10)
int SecondRelayDelay = SECOND_RELAY_DELAY;

//int FirstDelayOld = 1;
#define LEVEL_ADJUST
#define TRIGGER_DELAY
#define FLASH_DURATION

// Defines for line numbers
#define FIRSTLINE (20)
#define SECONDLINE (35)
#define HEADER (2)

// Defines for plotarea Sensor
#define START_Y (60)
#define END_Y (158)

// Defines for big value erase
#define BIG_VALUE_X 44
#define BIG_TYPE_X  17

// Defines for posioning
#define SMALL_TXT_HEIGHT  9

// Defines of all shown text
#define SHORT_LEVEL_LIGHT  "LL"
#define SHORT_LEVEL_SOUND  "SL"
#define SHORT_FIRST_DELAY  "FD"
#define SHORT_SECOND_DELAY "SD"
#define SHORT_ON_TIME      "ON"
#define SHORT_SENSOR       "IN"
#define SHORT_MODE         "MD"

#define TXT_LIGHT          "LIGHT"
#define TXT_SOUND          "SOUND"
#define TXT_CAMERA         "CAMERA"
#define TXT_FLASH          "FLASH"

#define NO_BUTTON_TIME     10000

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Option 2: use any pins but a little slower!
#define TFT_SCLK 13   // TFT PIN 5 set these to be whatever pins you like!
#define TFT_MOSI 11   // TFT PIN 6 set these to be whatever pins you like!
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
int ledPin = 10;
float p = 3.1415926;
unsigned char graph[128] ="";
unsigned char xPos =0;


enum dir { INCREASE, DECREASE };

void setup(void) {
  ledOff();
  tft.setFont();
  Serial.begin(115200);
  pinMode (ledPin, OUTPUT);
  pinMode( button1Pin, INPUT);
  pinMode( button2Pin, INPUT);
  pinMode( button3Pin, INPUT);
  pinMode(output1Pin, OUTPUT);
  pinMode(output2Pin, OUTPUT);
  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, HIGH);
  while(!Serial);
  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);
  // large block of text
  tft.fillScreen(ST7735_WHITE);
  // Draw knobs +- tekst
  tft.drawRect( 0, 1, 15, 15, ST7735_BLACK);
  tft.drawRect(20, 1, 15, 15, ST7735_BLACK);
  tft.drawRect( 2, 7, 11,  2, ST7735_RED);
  tft.drawRect(22, 7, 11,  2, ST7735_RED);
  tft.drawRect(27, 3,  2, 11, ST7735_RED);
  tft.drawRect( 0, 18, 15, 15, ST7735_BLACK);
  tft.setTextSize(2);
  testdrawtext(2, 19, "M", ST7735_RED);
  plotLevelVal = map( Values[CurrDeviceMode].Level,0,1023, START_Y, END_Y );
  tft.drawLine(1, plotLevelVal, tft.width() - 2, plotLevelVal, ST7735_RED);
  plotLevelValOld = plotLevelVal;
  EEPROM.get(0,Values);
  CurrDeviceMode = CAMERA;
  OldCurrDeviceMode = CAMERA;
  CurrEditMode = LEVEL;
  OldCurrEditMode = LEVEL;
  tft.setTextSize(2);
  EEPROM.put(0,Values);
  ModeChange = LEVEL;
  ShowMode(FULL);
  //LevelOld = Values[CurrEditMode].Level;
  //FirstDelayOld = Values[CurrEditMode].FirstDelay;
  memcpy( &OldValues, &Values, sizeof(MyObject));
  //tft.fillScreen(ST7735_ORANGE );
}

void ledOn() {
  digitalWrite(ledPin,LOW);
}

void ledOff() {
  digitalWrite(ledPin,HIGH);
}

void FirstRelayOn()
{
  digitalWrite(output1Pin, HIGH);
}
void FirstRelayOff()
{
  digitalWrite(output1Pin, LOW);
}
void SecondRelayOn()
{
  digitalWrite(output2Pin, HIGH);
}
void SecondRelayOff()
{
  digitalWrite(output2Pin, LOW);
}
int CalcIncrement(unsigned long buttonHigh)
{
  int retVal = 0;
  unsigned long buttOn = millis() - buttonHigh ;
  if (buttOn >  4000) return 500;
  if (buttOn >  2000) return 50;
  if (buttOn >   200) return 5;
  if ((buttOn >   11) && (buttOn < 200)) return 0;
  if (buttOn >   10) return 1;
  return retVal;
}


int CheckButton(int button, unsigned long buttonHigh, EDITMODES currMode, dir modifier )
{
  UPDATESCREEN retVal = PARTITIAL;
  int multFactor = modifier == INCREASE ? 1 : -1;
  int value = 0;
  int modValue = 0;
  int minValue; 
  int maxValue;
  countMode cntMode;
  if (button == LOW) return button;
  // First get the values
  switch(currMode)
  {
    case LEVEL:
      value = Values[CurrDeviceMode].Level;
      minValue = 0;
      maxValue = 1023;
      cntMode = LIMITED;
      break;
    case F_DELAY:
      value = Values[CurrDeviceMode].FirstDelay;
      minValue = 0;
      maxValue = 10000;
      cntMode = LIMITED;
      break;
    case S_DELAY:
      value = Values[CurrDeviceMode].SecondDelay;
      minValue = 0;
      maxValue = 10000;
      cntMode = LIMITED;
      break;
    case ON:
      value = Values[CurrDeviceMode].OnTime;
      minValue = 0;
      maxValue = 10000;
      cntMode = LIMITED;
      break;
    case SENSOR:
      value = Values[CurrDeviceMode].SensorMode;
      minValue = LIGHT_POS;
      maxValue = SOUND;
      cntMode = ROTATE;
      break;
    case DEVMODE:
      value = CurrDeviceMode;
      minValue = CAMERA;
      maxValue = FLASH;
      cntMode = ROTATE;
      break;
  }
  if (button == HIGH)
  {
    modValue = value + CalcIncrement(buttonHigh) * multFactor;
  }
  switch (modifier)
  {
    case INCREASE:
      switch( cntMode )
      {
        case ROTATE:
          modValue = modValue > maxValue ? minValue : modValue;
        break;
        case LIMITED:
          modValue = modValue > maxValue ? maxValue : modValue;
        break;
      }
      break;
    case DECREASE:
      switch( cntMode )
      {
        case ROTATE:
          modValue = modValue < minValue ? maxValue : modValue;
        break;
        case LIMITED:
          modValue = modValue < minValue ? minValue : modValue;
        break;
      }
      break;
  }

  if ( value != modValue )
  {
    ModeChange = currMode;
    // Put value back
    switch(currMode)
    {
      case LEVEL:
        Values[CurrDeviceMode].Level = modValue;
        break;
      case F_DELAY:
        Values[CurrDeviceMode].FirstDelay = modValue;
        break;
      case S_DELAY:
        Values[CurrDeviceMode].SecondDelay = modValue;
        break;
      case ON:
        Values[CurrDeviceMode].OnTime = modValue;
        break;
      case SENSOR:
        Values[CurrDeviceMode].SensorMode = modValue;
        retVal = FULL;
        break;
      case DEVMODE:
        CurrDeviceMode = modValue;
        retVal = FULL;
        break;
    }
    EEPROM.put(0,Values);
  }
  return retVal;
  
}

int NextEdit(int button, unsigned long buttonHigh)
{
  unsigned long buttOn;
  if (button == HIGH)
  {
    buttOn = millis() - buttonHigh;
    if ((buttOn >   11) && (buttOn < 500)) return 0;
    if (buttOn >   10)
    {
      CurrEditMode = CurrEditMode + 1;
      CurrEditMode = CurrEditMode > DEVMODE ? LEVEL : CurrEditMode;
      ModeChange = CurrEditMode;
      ClearScreen();
      ShowMode(FULL);
      //ModeChange = NONE;
    }
  }
}

/*    SSS  H   H  OOO  W   W M   M  OOO  DDDD  EEEEE  
 *   S   S H   H O   O W   W MM MM O   O D   D E
 *   S     H   H O   O W W W M M M O   O D   D E 
 *    SSS  HHHHH O   O W W W M   M O   O D   D EEE
 *       S H   H O   O W W W M   M O   O D   D E
 *   S   S H   H O   O W W W M   M O   O D   E E
 *    SSS  H   H  OOO   W W  M   M  OOO  DDDD  EEEEE
 */
void ShowMode(UPDATESCREEN updateType)
{
  if (ModeChange == NONE) return;


#ifdef JOSJOS
  switch(ModeChange)
  {
    case LEVEL:
      for (int16_t y=SECONDLINE-1; y < SECONDLINE + SMALL_TXT_HEIGHT; y+=1)
      {
        tft.drawFastHLine(18, y, (63-18), ST7735_WHITE);
      }
    break;
    case F_DELAY:
      for (int16_t y=SECONDLINE-1; y < SECONDLINE + SMALL_TXT_HEIGHT; y+=1)
      {
        tft.drawFastHLine(83, y, (63-18), ST7735_WHITE);
      }
    break;
    case S_DELAY:
      for (int16_t y=SECONDLINE + SMALL_TXT_HEIGHT; y < SECONDLINE + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT; y+=1)
      {
        tft.drawFastHLine(18, y, (63-18), ST7735_WHITE);
      }
    break;
    case ON:
      for (int16_t y=SECONDLINE + SMALL_TXT_HEIGHT; y < SECONDLINE + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT; y+=1)
      {
        tft.drawFastHLine(81, y, (63-18), ST7735_WHITE);
      }
    break;
    case SENSOR:
      for (int16_t y=SECONDLINE + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT; y < SECONDLINE + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT; y+=1)
      {
        tft.drawFastHLine(SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT, y, (63-SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT), ST7735_WHITE);
      }
    break;
    case DEVMODE:
      for (int16_t y=SECONDLINE + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT; y < SECONDLINE + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT; y+=1)
      {
        tft.drawFastHLine(81, y, (63-18), ST7735_WHITE);
      }
    break;
  }
#endif
    switch(CurrEditMode)
    {
      case LEVEL:
        if ((updateType == FULL ) || (Values[CurrDeviceMode].Level != OldValues.Level))
        {
          ClearValue();
          tft.setTextSize(2);
          tft.setCursor(BIG_VALUE_X, FIRSTLINE);
          tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
          tft.println(Values[CurrDeviceMode].Level); 
          OldValues.Level = Values[CurrDeviceMode].Level;
        }
      break;
      case F_DELAY:
        if ((updateType == FULL ) || (Values[CurrDeviceMode].FirstDelay != OldValues.FirstDelay) )
        {
        ClearValue();
        tft.setTextSize(2);
        tft.setCursor(BIG_VALUE_X, FIRSTLINE);
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE); 
        tft.print(Values[CurrDeviceMode].FirstDelay); 
        tft.print("ms"); 
        OldValues.FirstDelay = Values[CurrDeviceMode].FirstDelay;
        }
      break;
      case S_DELAY:
        if ((updateType == FULL ) || (Values[CurrDeviceMode].SecondDelay != OldValues.SecondDelay) )
        {
          ClearValue();
        
        tft.setTextSize(2);
        tft.setCursor(BIG_VALUE_X, FIRSTLINE);
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE); 
        tft.print(Values[CurrDeviceMode].SecondDelay); 
        tft.print("ms"); 
        OldValues.SecondDelay = Values[CurrDeviceMode].SecondDelay;
        }
      break;
      case ON:
        if ((updateType == FULL ) || (Values[CurrDeviceMode].OnTime != OldValues.OnTime) )
        {
        ClearValue();
        tft.setTextSize(2);
        tft.setCursor(BIG_VALUE_X, FIRSTLINE);
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE); 
        tft.print(Values[CurrDeviceMode].OnTime); 
        tft.print("ms"); 
        OldValues.OnTime = Values[CurrDeviceMode].OnTime;
        }
      break;
      case SENSOR:
        if ((updateType == FULL ) || (Values[CurrDeviceMode].SensorMode != OldValues.SensorMode) )
        {
        ClearValue();
        tft.setTextSize(2);
        tft.setCursor(BIG_VALUE_X, FIRSTLINE);
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE); 
        tft.print(SENSORMODE_STRING[Values[CurrDeviceMode].SensorMode]); 
        OldValues.SensorMode = Values[CurrDeviceMode].SensorMode;
        }
      break;
      case DEVMODE:
        if ((updateType == FULL ) || (CurrDeviceMode != OldCurrDeviceMode) )
        {
          ClearScreen();
        ClearValue();
        tft.setTextSize(2);
        tft.setCursor(BIG_VALUE_X, FIRSTLINE);
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE); 
        tft.print(CurrDeviceMode==CAMERA?TXT_CAMERA:TXT_FLASH); 
        OldCurrDeviceMode = CurrDeviceMode;
        }
      break;
    }
  tft.setTextSize(2);
  
  switch(CurrEditMode)
  {
    case LEVEL:
      testdrawtext(BIG_TYPE_X, FIRSTLINE, Values[CurrDeviceMode].SensorMode==SOUND?SHORT_LEVEL_SOUND:SHORT_LEVEL_LIGHT, ST7735_BLUE);
      break;
    case F_DELAY:
      testdrawtext(17, FIRSTLINE, SHORT_FIRST_DELAY, ST7735_BLUE);
      break;
    case S_DELAY:
      testdrawtext(17, FIRSTLINE, SHORT_SECOND_DELAY, ST7735_BLUE);
      break;
    case ON:
      testdrawtext(17, FIRSTLINE, SHORT_ON_TIME, ST7735_BLUE);
      break;
    case SENSOR:
      testdrawtext(17, FIRSTLINE, SHORT_SENSOR, ST7735_BLUE);
      break;
    case DEVMODE:
      testdrawtext(17, FIRSTLINE, SHORT_MODE, ST7735_BLUE);
      break;
  }
    tft.setTextSize(1);
  testdrawtext(50, HEADER, CurrDeviceMode == CAMERA ? TXT_CAMERA : TXT_FLASH, ST7735_BLUE);
  testdrawtext(90, HEADER, Values[CurrDeviceMode].SensorMode == SOUND ? TXT_SOUND : TXT_LIGHT, ST7735_BLUE);

  tft.setTextSize(1);
  tft.setTextColor(ST7735_BLUE, ST7735_WHITE);
  tft.setCursor(1,SECONDLINE);
  tft.print(Values[CurrDeviceMode].SensorMode==SOUND?SHORT_LEVEL_SOUND:SHORT_LEVEL_LIGHT);
  tft.print(" ");
  tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
  tft.print(Values[CurrDeviceMode].Level);
  tft.setTextColor(ST7735_BLUE, ST7735_WHITE);
  tft.setCursor(63,SECONDLINE);
  tft.print(SHORT_FIRST_DELAY);
  tft.print(" ");
  tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
  tft.print(Values[CurrDeviceMode].FirstDelay);
  tft.print("ms");
  tft.setTextColor(ST7735_BLUE, ST7735_WHITE);
  tft.setCursor(1,SECONDLINE+SMALL_TXT_HEIGHT);
  tft.print(SHORT_SECOND_DELAY);
  tft.print(" ");
  tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
  tft.print(Values[CurrDeviceMode].SecondDelay);
  tft.print("ms");
  tft.setTextColor(ST7735_BLUE, ST7735_WHITE);
  tft.setCursor(63,SECONDLINE+SMALL_TXT_HEIGHT);
  tft.print(SHORT_ON_TIME);
  tft.print(" ");
  tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
  tft.print(Values[CurrDeviceMode].OnTime);
  tft.print("ms");
  tft.setTextColor(ST7735_BLUE, ST7735_WHITE);
  tft.setCursor(1,SECONDLINE+SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT);
  tft.print(SHORT_SENSOR);
  tft.print(" ");
  tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
  tft.print(SENSORMODE_STRING[Values[CurrDeviceMode].SensorMode]);
  tft.setCursor(63,SECONDLINE+SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT);
  tft.setTextColor(ST7735_BLUE, ST7735_WHITE);
  tft.print(SHORT_MODE);
  tft.print(" ");
  tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
  tft.print(CurrDeviceMode==CAMERA?TXT_CAMERA:TXT_FLASH);
  ModeChange = NONE;
}

/*****************************************************************
 * L      OOO   OOO  PPPP
 * L     O   O O   O P   P
 * L     O   O O   O P   P
 * L     O   O O   O PPPP
 * L     O   O O   O P
 * L     O   O O   O P
 * LLLLL  OOO   OOO  P
 */
UPDATESCREEN button1Retval;
UPDATESCREEN button2Retval;

int updateSensor = 0;
int noButtonPressed = NO_BUTTON_TIME;
unsigned long currTime;
void loop() 
{
  button1State = digitalRead(button1Pin);
  if (button1State != button1StateOld)
  {
    button1High = millis();
  }
  else
  {
    if (button1State == LOW )
    {
      button1High = 0;
    }
  }
  button1StateOld = button1State;
  button2State = digitalRead(button2Pin);
  if (button2State != button2StateOld)
  {
    button2High = millis();
  }
  else
  {
    if (button2State == LOW )
    {
      button2High = 0;
    }
  }
  button2StateOld = button2State;
  
  button3State = digitalRead(button3Pin);
  if (button2State == HIGH)
  {
    noButtonPressed = 0;
  }
  if (button3State != button3StateOld)
  {
    button3High = millis();
  }
  else
  {
    if (button3State == LOW )
    {
      button3High = 0;
    }
  }
  button3StateOld = button3State;
  if ((button1State == HIGH) || (button2State == HIGH)|| (button3State == HIGH))
  {
    noButtonPressed = NO_BUTTON_TIME;
    //tft.fillScreen(ST7735_WHITE);
  }
  
  NextEdit(button3State, button3High);
  button1Retval = CheckButton( button1State, button1High, CurrEditMode, DECREASE);
  button2Retval = CheckButton( button2State, button2High, CurrEditMode, INCREASE); 
  //Sensor part
  sensorValue = analogRead(Values[CurrDeviceMode].SensorMode==SOUND?soundSensorPin:lightSensorPin);
  // Check if GUI must not be updated
  if (noButtonPressed > 0)
  {
    if ((button1Retval == FULL) || (button2Retval == FULL))
    {
      ShowMode(FULL);
    }
    else
    {
      ShowMode(PARTITIAL);
    }

    //Sensor part
    if (updateSensor > 100 )
    {
      if (sensorValueOld != sensorValue)
      {
      //for (int16_t y=SECONDLINE + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT; y < SECONDLINE + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT; y+=1)
      //{
      //  tft.drawFastHLine(SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT, y, (63-SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT), ST7735_WHITE);
      //}
      //tft.setTextColor(ST7735_BLUE,ST7735_WHITE);
      tft.setTextSize(2);
      tft.setCursor(1,SECONDLINE+SMALL_TXT_HEIGHT+SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT);
      tft.setTextColor(ST7735_BLACK,ST7735_WHITE);
//      tft.print(sensorValue);
      tft.print(noButtonPressed);
      tft.print("          ");
      }
      updateSensor = 0;
    }
    updateSensor = updateSensor + 1;
  
    plotSensVal = map( 1023-sensorValue, 0, 1023, START_Y, END_Y );
    plotLevelVal = map( 1023-Values[CurrDeviceMode].Level,0,1023,START_Y, END_Y );
    if (plotLevelVal != plotLevelValOld) 
    {
      //Erase line
      tft.drawLine(1, plotLevelValOld, tft.width() - 2, plotLevelValOld, ST7735_WHITE);
      //Draw line
      tft.drawLine(1, plotLevelVal, tft.width() - 2, plotLevelVal, CurrDeviceMode==CAMERA?ST7735_MAGENTA:ST7735_BLUE);
      plotLevelValOld = plotLevelVal;
    }
    if ( graph[xPos] == plotLevelVal )
    {
      tft.drawPixel(xPos, graph[xPos], CurrDeviceMode==CAMERA?ST7735_MAGENTA:ST7735_BLUE); // restore line
    }
    else
    {
      tft.drawPixel(xPos, graph[xPos], ST7735_WHITE);
    }
    graph[xPos] = plotSensVal;
    tft.drawPixel(xPos, graph[xPos], ST7735_BLACK);
    xPos = ((xPos >= 127)? xPos = 0 : xPos+1);
  }
  else
  {
    if (noButtonPressed == 0)
    {
      //tft.fillScreen(ST7735_BLACK);
      tft.setTextSize(2);
      tft.setCursor(1,SECONDLINE+SMALL_TXT_HEIGHT+SMALL_TXT_HEIGHT + SMALL_TXT_HEIGHT);
      tft.setTextColor(ST7735_RED,ST7735_WHITE);
      tft.print("Triggering");
      noButtonPressed = -1;
    }  
  }
  if (startTrigger == 0 )
  {
    //SENSORMODE_STRING[Values[CurrDeviceMode].SensorMode]
    switch(Values[CurrDeviceMode].SensorMode)
    {
      case LIGHT_POS:
      case SOUND:
        if ((sensorValue >= Values[CurrDeviceMode].Level) && ( sensorValueOld < Values[CurrDeviceMode].Level )) 
        {
          startTrigger = 1;
        }
        if ( startTrigger == 1 )
        {
            triggerDelay = millis();
        }
      break;
      case LIGHT_NEG:
        if ((sensorValue <= Values[CurrDeviceMode].Level) && ( sensorValueOld > Values[CurrDeviceMode].Level))
        {
          startTrigger = 1;
        }
        if ( startTrigger == 1 )
        {
            triggerDelay = millis();
        }
      break;
      
    }
        switch(Values[CurrDeviceMode].SensorMode)
    {
      case SOUND:
        digitalWrite(laserPin, LOW);
      break;
      case LIGHT_POS:
      case LIGHT_NEG:
        digitalWrite(laserPin, HIGH);
      break;
      
    }

  }
  
  if (startTrigger == 1 )
  {
    currTime = millis() - triggerDelay;
    Serial.println(currTime);
    if (currTime > Values[CurrDeviceMode].FirstDelay)
    {
      ledOn();
      FirstRelayOn();
      if (currTime > (Values[CurrDeviceMode].SecondDelay))
      {
        SecondRelayOn();
      }
      if (currTime > (Values[CurrDeviceMode].OnTime + Values[CurrDeviceMode].FirstDelay) )
      {
        ledOff();
        SecondRelayOff();
        FirstRelayOff();
        startTrigger = 0;
      }
    }
  }
  sensorValueOld = sensorValue;
//  delayMicroseconds(200);
  delayMicroseconds(1);
  if (noButtonPressed > 0)
  {
    noButtonPressed = noButtonPressed - 1;
  }
}

void ClearValue()
{
  for (int16_t y=FIRSTLINE; y < FIRSTLINE + 14; y+=1)
  {
    tft.drawFastHLine(BIG_VALUE_X, y, tft.width()-BIG_VALUE_X, ST7735_WHITE);
  }
}

void ClearScreen()
{
  for (int16_t y=20; y < 60; y+=1) 
  {
    tft.drawLine(BIG_TYPE_X, y, tft.width(),y, ST7735_WHITE);
  }
  for (int16_t y=0; y < 10; y+=1) 
  {
    tft.drawLine(40, y, tft.width()-40,y, ST7735_WHITE);
  }
}

void drawCamera(int x, int y)
{
  tft.drawRect(  x+0, y+0, 19, 13, ST7735_BLACK);
  tft.drawCircle(x+9, y+6,  4, ST7735_BLACK);
}

void drawFlash(int x, int y)
{
    tft.drawLine(x+11, y+0, x+3, y+ 9, ST7735_BLACK);
    tft.drawLine(x+ 3, y+9, x+6, y+ 6, ST7735_BLACK);
    tft.drawLine(x+ 6, y+6, x+0, y+18, ST7735_BLACK);
  
}

void testlines(uint16_t color) {
  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, 0, x, tft.height()-1, color);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, 0, tft.width()-1, y, color);
  }

  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, 0, x, tft.height()-1, color);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, 0, 0, y, color);
  }

  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, tft.height()-1, x, 0, color);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, tft.height()-1, tft.width()-1, y, color);
  }

  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, x, 0, color);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, 0, y, color);
  }
}

void drawTestGrid(int hor, int vert){
  tft.fillScreen(ST7735_WHITE);
  //Draw vertical lines
  for (int16_t x=0; x < tft.width(); x+=vert){
    tft.drawFastVLine(x, 0, tft.height(), ST7735_RED);
  }
  for (int16_t y=0; y < tft.height(); y+=hor){
    tft.drawFastHLine(0, y, tft.width(), ST7735_BLACK);
  }
}

void testdrawtext(int wid, int hei, char *text, uint16_t color) {
  tft.setCursor(wid, hei);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST7735_BLACK);
  for (int16_t y=0; y < tft.height(); y+=5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x=0; x < tft.width(); x+=5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}

void testdrawrects(uint16_t color) {
  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST7735_BLACK);
  for (int16_t x=tft.width()-1; x > 6; x-=6) {
    tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=radius; x < tft.width(); x+=radius*2) {
    for (int16_t y=radius; y < tft.height(); y+=radius*2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < tft.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < tft.height()+radius; y+=radius*2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  tft.fillScreen(ST7735_BLACK);
  int color = 0xF800;
  int t;
  int w = tft.width()/2;
  int x = tft.height()-1;
  int y = 0;
  int z = tft.width();
  for(t = 0 ; t <= 15; t+=1) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  tft.fillScreen(ST7735_BLACK);
  int color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = tft.width()-2;
    int h = tft.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
}
void testdrawScreenData(){
  tft.setCursor(0,20);
  tft.println("Screen Data:");
  tft.print("Screen Width: ");
  tft.println(tft.width());
  tft.print("Screen Height: ");
  tft.println(tft.height());
  
}
void testdrawCountdown(){
  tft.setTextWrap(true);
  tft.fillScreen(ST7735_RED);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(6);
  tft.setCursor(30, 50);
  for (int i=10; i>0; i--){
    tft.println(i);
    delay(1000);
    tft.setCursor(50, 50);
    tft.fillScreen(ST7735_RED);
  }
}

void circlePass(){
  tft.fillScreen(ST7735_BLACK);
  testfillcircles(5, ST7735_GREEN);
  delay(1000);
  testfillcircles(4, ST7735_RED);
  delay(1000);
  testfillcircles(2, ST7735_BLACK);
  testfillcircles(3, ST7735_YELLOW);
  delay(500);
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST7735_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST7735_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST7735_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST7735_WHITE);
  tft.print(" seconds.");
}

void mediabuttons() {
  // play
  tft.fillScreen(ST7735_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST7735_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST7735_RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST7735_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST7735_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST7735_GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST7735_BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST7735_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST7735_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST7735_GREEN);
}
