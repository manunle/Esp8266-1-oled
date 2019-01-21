 // Include the correct display library
 // For a connection via I2C using Wire include
 #include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
 // or #include "SH1106Wire.h", legacy include: `#include "SH1106.h"`
 // For a connection via I2C using brzo_i2c (must be installed) include
 // #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
 // #include "SSD1306Brzo.h"
 // #include "SH1106Brzo.h"
 // For a connection via SPI include
 // #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
 // #include "SSD1306Spi.h"
 // #include "SH1106SPi.h"

 // Use the corresponding display class:

 // Initialize the OLED display using SPI
 // D5 -> CLK
 // D7 -> MOSI (DOUT)
 // D0 -> RES
 // D2 -> DC
 // D8 -> CS
 // SSD1306Spi        display(D0, D2, D8);
 // or
 // SH1106Spi         display(D0, D2);

 // Initialize the OLED display using brzo_i2c
 // D3 -> SDA
 // D5 -> SCL
 // SSD1306Brzo display(0x3c, D3, D5);
 // or
 // SH1106Brzo  display(0x3c, D3, D5);

 // Initialize the OLED display using Wire library
#include <Arduino.h>
#include <PubSubClient.h>
#include <math.h>

#include "ESPBASE.h"

int D3 = 0;
int D5 = 2;
unsigned int idx = 0;
unsigned int milldelay = 3000;
unsigned long lastdisplay = 0;
String messages[] = {
      "This",
      "Is" ,
      "A",
      "Test",
      "",
      "",
      "",
      "",
      "",
      "",
      ""
};

SSD1306Wire  display(0x3c, D3, D5);
 // SH1106 display(0x3c, D3, D5);
long lastReconnectAttempt = 0;
String StatusTopic;
String sChipID;

ESPBASE Esp;
// Adapted from Adafruit_SSD1306
void drawLines() {
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(0, 0, i, display.getHeight()-1);
    display.display();
    delay(10);
  }
  for (int16_t i=0; i<display.getHeight(); i+=4) {
    display.drawLine(0, 0, display.getWidth()-1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(0, display.getHeight()-1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
    display.drawLine(0, display.getHeight()-1, display.getWidth()-1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i=display.getWidth()-1; i>=0; i-=4) {
    display.drawLine(display.getWidth()-1, display.getHeight()-1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
    display.drawLine(display.getWidth()-1, display.getHeight()-1, 0, i);
    display.display();
    delay(10);
  }
  delay(250);
  display.clear();
  for (int16_t i=0; i<display.getHeight(); i+=4) {
    display.drawLine(display.getWidth()-1, 0, 0, i);
    display.display();
    delay(10);
  }
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(display.getWidth()-1, 0, i, display.getHeight()-1);
    display.display();
    delay(10);
  }
  delay(250);
}

// Adapted from Adafruit_SSD1306
void drawRect(void) {
  for (int16_t i=0; i<display.getHeight()/2; i+=2) {
    display.drawRect(i, i, display.getWidth()-2*i, display.getHeight()-2*i);
    display.display();
    delay(10);
  }
}

// Adapted from Adafruit_SSD1306
void fillRect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.getHeight()/2; i+=3) {
    display.setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
    display.fillRect(i, i, display.getWidth() - i*2, display.getHeight() - i*2);
    display.display();
    delay(10);
    color++;
  }
  // Reset back to WHITE
  display.setColor(WHITE);
}

// Adapted from Adafruit_SSD1306
void drawCircle(void) {
  for (int16_t i=0; i<display.getHeight(); i+=2) {
    display.drawCircle(display.getWidth()/2, display.getHeight()/2, i);
    display.display();
    delay(10);
  }
  delay(1000);
  display.clear();

  // This will draw the part of the circel in quadrant 1
  // Quadrants are numberd like this:
  //   0010 | 0001
  //  ------|-----
  //   0100 | 1000
  //
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000001);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000011);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000111);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00001111);
  display.display();
}

/*void printBuffer(void) {
  // Initialize the log buffer
  // allocate memory to store 8 lines of text and 30 chars per line.
  display.setLogBuffer(3, 30);

  // Some test data
  const char* messages[] = {
      "Hello",
      "World" ,
      "----",
      "Show off",
      "how",
      "the log buffer",
      "is",
      "working.",
      "Even",
      "scrolling is",
      "working"
  };

  for (uint8_t i = 0; i < 11; i++) {
    display.clear();
    // Print to the screen
    display.println(messages[i]);
    // Draw it to the internal screen buffer
    display.drawLogBuffer(0, 0);
    // Display it on the screen
    display.display();
    delay(500);
  }
}*/

void setup() {
  display.init();

  // display.flipScreenVertically();

  display.setContrast(255);

  //drawLines();
  //delay(1000);
  //display.clear();

  //drawRect();
  //delay(1000);
  //display.clear();

  //fillRect();
  //delay(1000);
  //display.clear();

  //drawCircle();
  //delay(1000);
  //display.clear();
  display.setFont(ArialMT_Plain_16);
//  printBuffer();
//  delay(1000);
  display.clear();
  Serial.begin(115200);
  char cChipID[10];
  sprintf(cChipID,"%08X",ESP.getChipId());
  sChipID = String(cChipID);

  Esp.initialize();
  StatusTopic = String(DEVICE_TYPE) + "/" + config.DeviceName + "/status";
  customWatchdog = millis();
    display.sendCommand(DISPLAYALLON);
    delay(1000);
  Serial.println("Done with setup");  
}

void loop() {
    if(messages[idx] == "")
    {
        idx++;
        if(idx >= 11)
        {
            idx = 0;
        }
    }
    if(millis() > lastdisplay + milldelay)
    {
        Serial.println(messages[idx]);
        display.clear();
//        display.println(messages[idx]);
//        display.drawLogBuffer(0, 0);
        display.drawString(0,0,messages[idx]);
        display.display();
        idx++;
        if(idx >= 11)
        {
            idx = 0;
        }
        lastdisplay = millis();
    }
    Esp.loop();
}


String getSignalString()
{
  String signalstring = "";
  byte available_networks = WiFi.scanNetworks();
  signalstring = signalstring + sChipID + ":";
 
  for (int network = 0; network < available_networks; network++) {
    String sSSID = WiFi.SSID(network);
    if(network > 0)
      signalstring = signalstring + ",";
    signalstring = signalstring + WiFi.SSID(network) + "=" + String(WiFi.RSSI(network));
  }
  return signalstring;    
}

void sendStatus()
{
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String roundString(String value)
{
    uint8_t digits = 2;
    size_t n = 0;
    double number = value.toFloat();
    String ret = "";

    if(isnan(number))
        return ret = ret + "nan";
    if(isinf(number))
        return ret = ret + "inf";
    if(number > 4294967040.0)
        return ret = ret + "ovf";  // constant determined empirically
    if(number < -4294967040.0)
        return ret = ret + "ovf";  // constant determined empirically

    // Handle negative numbers
    if(number < 0.0) {
        ret = ret + '-';
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for(uint8_t i = 0; i < digits; ++i)
        rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it
    unsigned long int_part = (unsigned long) number;
    double remainder = number - (double) int_part;
    ret = ret + String(int_part);

    // Print the decimal point, but only if there are digits beyond
    if(digits > 0) {
        ret = ret + ".";
    }

    // Extract digits from the remainder one at a time
    while(digits-- > 0) {
        remainder *= 10.0;
        int toPrint = int(remainder);
        ret = ret + String(toPrint);
        remainder -= toPrint;
    }

    return ret;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char c_payload[length];
  memcpy(c_payload, payload, length);
  c_payload[length] = '\0';
  
  String s_topic = String(topic);
  String s_payload = String(c_payload);
  Serial.print(s_topic + ":" + s_payload);
  if(s_topic == "pizero/pizerotemp/value")
  {
      String label = getValue(s_payload,':',0);
      String value = getValue(s_payload,':',1);
      if(label == "Temperature")
      {
          value = roundString(value);
          messages[0] = "O Temp " + value;
      }
      if(label == "Humidity")
      {
          value = roundString(value);
          messages[3] = "Humidity " + value;
      }
  }
  if (s_topic == "EDWoodstove/woodstove/value") 
  {
      String label = getValue(s_payload,':',0);
      String value = getValue(s_payload,':',1);
      if(label == "StoveTemp")
      {
          value = roundString(value);
          messages[4] = "Stove " + value;
      }
  }
  if (s_topic == "Temperature/test1temp/value") 
  {
      String label = getValue(s_payload,':',0);
      String value = getValue(s_payload,':',1);
      if(label == "Temperature")
      {
          value = roundString(value);
          messages[2] = "I Temp " + value;
      }
  }
}

void mqttSubscribe()
{
    if (Esp.mqttClient->connected()) 
    {
        if (Esp.mqttClient->subscribe("pizero/pizerotemp/value")) 
        {
            Serial.println("Subscribed to pizero/pizerotemp/value");
            Esp.mqttSend(StatusTopic,"","Subscribed to pizero/pizerotemp/value");
            Esp.mqttSend(StatusTopic,verstr,","+Esp.MyIP()+" start");
        }
        if (Esp.mqttClient->subscribe("EDWoodstove/woodstove/value")) 
        {
            Serial.println("Subscribed to EDWoodstove/woodstove/value");
            Esp.mqttSend(StatusTopic,"","Subscribed to EDWoodstove/woodstove/value");
        }

        if (Esp.mqttClient->subscribe("Temperature/test1temp/value")) 
        {
            Serial.println("Subscribed to Temperature/test1temp/value");
            Esp.mqttSend(StatusTopic,"","Subscribed to Temperature/test1temp/value");
        }
    }
}


