// ------------------------------------
// Erdfeuchtemessung mit ESP8266-01
// ---
// Measure soil moisture with ESP8266-01
// (the source code variables and so on is original writen in german, 
// a translation will be come later if necessary)
// 
// @Autors: 
//       - Ralf Wießner (aka Zentris)
//       - code for ntp adopted from Michael Margolis
//       - code for time_ntp adopted from by Stefan Thesen
// 
// @Known_bugs:
//       -- none --
//
// @Open_issues:
//       - switching time stamp to MESZ and back
//       - use MAC for ESP identifiyer
//       - watchdog signal (get a blinking LED)
//       - translate variables to english
//       - translate comments to english
//
// @Feature_and_History:
//   1.0 : 
//       - setting the measurement intervals via HTTP request answer
//       - 2 measurment canals (GPIO0 and GPIO2)
//       - data transfer to local data server 
//       - data transfer to remote Thingsspeak server (optional)
//       - 10 probs on each measurement loop
//       - create median and create the mean value over 5 media values
//       - use a local timeserver (here a fritzbox)
//         (adoption of time_ntp)
// ------------------------------------

#define versionnumber "1.0"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include "time_ntp.h"

ESP8266WiFiMulti WiFiMulti;

// ESP identifier 
// (obsolete, will be removed in next version and replaced with MAC)
#define ESPNO 1

// define the GPIO
#define INTPIN 0
#define INTPIN2 2

// all times in milliSec
#define MESSABSTAND_DEFAULT 30000
#define messzeitms           1000
#define messintervalle         10
#define ntpRefreshAfter   3600000

// HTTP request codes
#define HTTP_CODE_OK 200

unsigned long messabstand     = MESSABSTAND_DEFAULT; // default = 30sec
unsigned long ntpdRefreshTime = 0;    				 // Zeitpunkt, wann die Zeit erneut geholt wird
 
const char* ccSSID          = <--yourSSID-->;
const char* ccPASSWORD      = <--yourPassword-->;

const char* ccDATASERVER            = <--yourDATASERVER-IP-->;   
const char* ccDATASERVERUPLOADTOOL  = "/homecontrol/upd_feuchte.php";  // path fix if you use this project
const int   iDATASERVERPORT = 80;

IPAddress ipAddrNTPServer(192, 168, 178, 1);  // address of my time server
//IPAddress ipAddrNTPServer(129, 6, 15, 28);  // time.nist.gov NTP server

uint8_t MACArray[6];      // return type of mac request
char    MACAddrChar[18];  // readable mac address

// ntp timestamp
unsigned long ulSecs2000_timer = 0;   // normalised timestamp in sec from 1.1.2000 00:00:00
 
date_time_t *date = new date_time_t;  // date structure for output calculation

volatile unsigned long counter = 0;   // interrupt loop counter
unsigned long bodenfeuchteGemittelt;  // 

/**
 * Füllt den übergeben Zahlenwert mit einer führenden '0', wenn
 * einstellig, addiert ggf. den übergebenen 'addValue Wert dazu.
 * @todo: translate
 * @return: String
 */
String fillZero(unsigned char& value, int addValue=0) {
  int i = int(value) + addValue;
  if (i<10) { return String("0") + String(i);}
  else      { return String(i); }
}

/**
 * Erzeugt einen String mit dem aktuellen Datum aus der globalen Variable 'date'.
 * @todo: translate
 * @return: String Format: "YYYY-MM-DD"
 */
String getDatum() {
  String s = fillZero(date->year, 2000) + "-" + fillZero(date->month) + "-" + fillZero(date->day);
  return(s);
}

/**
 * Erzeugt einen String mit der aktuellen Zeit aus der globalen Variable 'date'.
 * @todo: translate
 * @return: String Format: "hh:mm:ss"
 */
String getTime() {
  String s = fillZero(date->hour) + ":" + fillZero(date->minute) + ":" + fillZero(date->second);
  return(s);
}

/**
 * Get the current time (UTC) and date from the time server and fill the data structure.
 * The time will be adapted to MEZ, no MESZ!
 */
void getDateTimeNTP() {
  ulSecs2000_timer=getNTPTimestamp(ipAddrNTPServer)+3600;    // add +3600 sec (1h) for MET
  epoch_to_date_time(date, ulSecs2000_timer);                // fresh up the date structure
  ulSecs2000_timer -= millis()/1000;                         // keep distance to millis counter at now
  Serial.print("Current Time UTC from NTP server: " );
  Serial.println(epoch_to_string(ulSecs2000_timer).c_str());
  ntpdRefreshTime = ulSecs2000_timer + ntpRefreshAfter;      // setze erneutes Zeitholen
}

/**
 * Main Wifi start 
 */
void WiFiStart() {
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected to ");
  Serial.println(WiFi.localIP());
  
  // connect to NTP and get time, fill internal struct
  if (ulSecs2000_timer + millis()/1000 >= ntpdRefreshTime) {
    Serial.print("... get the current time from NTP Server ");
    Serial.println(ipAddrNTPServer);
    getDateTimeNTP();
  }
}

/**
 * Setup sequence, will be run only once
 */
void setup() {
  Serial.begin(57600);
  
  WiFiMulti.addAP(ccSSID, ccPASSWORD);
  WiFi.macAddress(MACArray);

  // get the mac address for identifying of the node.
  for (int i = 0; i < sizeof(MACArray); ++i) {
    sprintf(MACAddrChar,"%s%02x:",MACAddrChar,MACArray[i]);
  }
  MACAddrChar[sizeof(MACAddrChar)-1]= 0x00; // remove the last ':'
  Serial.println(MACAddrChar);
}

/**
 * Einfache Median/Mittelwertberechnung
 * ------------------------------------
 * Übergebenes Array sortieren und über die im Array "in der Mitte" liegenden
 * Werte den Durchschnitt ermitteln.
 * @todo: translate
 */
unsigned long median(unsigned long *values, int arraySize) {
  unsigned long tmp;
  const int relVal = 2;  // +- 2 Werte + 1 für die Mittelwertberechnung
  
  for (int i=0; i<arraySize; i++) {
    for (int j=arraySize-1; j>i; j--) {
      if( values[j] < values[j - 1] ) {
        tmp = values[j];
        values[j] = values[j - 1];
        values[j - 1] = tmp;
      }
    }
  } 
  for (int i=arraySize/2-relVal; i<arraySize/2+relVal; tmp +=values[i++]) {}
  return tmp/(relVal*2+1);
}

/**
 * Interrupt function: increment the counter
 */
void intfunc() {
  counter++;
}

/**
 * Feuchtemessung per Frequenzmessung + Messwertglättung
 */
void bodenfeuchtemessung(int gpioPin) {
  unsigned long speicherArray[messintervalle];
  
  for (int i=0; i<messintervalle; i++) {
    Serial.print("Messung Nr.: "); Serial.print(i); 
    Serial.print(" an GPIO-Pin "); Serial.print(gpioPin);
    Serial.print(" = ");
    counter=0;
    //  attachInterrupt(gpioPin, intfunc, CHANGE);
    attachInterrupt(gpioPin, intfunc, RISING);
    delay(messzeitms);
    detachInterrupt(gpioPin);
    speicherArray[i] = counter;
    Serial.println(counter);
  }
  bodenfeuchteGemittelt = median(speicherArray, messintervalle); 
  Serial.print("Gemittelt: "); Serial.println(bodenfeuchteGemittelt);
//  writeWebLog("[bodenfeuchtemessung:010] Median=" + String(bodenfeuchteGemittelt));
}

/**
 * @@@messabstand=20000;
 */
void getMessabstandFromHttpRequest(String s, const char * tag) {
  const int maxValueLength = 15;
  char value[maxValueLength];
    
  messabstand = MESSABSTAND_DEFAULT;  // default setzen, falls was schiefgeht
  
  char * pos = strstr(s.c_str(), "@@@");
  if (pos > 0) {
    char * sp = strstr(pos, tag);
    if (sp > 0) {
      char * va = strstr(sp, "=");
      if (va > 0) {
        char * ve = strstr(va, ";");
        if (ve+1-va < maxValueLength) {
          memset(value, 0, maxValueLength); 
          strncpy(value, va+1, ve-va-1);
          messabstand = atol(value);  // alles gut, jetzt zuweisen
        }
      }
    }
  }
}


/**
 * Datentransfer zum lokalen Datenserver per HTTP-Get Request.
 */ 
void saveData2LocalNet(int gpio=0) {
  HTTPClient http;
 
  epoch_to_date_time(date, ulSecs2000_timer + millis()/1000); // fresh up the date structure for current time
  String request= String(ccDATASERVERUPLOADTOOL) + "?espno=" + ESPNO + "&date=" + getDatum() + "&time=" + getTime() 
          + "&feuchte=" + String(bodenfeuchteGemittelt) + "&mac=" + String(MACAddrChar) + "&gpio=" + String(gpio);
          
  Serial.print("Try to get REST request: ");  Serial.println(request);
  http.begin(ccDATASERVER, iDATASERVERPORT, request);
  int httpCode = http.GET();
  if (httpCode) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      Serial.println("[HTTP] GET... successfull");
      // file found at server
      String payload = http.getString();
      getMessabstandFromHttpRequest(payload, "messabstand");
    }
    else Serial.println("[HTTP] GET... errornous!");
  }
}

/**
 * ThingsSpeak: https://api.thingspeak.com/update?api_key=<--yourKey-->&field1=0
 */
void saveData2ThingsSpeak() {
  String tsServer = "api.thingspeak.com";
  String tsServerSIP = "184.106.153.149";  // statische Addresse!
  String tsAPIKey = "<--yourAPIKey-->";
  
  Serial.println("Try to send to ThingsSpeak...");  
  
  Serial.println("Try to get REST request: ");  
  String request= "/update?api_key=" + tsAPIKey + "&field1=" + String(bodenfeuchteGemittelt);
  Serial.println(request);

  HTTPClient http;
  http.begin(tsServerSIP.c_str(), 80, request);
  int httpCode = http.GET();
  if (httpCode) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // don't found at server
    if (httpCode != 200) {
      Serial.print("[HTTP] GET... failed, no connection or no HTTP server\n");
      Serial.println(http.getString()); 
    }
    else Serial.println("[HTTP] GET... successfull");
  } 
}

/**
 * Main loop
 */
void loop() {
  unsigned long looptimeStart = millis();
  
  Serial.println("\n--START-MESSUNG--");
  WiFiStart();

  // Sensor GPIO 0
  bodenfeuchtemessung(INTPIN);
  saveData2LocalNet(INTPIN);
  // saveData2ThingsSpeak();    // currently not active, optional

  // Sensor GPIO 2
  bodenfeuchtemessung(INTPIN2);
  saveData2LocalNet(INTPIN2);
  // saveData2ThingsSpeak();    // currently not active, optional

  Serial.print("messabstand       = "); Serial.println(messabstand);

  unsigned long looptimeEnde = millis();           // whole loope time without delay 
  unsigned long looptime = looptimeEnde - looptimeStart;
 
  Serial.print("looptime          = "); Serial.println(looptimeEnde - looptimeStart);
  Serial.print("messabstand (neu) = "); Serial.println(messabstand - looptime);
  
  epoch_to_date_time(date, ulSecs2000_timer + (millis() + messabstand - looptime)/1000); // fresh up the date structure for current time
   
  String s = "Nächste Messung: " + getDatum() + " :: " + getTime() + "\n";
  Serial.printf(String("Nächste Messung: " + getDatum() + " :: " + getTime() + "\n").c_str());
  
  delay(messabstand - looptime);
}
