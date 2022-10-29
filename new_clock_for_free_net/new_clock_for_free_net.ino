#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include<WiFi.h>
#include "time.h"

const char* essid = "xxxxxxx";   #here set the ssid and password
const char* password = "xxxxxx";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

uint32_t targetTime = 0;       // for next 1 second timeout

byte omm = 99;
boolean initial = 1;
byte xcolon = 0;
unsigned int colour = 0;

static uint8_t conv2d(const char* p) {      //a function to convert a string into int
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

//uint8_t hh=conv2d(__TIME__), mm=conv2d(__TIME__+3), ss=conv2d(__TIME__+6);  // Get H, M, S from compile time
uint8_t hh,mm,ss;
char date[20];

void printLocalTime()   //output a time message in the serial monitor
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //Serial.println(&timeinfo, "%H");
  //Serial.println(&timeinfo, "%M");
  //Serial.println(&timeinfo, "%S");
  
}

void setup(void) {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  tft.setCursor(0,0,4);
  tft.setTextColor(TFT_GREEN, TFT_BLACK); // Note: the new fonts do not draw the background colour

  bool wifiFound = false;  

  Serial.begin(115200);

  //Serial.println(hh);
  //Serial.println(mm);
  //Serial.println(ss);
  
  while (!wifiFound) {
    Serial.println("scan start");
    int n = WiFi.scanNetworks(); // WiFi.scanNetworks returns the number of networks found
    Serial.println("scan done");
    
    if (n == 0) 
    {
        Serial.println("no networks found");
    } 
    else 
    {
        Serial.print(n);
        Serial.println(" networks found");
        
        for (int i = 0; i < n; ++i) 
        {
            String ssid = WiFi.SSID(i);
            int    rssi = WiFi.RSSI(i);
          
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(ssid);
            Serial.print(" (");
            Serial.print(rssi);
            Serial.print(")");
            Serial.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
            
            ssid.trim();
            if (ssid == essid) 
            {
              Serial.print(" <==== WiFi found");
              wifiFound = true;
            }
            Serial.println("");
        }
        
    }
    Serial.println("");

     // Wait a bit before scanning again
    if (!wifiFound)
      delay(5000);
  }
  
  WiFi.begin(essid,password);
  Serial.print("Connecting to ");
  Serial.print(essid);

  while (WiFi.status() != WL_CONNECTED)   //every 500 milisecond, output a dot to prove the program not stuck
  {
    delay(500);
    Serial.print('.');
  }

  Serial.println('\n');
  Serial.println("Connection estabilished!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  tft.println("Connection\n");
  tft.println("Estabilished!");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain local time");
  }
  else
  {
    char h[3],m[3],s[3];
    strftime(h,3,"%H",&timeinfo);
    strftime(m,3,"%M",&timeinfo);
    strftime(s,3,"%S",&timeinfo);
    strftime(date,20,"%d %B %Y",&timeinfo);
    ss = conv2d(s);
    hh = conv2d(h);
    mm = conv2d(m);

  }
  
  delay(10000);

  WiFi.disconnect(true);   //disconnect wifi
  WiFi.mode(WIFI_OFF);

  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  targetTime = millis() + 1000; 

  
}

void loop() {
  
  
  if (targetTime < millis()) {
    targetTime = millis()+1000;
    ss++;              // Advance second
    if (ss==60) {
      ss=0;
      omm = mm;
      mm++;            // Advance minute
      if(mm>59) {
        mm=0;
        hh++;          // Advance hour
        if (hh>23) {
          hh=0;
        }
      }
    }

    if (ss==0 || initial) {
      initial = 0;
      tft.setTextColor(TFT_GREEN, TFT_BLACK);  
      tft.drawCentreString(date,120,60,4);

      
    }

    // Update digital time
    byte xpos = 6;
    byte ypos = 0;
    if (omm != mm) { // Only redraw every minute to minimise flicker
      // Uncomment ONE of the next 2 lines, using the ghost image demonstrates text overlay as time is drawn over it
      tft.setTextColor(0x39C4, TFT_BLACK);  // Leave a 7 segment ghost image, comment out next line!
      //tft.setTextColor(TFT_BLACK, TFT_BLACK); // Set font colour to black to wipe image
      // Font 7 is to show a pseudo 7 segment display.
      // Font 7 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .
      tft.drawString("88:88",xpos,ypos,7); // Overwrite the text to clear it
      tft.setTextColor(0xFBE0, TFT_BLACK); // Orange
      omm = mm;

      if (hh<10) xpos+= tft.drawChar('0',xpos,ypos,7);
      xpos+= tft.drawNumber(hh,xpos,ypos,7);
      xcolon=xpos;
      xpos+= tft.drawChar(':',xpos,ypos,7);
      if (mm<10) xpos+= tft.drawChar('0',xpos,ypos,7);
      tft.drawNumber(mm,xpos,ypos,7);
    }

    if (ss%2) { // Flash the colon
      tft.setTextColor(0x39C4, TFT_BLACK);
      xpos+= tft.drawChar(':',xcolon,ypos,7);
      tft.setTextColor(0xFBE0, TFT_BLACK);
    }
    
    else {
      tft.drawChar(':',xcolon,ypos,7);
      
    }
    
  }  

  
}



