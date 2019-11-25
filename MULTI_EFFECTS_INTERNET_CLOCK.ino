/*
// REAL PIN ON NODEMCU
#define RowA3_Pin         D0  // A PIN - 74HC154
#define RowA2_Pin         D1  // B PIN - 74HC154
#define RowA1_Pin         D2  // C PIN - 74HC154
#define RowA0_Pin         D3  // D PIN - 74HC154

#define LATCH_Pin         D4  // LATCH PIN - TPIC6B595
#define CLOCK_Pin         D5  // CLOCK PIN - TPIC6B595
#define BUTTON            D6  // PUSH BUTTON
#define DATA_Pin          D7  // DATA PIN - TPIC6B595

#define BLANK_Pin         D8  // BLANK PIN - TPIC6B595
*/
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include "timezone.h"
#include "font8x8.h"
#include "font8x16.h"

#define RowA3_Pin     16  // GPIO16
#define RowA2_Pin     5   // GPIO5
#define RowA1_Pin     4   // GPIO4
#define RowA0_Pin     0   // GPIO0 

#define BLANK_Pin     15  // GPIO15  
#define DATA_Pin      13  // GPIO13   
#define CLOCK_Pin     14  // GPIO14  
#define LATCH_Pin     2   // GPIO2 

#define BUTTON        12  // GPIO12

#define ANI_RIGHT     0
#define ANI_LEFT      1
#define ANI_DOWN      2
#define ANI_UP        3
#define LEFT_LEFT     4

const char *WIFI_NETWORK_NAME = "FTP-XXXXXX"; // Change to your wifi network name
const char *WIFI_PASSWORD     = "xxxxxxxx";   // Change to your wifi password

const char *TIME_SERVER       = "asia.pool.ntp.org";
int myTimeZone = VST; // change this to your time zone (see in timezone.h)

time_t now;

unsigned long samplingtime  = 0;
unsigned long samplingtime1 = 0;
unsigned long samplingtime2 = 0;
unsigned long samplingtime3 = 0;

unsigned long samplingtimes0 = 0;
unsigned long samplingtimes1 = 0;
unsigned long samplingtimem0 = 0;
unsigned long samplingtimem1 = 0;
unsigned long samplingtimeh0 = 0;
unsigned long samplingtimeh1 = 0;
byte row, level;

byte matrixBuffer[48];

void DIY_SPI(uint8_t DATA);

void ICACHE_RAM_ATTR timer1_ISR(void);   
void LED(uint8_t X ,uint8_t Y, bool ON);
void clearscreen();
void rowscan(byte row);
byte getPixelChar(uint8_t x, uint8_t y, char ch);
byte getPixelHString(uint16_t x, uint16_t y, char *p);
unsigned int lenString(char *p);
int checkConstrains(int value, int min, int max);
void printChar(uint8_t x, uint8_t y, bool For_color, bool Bk_color, char ch);
void hScroll(uint8_t y, bool For_color, bool Bk_color, char *mystring, uint8_t delaytime, uint8_t times, uint8_t dir);

void xyfillRectangle(uint8_t xoffset,  uint8_t yoffset, uint8_t y1, uint8_t x1, uint8_t y2, uint8_t x2, bool state);
void EraserHandwritten(uint8_t xoffset, uint8_t yoffset, bool state, int delaytime);
void DrawDigit(byte dots[][2], byte siz, byte xoffset, byte yoffset, bool state, int delaytime);
void HandwrittenNumber(byte xoffset, byte yoffset, byte number, bool state, int delaytime);
void drawImage(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color);

void HScrollImageR(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color);
void HScrollImageL(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color);
void HScrollImageLL(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color);
void VScrollImageU(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color);
void VScrollImageD(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color);
void ScrollNumber(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, char number, bool For_color, bool Bk_color, uint16_t scrolltype);
void EraseShowNumber(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, char previous_number, char current_number, bool For_color, bool Bk_color, uint16_t animation);

void MutiEffect_S0();
void MutiEffect_S1();
void MutiEffect_M0();
void MutiEffect_M1();
void MutiEffect_H0();
void MutiEffect_H1();

void SCROLLYYYYMMDDHHMMSS();

void FillDot();
void ReadButton();

char s0, s1, m0, m1, h0, h1;
char prves0, prves1, prvem0, prvem1, prveh0, prveh1;

bool dotstate=0;

char YYYYMMDD[34];
char HHMMSS[32];

#define dist(a, b, c, d) sqrt(double((a - c) * (a - c) + (b - d) * (b - d)))

int buttonPushCounter = 0;   
int buttonState = 0;         
int lastButtonState = 0; 
int animation = 0;
static const uint8_t zero[]PROGMEM  = { B01111000, B11001100, B11011100, B11111100, B11101100, B11001100, B01111000, B00000000 }; // 0
static const uint8_t one[]PROGMEM   = { B00110000, B11110000, B00110000, B00110000, B00110000, B00110000, B11111100, B00000000 }; // 1
static const uint8_t two[]PROGMEM   = { B01111000, B11001100, B00001100, B00111000, B01100000, B11001100, B11111100, B00000000 }; // 2
static const uint8_t three[]PROGMEM = { B01111000, B11001100, B00001100, B00111000, B00001100, B11001100, B01111000, B00000000 }; // 3
static const uint8_t four[]PROGMEM  = { B00011100, B00111100, B01101100, B11001100, B11111110, B00001100, B00001100, B00000000 }; // 4
static const uint8_t five[]PROGMEM  = { B11111100, B11000000, B11111000, B00001100, B00001100, B11001100, B01111000, B00000000 }; // 5
static const uint8_t six[]PROGMEM   = { B00111000, B01100000, B11000000, B11111000, B11001100, B11001100, B01111000, B00000000 }; // 6
static const uint8_t seven[]PROGMEM = { B11111100, B11001100, B00001100, B00011000, B00110000, B01100000, B01100000, B00000000 }; // 7
static const uint8_t eight[]PROGMEM = { B01111000, B11001100, B11001100, B01111000, B11001100, B11001100, B01111000, B00000000 }; // 8
static const uint8_t nine[]PROGMEM  = { B01111000, B11001100, B11001100, B01111100, B00001100, B00011000, B01110000, B00000000 }; // 9

byte Zero[][2]          = {{4, 0},{3, 0},{2, 0},{1, 0},{0, 1},{1, 1},{1, 2},{0, 2},{0, 3},{1, 3},{1, 4},{0, 4},{0, 5},{1, 5},{1, 6},{2, 6},{3, 6},{4, 6},{4, 5},{5, 5},{5, 4},{4, 4},{4, 3},{5, 3},{5, 2},{4, 2},{5, 1},{4, 1},{3, 2},{3, 3},{2, 3},{2, 4}}; // 0
byte One[][2]           = {{0, 1},{1, 1},{2, 0},{3, 0},{3, 1},{2, 1},{2, 2},{3, 2},{3, 3},{2, 3},{2, 4},{3, 4},{3, 5},{2, 5},{0, 6},{1, 6},{2, 6},{3, 6},{4, 6},{5, 6}}; // 1
byte Two[][2]           = {{1, 1},{0, 1},{1, 0},{2, 0},{3, 0},{4, 0},{5, 1},{4, 1},{4, 2},{5, 2},{4, 3},{3, 3},{2, 3},{2, 4},{1, 4},{1, 5},{0, 5},{0, 6},{1, 6},{2, 6},{3, 6},{4, 6},{5, 6},{5, 5},{4, 5}}; // 2
byte Three[][2]         = {{0, 1},{1, 1},{1, 0},{2, 0},{3, 0},{4, 0},{5, 1},{4, 1},{5, 2},{4, 2},{2, 3},{3, 3},{4, 3},{5, 4},{4, 4},{5, 5},{4, 5},{4, 6},{3, 6},{2, 6},{1, 6},{1, 5},{0, 5}}; // 3
byte Four[][2]          = {{3, 0},{3, 1},{2, 1},{2, 2},{1, 2},{1, 3},{0, 3},{0, 4},{1, 4},{2, 4},{3, 4},{6, 4},{4, 0},{5, 0},{5, 1},{4, 1},{4, 2},{5, 2},{4, 3},{5, 3},{4, 4},{5, 4},{4, 5},{5, 5},{4, 6},{5, 6}}; // 4
byte Five[][2]          = {{5, 0},{4, 0},{3, 0},{2, 0},{1, 0},{0, 0},{0, 1},{1, 1},{0, 2},{1, 2},{2, 2},{3, 2},{4, 2},{5, 3},{4, 3},{4, 4},{5, 4},{5, 5},{4, 5},{4, 6},{3, 6},{2, 6},{1, 6},{1, 5},{0, 5}}; // 5
byte Six[][2]           = {{4, 0},{3, 0},{2, 0},{2, 1},{1, 1},{0, 2},{1, 2},{1, 3},{0, 3},{0, 4},{1, 4},{1, 5},{0, 5},{1, 6},{2, 6},{3, 6},{4, 6},{4, 5},{5, 5},{5, 4},{4, 4},{4, 3},{3, 3},{2, 3}}; // 6
byte Seven[][2]         = {{0, 1},{1, 1},{1, 0},{2, 0},{3, 0},{4, 0},{5, 0},{4, 1},{5, 1},{5, 2},{4, 2},{4, 3},{3, 3},{3, 4},{2, 4},{1, 5},{2, 5},{2, 6},{1, 6}}; // 7
byte Eight[][2]         = {{4, 2},{5, 2},{5, 1},{4, 1},{4, 0},{3, 0},{2, 0},{1, 0},{1, 1},{0, 1},{0, 2},{1, 2},{1, 3},{2, 3},{3, 3},{4, 3},{4, 4},{5, 4},{5, 5},{4, 5},{4, 6},{3, 6},{2, 6},{1, 6},{1, 5},{0, 5},{0, 4},{1, 4}}; // 8
byte Nine[][2]          = {{3, 3},{2, 3},{1, 3},{1, 2},{0, 2},{0, 1},{1, 1},{1, 0},{2, 0},{3, 0},{4, 0},{4, 1},{5, 1},{5, 2},{4, 2},{4, 3},{5, 3},{5, 4},{4, 4},{4, 5},{3, 5},{3, 6},{2, 6},{1, 6}}; // 9
byte Delete[][2]        = {{0, 0},{0, 1},{1, 0},{3, 0},{2, 1},{1, 2},{0, 3},{0, 5},{1, 4},{2, 3},{3, 2},{4, 1},{5, 0},{7, 0},{6, 1},{5, 2},{4, 3},{3, 4},{2, 5},{1, 6},{0, 7},{2, 7},{3, 6},{4, 5},{5, 4},{6, 3},{7, 2},{7, 4},{6, 5},{5, 6},{4, 7},{6, 7},{7, 6}}; // DEL
byte Capital_H[][2]     = {{0, 0},{1, 0},{0, 1},{1, 1},{0, 2},{1, 2},{0, 3},{1, 3},{0, 4},{1, 4},{0, 5},{1, 5},{0, 6},{1, 6},{4, 0},{5, 0},{4, 1},{5, 1},{4, 2},{5, 2},{4, 3},{5, 3},{4, 4},{5, 4},{4, 5},{5, 5},{4, 6},{5, 6},{2, 3},{3, 3}}; // H
byte Capital_M[][2]     = {{1, 6},{0, 6},{0, 5},{1, 5},{1, 4},{0, 4},{0, 3},{1, 3},{1, 2},{0, 2},{1, 1},{0, 1},{0, 0},{1, 0},{2, 1},{2, 2},{3, 3},{3, 2},{4, 2},{4, 1},{5, 0},{6, 0},{6, 1},{5, 1},{5, 2},{6, 2},{6, 3},{5, 3},{5, 4},{6, 4},{6, 5},{5, 5},{5, 6},{6, 6}}; // M
byte Capital_S[][2]     = {{5, 1},{4, 1},{4, 0},{3, 0},{2, 0},{1, 0},{0, 1},{1, 1},{0, 2},{1, 2},{2, 2},{2, 3},{3, 3},{3, 4},{4, 3},{4, 4},{5, 4},{5, 5},{4, 5},{4, 6},{3, 6},{2, 6},{1, 6},{1, 5},{0, 5}}; // S

void setup () 
{    
   WiFi.begin(WIFI_NETWORK_NAME, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  configTime(3600*myTimeZone, 0, TIME_SERVER);

  while (now < EPOCH_1_1_2019)
  {
    now = time(nullptr);
    delay(500);
  }
  
  row = 0;
  level = 0;
  
  noInterrupts();
  
  pinMode(DATA_Pin, OUTPUT);
  pinMode(CLOCK_Pin, OUTPUT);
  pinMode(LATCH_Pin, OUTPUT);
  //pinMode(BLANK_Pin, OUTPUT);
  pinMode(RowA0_Pin, OUTPUT);
  pinMode(RowA1_Pin, OUTPUT);
  pinMode(RowA2_Pin, OUTPUT);
  pinMode(RowA3_Pin, OUTPUT);
  pinMode(BUTTON, INPUT);
  
  timer1_isr_init();
  timer1_attachInterrupt(timer1_ISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(500);
  interrupts();
  clearscreen();

  if (WiFi.status() == WL_CONNECTED)
    {
    hScroll(0, 1, 0, "     ESP8266 NODEMCU IS CONNECTED TO NTP SERVER...     ", 30, 1, 1);
    hScroll(0, 1, 0, "     LED MATRIX INTERNET CLOCK START...     ", 30, 1, 1);
    }
    clearscreen();
    hScroll(0, 1, 0, "     TIME FORMAT...     ", 30, 1, 1); 
    clearscreen();
    
    xyfillRectangle(0, 0, 7, 6, 8, 7, 1);
    xyfillRectangle(0, 0, 7, 14, 8, 15, 1);
    
    HandwrittenNumber(0, 0, 'H', 1, 25);
    HandwrittenNumber(0, 9, 'H', 1, 25); 
    HandwrittenNumber(8, 0, 'M', 1, 25);
    HandwrittenNumber(8, 9, 'M', 1, 25);
    HandwrittenNumber(16, 0, 'S', 1, 25);
    HandwrittenNumber(16, 9, 'S', 1, 25);
    delay(4000);
    EraserHandwritten(0,  0, 1, 20);
    EraserHandwritten(0,  9, 1, 20);
    EraserHandwritten(8,  0, 1, 20);
    EraserHandwritten(8,  9, 1, 20);
    EraserHandwritten(16, 0, 1, 20);
    EraserHandwritten(16, 9, 1, 20);
    
    xyfillRectangle(0, 0, 7, 6, 8, 7, 0);
    xyfillRectangle(0, 0, 7, 14, 8, 15, 0);
    
    clearscreen();
    hScroll(0, 1, 0, "     CHANGE CLOCK EFFECTS BY PUSH BUTTON .....      ", 30, 1, 1);
    hScroll(0, 1, 0, "     RESET TIME TO 00:00:00 .....      ", 30, 1, 1);
    clearscreen(); 
  
    xyfillRectangle(0, 0, 7, 6, 8, 7, 1);
    xyfillRectangle(0, 0, 7, 14, 8, 15, 1);
      
    HandwrittenNumber(0, 0, '0', 1, 15);    
    HandwrittenNumber(0, 9, '0', 1, 15);    
    HandwrittenNumber(8, 0, '0', 1, 15);
    HandwrittenNumber(8, 9, '0', 1, 15);
    HandwrittenNumber(16, 0, '0', 1, 15);
    HandwrittenNumber(16, 9, '0', 1, 15);
}    

void loop()
{
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    int year        = timeinfo->tm_year + 1900;
    int month       = timeinfo->tm_mon + 1;
    int day         = timeinfo->tm_mday;
    int hour        = timeinfo->tm_hour;
    int mins        = timeinfo->tm_min;
    int sec         = timeinfo->tm_sec;
    int day_of_week = timeinfo->tm_wday;
  
    YYYYMMDD[0] = ' ' ;
    YYYYMMDD[1] = ' ' ;
    YYYYMMDD[2] = ' ' ;
    YYYYMMDD[3] = ' ' ;
    YYYYMMDD[4] = ' ' ;
    YYYYMMDD[5] = ' ' ;
    YYYYMMDD[6] = ' ' ;
    YYYYMMDD[7] = ' ' ;
    YYYYMMDD[8] = 'D' ;
    YYYYMMDD[9] = 'a' ;
    YYYYMMDD[10] = 't' ;
    YYYYMMDD[11] = 'e' ;
    YYYYMMDD[12] = ':' ;
    YYYYMMDD[13] = ' ' ;
    YYYYMMDD[14] = ((year/1000) % 10) + 48 ;
    YYYYMMDD[15] = ((year/ 100) % 10) + 48 ;
    YYYYMMDD[16] = ((year/10) %10) + 48 ;
    YYYYMMDD[17] = (year %10) + 48 ;
    YYYYMMDD[18] = '-' ;
    YYYYMMDD[19] = ((month/10) %10) + 48 ;
    YYYYMMDD[20] = (month %10) + 48 ;
    YYYYMMDD[21] = '-' ;
    YYYYMMDD[22] = ((day/10) %10) + 48 ;
    YYYYMMDD[23] = (day %10) + 48 ;
    YYYYMMDD[24] = ' ' ;
    YYYYMMDD[25] = ' ' ;
    YYYYMMDD[26] = ' ' ;
    YYYYMMDD[27] = ' ' ;
    YYYYMMDD[28] = ' ' ;
    YYYYMMDD[29] = ' ' ;
    YYYYMMDD[30] = ' ' ;
    YYYYMMDD[31] = ' ' ;
    YYYYMMDD[32] = ' ' ;
    YYYYMMDD[33] = '\0' ;
  
    HHMMSS[0] = ' ' ;
    HHMMSS[1] = ' ' ;
    HHMMSS[2] = ' ' ;
    HHMMSS[3] = ' ' ;
    HHMMSS[4] = ' ' ;
    HHMMSS[5] = ' ' ;
    HHMMSS[6] = ' ' ;
    HHMMSS[7] = ' ' ;
    HHMMSS[8] = 'H' ;
    HHMMSS[9] = 'o' ;
    HHMMSS[10] = 'u' ;
    HHMMSS[11] = 'r' ;
    HHMMSS[12] = '-' ;
    HHMMSS[13] = ' ' ;
    HHMMSS[14] = ((hour/10) %10) + 48 ;
    HHMMSS[15] = (hour%10) + 48 ;
    HHMMSS[16] = ':' ;
    HHMMSS[17] = ((mins/10) %10) + 48 ;
    HHMMSS[18] = (mins%10) + 48 ;
    HHMMSS[19] = ':' ;
    HHMMSS[20] = ((sec/10) %10) + 48 ;
    HHMMSS[21] = (sec%10) + 48 ;
    HHMMSS[22] = ' ' ;
    HHMMSS[23] = ' ' ;
    HHMMSS[24] = ' ' ;
    HHMMSS[25] = ' ' ;
    HHMMSS[26] = ' ' ;
    HHMMSS[27] = ' ' ;
    HHMMSS[28] = ' ' ;
    HHMMSS[29] = ' ' ;
    HHMMSS[30] = ' ' ;
    HHMMSS[31] = '\0' ;

    ReadButton();
    FillDot();
    MutiEffect_S0();
    MutiEffect_S1();
    MutiEffect_M0();
    MutiEffect_M1();
    MutiEffect_H0();
    MutiEffect_H1();  
    SCROLLYYYYMMDDHHMMSS();
}

void LED(uint8_t X ,uint8_t Y, bool ON)
{
  uint8_t whichbyte = ((Y*3)+X/8);
  uint8_t whichbit = 7-(X % 8);
  if (ON)
  {
    bitWrite(matrixBuffer[whichbyte], whichbit, 1);
  }
  else
  {
    bitWrite(matrixBuffer[whichbyte], whichbit, 0);
  }
}

void clearscreen()
{
    for (uint8_t i=0; i<48; i++)
    {
      matrixBuffer[i]=0;
    }
}

void rowscan(byte row)
{
  if (row & 0x08)   digitalWrite(RowA3_Pin,HIGH);
    else            digitalWrite(RowA3_Pin,LOW);

  if (row & 0x04)   digitalWrite(RowA2_Pin,HIGH);   
    else            digitalWrite(RowA2_Pin,LOW);          

  if (row & 0x02)   digitalWrite(RowA1_Pin,HIGH);   
    else            digitalWrite(RowA1_Pin,LOW);          

  if (row & 0x01)   digitalWrite(RowA0_Pin,HIGH);   
    else            digitalWrite(RowA0_Pin,LOW);  
    
}  
void DIY_SPI(uint8_t DATA)
{
    for (uint8_t i = 0; i<8; i++)  
    {
      digitalWrite(DATA_Pin, !!(DATA & (1 << (7 - i))));
      digitalWrite(CLOCK_Pin,HIGH);
      digitalWrite(CLOCK_Pin,LOW);                
    }
}

void ICACHE_RAM_ATTR timer1_ISR(void)
{   
  digitalWrite(BLANK_Pin, HIGH);
  
  DIY_SPI((matrixBuffer[level + 0]));
  DIY_SPI((matrixBuffer[level + 1]));
  DIY_SPI((matrixBuffer[level + 2]));
  
  rowscan(row);
  
  digitalWrite(LATCH_Pin, HIGH);
  digitalWrite(LATCH_Pin, LOW);

  digitalWrite(BLANK_Pin, LOW);
  
  row++;
  level = row * 3;
  if (row == 16) row=0;
  if (level == 48) level=0;
  
  pinMode(BLANK_Pin, OUTPUT);
  timer1_write(500);
}

void fillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool ON)
{
    for (uint16_t x = x1; x <= x2; x++) {
        for (uint16_t y = y1; y <= y2; y++) {
            LED(x, y, ON);      
        }
    }
}

byte getPixelChar(uint8_t x, uint8_t y, char ch)

{
    if (x > 7) return 0;
    return bitRead(pgm_read_byte(&font8x16[ch-32][15-y]),7-x);
  
}
byte getPixelHString(uint16_t x, uint16_t y, char *p)

{
    p=p+x/8;
    return getPixelChar(x%8,y,*p); 
}

unsigned int lenString(char *p)
{
  unsigned int retVal=0;
  while(*p!='\0')
  { 
   retVal++;
   p++;
  }
  return retVal;
}

void printChar(uint8_t x, uint8_t y, bool For_color, bool Bk_color, char ch)
{
  uint8_t xx,yy;
  xx=0;
  yy=0;
    
  for (yy=0; yy < 8; yy++)
    {
    for (xx=0; xx < 8; xx++)
      {
      if (bitRead(pgm_read_byte(&font8x8[ch-32][7-yy]),7-xx))
      
        {
            LED(x+xx, y+yy, For_color);
        }
      else
        {
            LED(x+xx, y+yy, Bk_color);      
        }
      }
    }
}


void EraseShowNumber(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, char previous_number, char current_number, bool For_color, bool Bk_color, uint16_t animation)
{

 if (animation == 0)
  {            
    EraserHandwritten(xoffset, yoffset, 1, 15);
    HandwrittenNumber(xoffset, yoffset, current_number, 1, 10);
  }
  else if (animation == 1)
  {    
    ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, LEFT_LEFT);
    ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_RIGHT);
  }
  else if (animation == 2)
  {    
    ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, LEFT_LEFT);
    ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_LEFT);
  }          
  else if (animation == 3)
  {    
    ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, ANI_UP);
    ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_DOWN);
  }
  else if (animation == 4)
  {    
    ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, ANI_UP);
    HandwrittenNumber(xoffset, yoffset, current_number, 1, 15);
  }
  else if (animation == 5)
  {    
    ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, ANI_UP);
    ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_RIGHT);
  }
  else if (animation == 6)
  {    
    ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, ANI_UP);
    ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_LEFT);
  }  
  else if (animation == 7)
  {    
    ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, LEFT_LEFT);
    ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_DOWN);
  } 
  else if (animation == 8)
  {    
    ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, LEFT_LEFT);
    HandwrittenNumber(xoffset, yoffset, current_number, 1, 15);
  } 
  else if (animation == 9)
  {            
    EraserHandwritten(xoffset, yoffset, 1, 15);
    ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_RIGHT);
  }
  else if (animation == 10)
  {            
    EraserHandwritten(xoffset, yoffset, 1, 15);
    ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_LEFT);
  }
  else if (animation == 11)
  {            
    EraserHandwritten(xoffset, yoffset, 1, 15);
    ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_DOWN); 
  }

  else
  {     
    if (rand()%3 ==0)
      {            
        EraserHandwritten(xoffset, yoffset, 1, 15);
      }
    else if (rand()%3 ==1)
      {
        ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, ANI_UP);       
      }
    else
      {
        ScrollNumber(xoffset, yoffset, 8, 8, previous_number, 1, 0, LEFT_LEFT);  
      }
    if (rand()%4 ==0)
      { 
        HandwrittenNumber(xoffset, yoffset, current_number, 1, 15);
      }
    else if (rand()%4 ==1)
      {
        ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_LEFT);
      }
    else if (rand()%4 ==2)
      {
        ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_RIGHT);
      }
    else
      {
        ScrollNumber(xoffset, yoffset, 8, 8, current_number, 1, 0, ANI_DOWN);
      }
  }             
}

void ScrollNumber(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, char number, bool For_color, bool Bk_color, uint16_t scrolltype)
{
  if(scrolltype==0)
    {    
      switch (number)
      {
      case '0': HScrollImageR(xoffset, yoffset, width, height, zero,  For_color, Bk_color); break; //  Scroll-number character ' 0 ' //
      case '1': HScrollImageR(xoffset, yoffset, width, height, one,   For_color, Bk_color); break; //  Scroll-number character ' 1 ' //
      case '2': HScrollImageR(xoffset, yoffset, width, height, two,   For_color, Bk_color); break; //  Scroll-number character ' 2 ' //
      case '3': HScrollImageR(xoffset, yoffset, width, height, three, For_color, Bk_color); break; //  Scroll-number character ' 3 ' //
      case '4': HScrollImageR(xoffset, yoffset, width, height, four,  For_color, Bk_color); break; //  Scroll-number character ' 4 ' //
      case '5': HScrollImageR(xoffset, yoffset, width, height, five,  For_color, Bk_color); break; //  Scroll-number character ' 5 ' //
      case '6': HScrollImageR(xoffset, yoffset, width, height, six,   For_color, Bk_color); break; //  Scroll-number character ' 6 ' //
      case '7': HScrollImageR(xoffset, yoffset, width, height, seven, For_color, Bk_color); break; //  Scroll-number character ' 7 ' //
      case '8': HScrollImageR(xoffset, yoffset, width, height, eight, For_color, Bk_color); break; //  Scroll-number character ' 8 ' //
      case '9': HScrollImageR(xoffset, yoffset, width, height, nine,  For_color, Bk_color); break; //  Scroll-number character ' 9 ' //
      }
    }
    else if(scrolltype==1)
    {    
      switch (number)
      { 
      case '0': HScrollImageL(xoffset, yoffset, width, height, zero,  For_color, Bk_color); break; //  Scroll-number character ' 0 ' //
      case '1': HScrollImageL(xoffset, yoffset, width, height, one,   For_color, Bk_color); break; //  Scroll-number character ' 1 ' //
      case '2': HScrollImageL(xoffset, yoffset, width, height, two,   For_color, Bk_color); break; //  Scroll-number character ' 2 ' //
      case '3': HScrollImageL(xoffset, yoffset, width, height, three, For_color, Bk_color); break; //  Scroll-number character ' 3 ' //
      case '4': HScrollImageL(xoffset, yoffset, width, height, four,  For_color, Bk_color); break; //  Scroll-number character ' 4 ' //
      case '5': HScrollImageL(xoffset, yoffset, width, height, five,  For_color, Bk_color); break; //  Scroll-number character ' 5 ' //
      case '6': HScrollImageL(xoffset, yoffset, width, height, six,   For_color, Bk_color); break; //  Scroll-number character ' 6 ' //
      case '7': HScrollImageL(xoffset, yoffset, width, height, seven, For_color, Bk_color); break; //  Scroll-number character ' 7 ' //
      case '8': HScrollImageL(xoffset, yoffset, width, height, eight, For_color, Bk_color); break; //  Scroll-number character ' 8 ' //
      case '9': HScrollImageL(xoffset, yoffset, width, height, nine,  For_color, Bk_color); break; //  Scroll-number character ' 9 ' //
      }
    }

    else if(scrolltype==2)
    {    
      switch (number)
      {      
      case '0': VScrollImageD(xoffset, yoffset, width, height, zero,  For_color, Bk_color); break; //  Scroll-number character ' 0 ' //
      case '1': VScrollImageD(xoffset, yoffset, width, height, one,   For_color, Bk_color); break; //  Scroll-number character ' 1 ' //
      case '2': VScrollImageD(xoffset, yoffset, width, height, two,   For_color, Bk_color); break; //  Scroll-number character ' 2 ' //
      case '3': VScrollImageD(xoffset, yoffset, width, height, three, For_color, Bk_color); break; //  Scroll-number character ' 3 ' //
      case '4': VScrollImageD(xoffset, yoffset, width, height, four,  For_color, Bk_color); break; //  Scroll-number character ' 4 ' //
      case '5': VScrollImageD(xoffset, yoffset, width, height, five,  For_color, Bk_color); break; //  Scroll-number character ' 5 ' //
      case '6': VScrollImageD(xoffset, yoffset, width, height, six,   For_color, Bk_color); break; //  Scroll-number character ' 6 ' //
      case '7': VScrollImageD(xoffset, yoffset, width, height, seven, For_color, Bk_color); break; //  Scroll-number character ' 7 ' //
      case '8': VScrollImageD(xoffset, yoffset, width, height, eight, For_color, Bk_color); break; //  Scroll-number character ' 8 ' //
      case '9': VScrollImageD(xoffset, yoffset, width, height, nine,  For_color, Bk_color); break; //  Scroll-number character ' 9 ' //
      }
    }    
    else if(scrolltype==3)
    {
      switch (number)
      {      
      case '0': VScrollImageU(xoffset, yoffset, width, height, zero,  For_color, Bk_color); break; //  Scroll-number character ' 0 ' //
      case '1': VScrollImageU(xoffset, yoffset, width, height, one,   For_color, Bk_color); break; //  Scroll-number character ' 1 ' //
      case '2': VScrollImageU(xoffset, yoffset, width, height, two,   For_color, Bk_color); break; //  Scroll-number character ' 2 ' //
      case '3': VScrollImageU(xoffset, yoffset, width, height, three, For_color, Bk_color); break; //  Scroll-number character ' 3 ' //
      case '4': VScrollImageU(xoffset, yoffset, width, height, four,  For_color, Bk_color); break; //  Scroll-number character ' 4 ' //
      case '5': VScrollImageU(xoffset, yoffset, width, height, five,  For_color, Bk_color); break; //  Scroll-number character ' 5 ' //
      case '6': VScrollImageU(xoffset, yoffset, width, height, six,   For_color, Bk_color); break; //  Scroll-number character ' 6 ' //
      case '7': VScrollImageU(xoffset, yoffset, width, height, seven, For_color, Bk_color); break; //  Scroll-number character ' 7 ' //
      case '8': VScrollImageU(xoffset, yoffset, width, height, eight, For_color, Bk_color); break; //  Scroll-number character ' 8 ' //
      case '9': VScrollImageU(xoffset, yoffset, width, height, nine,  For_color, Bk_color); break; //  Scroll-number character ' 9 ' //      
    }
  }
    else
    {
      switch (number)
      {      
      case '0': HScrollImageLL(xoffset, yoffset, width, height, zero,  For_color, Bk_color); break; //  Scroll-number character ' 0 ' //
      case '1': HScrollImageLL(xoffset, yoffset, width, height, one,   For_color, Bk_color); break; //  Scroll-number character ' 1 ' //
      case '2': HScrollImageLL(xoffset, yoffset, width, height, two,   For_color, Bk_color); break; //  Scroll-number character ' 2 ' //
      case '3': HScrollImageLL(xoffset, yoffset, width, height, three, For_color, Bk_color); break; //  Scroll-number character ' 3 ' //
      case '4': HScrollImageLL(xoffset, yoffset, width, height, four,  For_color, Bk_color); break; //  Scroll-number character ' 4 ' //
      case '5': HScrollImageLL(xoffset, yoffset, width, height, five,  For_color, Bk_color); break; //  Scroll-number character ' 5 ' //
      case '6': HScrollImageLL(xoffset, yoffset, width, height, six,   For_color, Bk_color); break; //  Scroll-number character ' 6 ' //
      case '7': HScrollImageLL(xoffset, yoffset, width, height, seven, For_color, Bk_color); break; //  Scroll-number character ' 7 ' //
      case '8': HScrollImageLL(xoffset, yoffset, width, height, eight, For_color, Bk_color); break; //  Scroll-number character ' 8 ' //
      case '9': HScrollImageLL(xoffset, yoffset, width, height, nine,  For_color, Bk_color); break; //  Scroll-number character ' 9 ' //      
    }
  }
}

void drawImage(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color)
{
    for (uint16_t y = 0; y < height; y++)
    {
        for (uint16_t x = 0; x < width; x++)
        {
            bool colorImage = bitRead(pgm_read_byte(&image[7-y]),7-x) & 1;
              if (colorImage)
                {
                LED(x + xoffset, y + yoffset, For_color);
                }
              else
                {
                LED(x + xoffset, y + yoffset, Bk_color);        
                }
          }
    }
}

void HScrollImageR(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color)
{
  
    for (uint16_t i = width; i > 0; i--)
    {
    for (uint16_t y = 0; y < height; y++)
      {
        for (uint16_t x = 0; x < width - i; x++)
        {
            uint16_t   myindex = (i + x - 1)/8 + y * (width / 8);
            uint16_t   mybitmask = 7 - (i + x-1) % 8;
            bool colorImage = bitRead(pgm_read_byte(&image[7 - myindex]),mybitmask) & 1;
              if (colorImage)
                {
                LED(x + xoffset, y + yoffset - 1, For_color);
                }
              else
                {
                LED(x + xoffset, y + yoffset - 1, Bk_color);        
                }
          }
      }
  delay(30);
  }  
}


void HScrollImageL(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color)
{
    for (uint16_t x = width; x > 0; x--)
    {
      drawImage(x + xoffset - 1, yoffset - 1, width - x, height, image, For_color, Bk_color);
      delay(30);
    }
}

void HScrollImageLL(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color)
{
  for (uint16_t i = 0; i < width; i++)
    {
    for (uint16_t y = 0; y < height; y++)
      {
        for (uint16_t x = 0; x < width - i; x++)
        {
            uint16_t   myindex = (i + x )/8 + y * (width / 8);
            uint16_t   mybitmask = 7 - (i + x) % 8;
            bool colorImage = bitRead(pgm_read_byte(&image[7 - myindex]),mybitmask) & 1;
              if (colorImage)
                {
                LED(x + xoffset, y + yoffset - 1, For_color);
                }
              else
                {
                LED(x + xoffset, y + yoffset - 1, Bk_color);        
                }
          }
      }
  delay(30);
  } 
}


void VScrollImageD(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color)
{
    for (uint16_t y = height; y > 0 ; y--)
    { 
      drawImage(xoffset, yoffset + y - 2, width, height - y + 1, image, For_color, Bk_color);
      delay(30);
    }
}


void VScrollImageU(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image, bool For_color, bool Bk_color)
{
    for (uint16_t y = 0; y < height; y++)
    {
      drawImage(xoffset, yoffset + y, width, height - y - 1, image, For_color, Bk_color);
      delay(30);
    }
}

void hScroll(uint8_t y, bool For_color, bool Bk_color, char *mystring, uint8_t delaytime, uint8_t times, uint8_t dir)
{
  int offset;
  bool color;
    while (times)
    {
    for ((dir) ? offset=0 : offset=((lenString(mystring)-8)*8-1); (dir) ? offset <((lenString(mystring)-8)*8-1) : offset >0; (dir) ? offset++ : offset--)
      {
      for (byte xx=0; xx<24; xx++)
        {     
            for (byte yy=0; yy<16; yy++)
              {
                if (getPixelHString(xx+offset,yy,mystring)) 
                  {
                  color = For_color;
                  }
                else color=Bk_color;
                  LED(xx,(yy+y),color);
              }   
          }
          delay(delaytime);  
      }
    times--;
  } 
}

void HandwrittenNumber(byte xoffset, byte yoffset, char number, bool state, int delaytime)
{
switch (number)
  {
    case '0': DrawDigit(Zero,       32, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 0 ' //
    case '1': DrawDigit(One,        20, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 1 ' //
    case '2': DrawDigit(Two,        25, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 2 ' //
    case '3': DrawDigit(Three,      23, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 3 ' //
    case '4': DrawDigit(Four,       26, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 4 ' //
    case '5': DrawDigit(Five,       25, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 5 ' //
    case '6': DrawDigit(Six,        24, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 6 ' //
    case '7': DrawDigit(Seven,      19, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 7 ' //
    case '8': DrawDigit(Eight,      28, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 8 ' //
    case '9': DrawDigit(Nine,       24, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 9 ' //
    case 'H': DrawDigit(Capital_H,  30, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 9 ' //
    case 'M': DrawDigit(Capital_M,  34, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 9 ' //
    case 'S': DrawDigit(Capital_S,  25, xoffset, yoffset, state, delaytime); break; //  Hand-writting character ' 9 ' //
  }
}
void DrawDigit(byte num[][2], byte siz, byte xoffset, byte yoffset, bool state, int delaytime)
{

    for (int i = 0; i < siz; i++)
      {
        LED(xoffset + num[i][0], yoffset + (6- (num[i][1])), state);
        delay(delaytime);
      }     
}
void EraserHandwritten(uint8_t xoffset, uint8_t yoffset, bool state, int delaytime)
{  
  for (int i = 0; i < 33; i++)
    {
      byte x1 = Delete[i][0];
      byte y1 = Delete[i][1];
      byte x2 = x1 + 1;
      byte y2 = y1 + 1;
      y1 = checkConstrains(y1, 0, 6);
      y2 = checkConstrains(y2, 0, 6);
      x1 = checkConstrains(x1, 0, 6);
      x2 = checkConstrains(x2, 0, 6);
      xyfillRectangle(xoffset, yoffset, y1, x1, y2, x2, state);
      delay(delaytime);
      xyfillRectangle(xoffset, yoffset, y1, x1, y2, x2, 0);
    }
  
}

void xyfillRectangle(uint8_t xoffset,  uint8_t yoffset, uint8_t y1, uint8_t x1, uint8_t y2, uint8_t x2, bool state)
{
  for (uint8_t y = y1; y <= y2; y++) 
    {
      for (uint8_t x = x1; x <= x2; x++) 
        {
          LED(xoffset + x, yoffset + y, state); 
        }
    }         
}

int checkConstrains(int value, int min, int max) {
  if(value < min) {
    return min;
  } else if (value > max) {
    return max;
  } else {
    return value;
  }
}

void MutiEffect_S1()
{ 
  if ( (unsigned long) (micros() - samplingtimes1) > 155  )
  {                  
    s1 = HHMMSS[20];             
    if (s1 != prves1)  
      {                  
        EraseShowNumber(16, 9, 8, 8, prves1, s1, 1, 0, animation);
        prves1 = s1;         
      }
   
      samplingtimes1 = micros(); 
    }
}        

void MutiEffect_S0()
{ 
  if ( (unsigned long) (micros() - samplingtimes0) > 99  )
  {   
      s0 = HHMMSS[21];       
      if (s0 != prves0)  
      {  
        EraseShowNumber(16, 0, 8, 8, prves0, s0, 1, 0, animation);
        prves0 = s0;
      }          
      samplingtimes0 = micros(); 
    }
}


void MutiEffect_M1()
{ 
  if ( (unsigned long) (micros() - samplingtimem1) > 555  )
  {     
    m1 = HHMMSS[17];
    if (m1 != prvem1)  
      {      
        EraseShowNumber(8, 9, 8, 8, prvem1, m1, 1, 0, animation);              
        prvem1 = m1;
      }
      samplingtimem1 = micros(); 
    }
}   

void MutiEffect_M0()
{ 
  if ( (unsigned long) (micros() - samplingtimem0) > 255  )
  {
    m0 = HHMMSS[18];
    if (m0!=prvem0)  
      {         
        EraseShowNumber(8, 0, 8, 8, prvem0, m0, 1, 0, animation);
        prvem0 = m0;
      }
      samplingtimem0 = micros(); 
    }
} 

void MutiEffect_H1()
{ 
  if ( (unsigned long) (micros() - samplingtimeh1) > 555  )
  {

    h1 = HHMMSS[14];
    if (h1 != prveh1)  
      {          
        EraseShowNumber(0, 9, 8, 8, prveh1, h1, 1, 0, animation);
        prveh1 = h1;
      }
      samplingtimeh1 = micros(); 
    }
}       

void MutiEffect_H0()
{ 
  if ( (unsigned long) (micros() - samplingtimeh0) > 455  )
  {    
    h0 = HHMMSS[15];    
    if (h0 != prveh0)  
      {   
        EraseShowNumber(0, 0, 8, 8, prveh0, h0, 1, 0, animation);
        prveh0 = h0;
      }
      samplingtimeh0 = micros(); 
    }
} 

void FillDot()
{ 
  if ( (unsigned long) (millis() - samplingtime2) > 200  )
  {      
      xyfillRectangle(0, 0, 7, 6, 8, 7, dotstate);
      xyfillRectangle(0, 0, 7, 14, 8, 15, dotstate);
      dotstate = !dotstate;
      samplingtime2 = millis(); 
    }
} 

void SCROLLYYYYMMDDHHMMSS()
{ 
  if ( (unsigned long) (millis() - samplingtime1) > 90000  )
  {  
      EraserHandwritten(0,  0, 1, 10);
      EraserHandwritten(0,  9, 1, 10);
      EraserHandwritten(8,  0, 1, 10);  
      EraserHandwritten(8,  9, 1, 10);
      EraserHandwritten(16, 0, 1, 10);
      EraserHandwritten(16, 9, 1, 10);
      
      xyfillRectangle(0, 0, 7, 6, 8, 7, 0);
      xyfillRectangle(0, 0, 7, 14, 8, 15, 0);
      
      hScroll(0, 1, 0, YYYYMMDD,  20, 1, 1);
      hScroll(0, 1, 0, HHMMSS,    20, 1, 1);

      clearscreen();
      
      xyfillRectangle(0, 0, 7, 6, 8, 7, 1);
      xyfillRectangle(0, 0, 7, 14, 8, 15, 1);
      
      HandwrittenNumber(0,  0, HHMMSS[15], 1, 10);
      HandwrittenNumber(0,  9, HHMMSS[14], 1, 10);
      HandwrittenNumber(8,  0, HHMMSS[18], 1, 10);  
      HandwrittenNumber(8,  9, HHMMSS[17], 1, 10); 
      HandwrittenNumber(16, 0, HHMMSS[21], 1, 10);
      HandwrittenNumber(16, 9, HHMMSS[20], 1, 10);
      samplingtime1 = millis(); 
    }
}

void ReadButton()
{
  if ( (unsigned long) (millis() - samplingtime3) > 55  )
  {   
  buttonState = digitalRead(BUTTON);
 
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      buttonPushCounter++;
    }
    else {

    }
  }
  lastButtonState = buttonState;

  switch (buttonPushCounter % 13) 
  {
    case 0:
    animation=0;
    break;
    case 1:
    animation=1;    
    break;
    case 2:
    animation=2;   
    break;
    case 3:
    animation=3;
    break;
    case 4:
    animation=4;
    break;
    case 5:
    animation=5;    
    break;
    case 6:
    animation=6;   
    break;
    case 7:
    animation=7;
    break; 
    case 8:
    animation=8;
    break;
    case 9:
    animation=9;    
    break;
    case 10:
    animation=10;   
    break;
    case 11:
    animation=11;
    break; 
    case 12:
    animation=12;
    break;         
  }
  samplingtime3 = millis();
  }
}        
