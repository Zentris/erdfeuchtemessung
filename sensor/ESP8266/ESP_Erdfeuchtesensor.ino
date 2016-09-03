// ------------------------------------
// Erdfeuchtemessung mit ESP8266-01 / ESP8266-12(e)
// ---
/* History:
  1.0: - [finish] : changing of measurement distance per http request payload
  1.1: - [finish] : move private data to private.h file 
       - [todo]   : sleep mode for ESP8266-12(e) device (must be switched on before compile)
*/ 
// ------------------------------------

#define versionNumber "1.1"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include "time_ntp.h"
#include "privats.h"

// single ESP Number (deprecated)
#define ESPNO 1

// Switch on/off output of TTY (off = 0)
#define TTY_USE        1

//#define TTY_SPEED 115200
#define TTY_SPEED 57600

// use input of following GPIOs
//#define INTPIN_0 0
#define INTPIN_2 2


// Time in milliSec
#define MEASURING_DISTANCE_DEFAULT   10000  
#define MEASURING_TIME                1000
#define MEASURING_INTERVALLS            10
#define NTP_REFRESH_AFTER          3600000


// default HTTP requestcodes for "ok"
#define HTTP_CODE_OK 200


unsigned long measuringDistance = MEASURING_DISTANCE_DEFAULT;           // default = 30sec
unsigned long ntpdRefreshTime = 0;    // time if the get the ntp time re-read 
 
uint8_t MACArray[6];                  // return type of mac request
char    MACAddrChar[18];              // readable mac address

volatile unsigned long counter = 0;   // interrupt loop counter
unsigned long soilMoistAveraged;  // 

unsigned long ulSecs2000_timer = 0;   // normalised timestamp in sec from 1.1.2000 00:00:00
date_time_t *date = new date_time_t;  // date structure for output calculation


// define/init classes
ESP8266WiFiMulti WiFiMulti;
IPAddress ipAddrNTPServer(ntpIP.b1, ntpIP.b2, ntpIP.b3, ntpIP.b4);    // local fritz box


/**
 * Füllt den übergeben Zahlenwert mit einer führenden '0', wenn
 * einstellig, addiert ggf. den übergebenen 'addValue Wert dazu.
 * @return: String
 * 
 * @todo: übersetzen
 */
String fillZero(unsigned char& value, int addValue=0) {
  int i = int(value) + addValue;
  if (i<10) { return String("0") + String(i);}
  else      { return String(i); }
}

/**
 * Erzeugt einen String mit dem aktuellen Datum aus der globalen Variable 'date'.
 * @return: String Format: "YYYY-MM-DD"
 * 
 * @todo: übersetzen
 */
String getDatum() {
  String s = fillZero(date->year, 2000) + "-" + fillZero(date->month) + "-" + fillZero(date->day);
  return(s);
}

/**
 * Erzeugt einen String mit der aktuellen Zeit aus der globalen Variable 'date'.
 * @return: String Format: "hh:mm:ss"
 * 
 * @todo: übersetzen
 */
String getTime() {
  String s = fillZero(date->hour) + ":" + fillZero(date->minute) + ":" + fillZero(date->second);
  return(s);
}

/*
 * Logging function over web interface
 */
#ifdef LOGSERVER
void writeWebLog(String s) {
  HTTPClient http;
  epoch_to_date_time(date, ulSecs2000_timer + millis()/1000); // fresh up the date structure for current time
  String request= String(ccLOGSERVERUPLOADTOOL) + "?espno=" + ESPNO + "&date=" + getDatum() + "&time=" + getTime() + "&logtext=" + s;
  Serial.println(request);
  http.begin(ccLOGSERVER, iLOGSERVERPORT, request);
  http.GET(); // fire and forget...  
}
#endif


/**
 * Holt die aktuelle Zeit (+Datum) per NTP vom Server
 * und füllt die 'date' Struktur
 * 
 * @todo: übersetzen
 */
void getDateTimeNTP() {
  ulSecs2000_timer=getNTPTimestamp(ipAddrNTPServer)+3600;    // add +3600 sec (1h) for MET
  epoch_to_date_time(date, ulSecs2000_timer); // fresh up the date structure
  ulSecs2000_timer -= millis()/1000;          // keep distance to millis counter at now
  if (TTY_USE) {
    Serial.print("Current Time UTC from NTP server: " );
    Serial.println(epoch_to_string(ulSecs2000_timer).c_str());
  }
  ntpdRefreshTime = ulSecs2000_timer + NTP_REFRESH_AFTER; // setze erneutes Zeitholen
}

/**
 * 
 */
void WiFiStart() {
  
//  while (WiFi.status() != WL_CONNECTED) 
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected to ");
  Serial.println(WiFi.localIP());

//  writeWebLog("[WiFiStart:010] Wifi connected to " + WiFi.localIP());
//  writeWebLog("[WiFiStart:010] Wifi connected");
  
  ///////////////////////////////
  // connect to NTP and get time, fill dateTime struct
  ///////////////////////////////
  if (ulSecs2000_timer + millis()/1000 >= ntpdRefreshTime) {
    Serial.print("... get the current time from NTP Server ");
    Serial.println(ipAddrNTPServer);
    getDateTimeNTP();
//    writeWebLog("[WiFiStart:020] Zeit vom NTP Server " + String(ipAddrNTPServer) + " geholt");
  }
}

void setup() {
  Serial.begin(TTY_SPEED);
  while (!Serial) {};  // wait for serial port to connect. Needed for native USB
  
//  WiFi.begin(ssid, password);
  WiFiMulti.addAP(ccSSID, ccPASSWORD);
  WiFi.macAddress(MACArray);

  // get the mac address for identifying of the node.
  memset((void*)MACAddrChar, 0, sizeof(MACAddrChar));
  for (unsigned int i=0; i < sizeof(MACArray); ++i) {
    sprintf(MACAddrChar,"%s%02x:", MACAddrChar, MACArray[i]);
  }
  
  MACAddrChar[sizeof(MACAddrChar)-1]= 0x00; // remove the last ':'
  Serial.println(MACAddrChar);
 }

/**
 * Einfache Median/Mittelwertberechnung
 * ------------------------------------
 * Übergebenes Array sortieren und über die im Array "in der Mitte" liegenden
 * Werte den Durchschnitt ermitteln.
 * 
 * @todo: übersetzen
 */
unsigned long median(unsigned long *values, int arraySize) {
  unsigned long tmp = 0;            // set to 0, make the compiler happy :-)
  const unsigned long relVal = 2;   // +- 2 Werte + 1 für die Mittelwertberechnung
  
  for (int i=0; i<arraySize; i++) {
    for (int j=arraySize-1; j>i; j--) {
      if( values[j] < values[j - 1] ) {
        tmp = values[j];
        values[j] = values[j - 1];
        values[j - 1] = tmp;
      }
    }
  } 
  for (unsigned long i=arraySize/2-relVal; i<arraySize/2+relVal; tmp +=values[i++]) {}
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
 * 
 * @todo: übersetzen
 */
void bodenfeuchtemessung(int gpioPin) {
  unsigned long speicherArray[MEASURING_INTERVALLS];
  
  for (int i=0; i<MEASURING_INTERVALLS; i++) {
    Serial.print("Measuring No.: "); Serial.print(i); 
    Serial.print(" with GPIO-Pin "); Serial.print(gpioPin);
    Serial.print(" = ");
    counter = 0;
    Serial.flush();

#ifdef SERIAL_OUT_ON_MEASURING
    Serial.end();     
#endif
    
    //  attachInterrupt(gpioPin, intfunc, CHANGE);  
    attachInterrupt(gpioPin, intfunc, RISING);
    delay(MEASURING_TIME);
    detachInterrupt(gpioPin);
    speicherArray[i] = counter;
    
#ifdef SERIAL_OUT_ON_MEASURING
    Serial.begin(TTY_SPEED);
    while (!Serial) {};  // wait for serial port to connect. Needed for native USB
#endif    
    
    Serial.println(counter);
  }
  
  soilMoistAveraged = median(speicherArray, MEASURING_INTERVALLS); 
  Serial.print("average frequence: "); Serial.println(soilMoistAveraged);
//  writeWebLog("[bodenfeuchtemessung:010] Median=" + String(soilMoistAveraged));
}

/**
 * @@@measuringDistance=20000;
 */
void getMeasuringDistanceFromHttpRequest(String s, const char * tag) {
  const int maxValueLength = 15;
  char value[maxValueLength];
    
  measuringDistance = MEASURING_DISTANCE_DEFAULT;  // set default 
  
  char * pos = strstr(s.c_str(), "@@@");
  if (pos > (void*) 0) {
    char * sp = strstr(pos, tag);
    if (sp > (void*) 0) {
      char * va = strstr(sp, "=");
      if (va > (void*) 0) {
        char * ve = strstr(va, ";");
        if (ve+1-va < maxValueLength) {
          memset(value, 0, maxValueLength); 
          strncpy(value, va+1, ve-va-1);
          Serial.print("new measuring distance in ms: "); Serial.println(value);
          measuringDistance = atol(value);        // ok, valid.
        }
      }
    }
  }
}


/**
 * Data transfer to a local data server via http-request (rudimentary REST interface)
 * 
 * local test (only for demonstration!):  
 *   http://192.168.178.23/datalogger/upd_feuchte.php?espno=1&date=2015-08-13_14-22-01&feuchte=123456789&time=16:00:00'  
 */ 
void saveData2NetServer(serverData aServer, int gpio=0) {
  HTTPClient http;
 
  epoch_to_date_time(date, ulSecs2000_timer + millis()/1000); // fresh up the date structure for current time
  
  String request = String(aServer.serverScript) + "?espno=" + ESPNO + "&date=" + getDatum() + "&time=" + getTime() 
          + "&feuchte=" + String(soilMoistAveraged) + "&mac=" + String(MACAddrChar) + "&gpio=" + String(gpio);
          
  Serial.print("Try to get http request: ");  Serial.println(request);
  
  http.begin(aServer.serverAdress, aServer.serverPort, request);
  int httpCode = http.GET();
  if (httpCode) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      Serial.println("[HTTP] GET... successfull");
      // file found at server
      String payload = http.getString();
//      Serial.println(payload);
      getMeasuringDistanceFromHttpRequest(payload, "measuringDistance");
    }
    else Serial.println("[HTTP] GET... error");
  }
}

/**
 * ThingsSpeak 
*/
void saveData2ThingsSpeak(tsData aTS) {
  Serial.println("Try to send to ThingsSpeak...");  
  Serial.println("Try to get REST request: ");  
  String request= "/update?api_key=" + aTS.tsAPIKey + "&field1=" + String(soilMoistAveraged);
  Serial.println(request);

  HTTPClient http;
  http.begin(aTS.tsServerIP.c_str(), 80, request);
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


void loop() {
  unsigned long looptimeStart = millis();
  
  WiFiStart();

  Serial.println("\n--START-MESSUNG--");

#ifdef INTPIN_0
  bodenfeuchtemessung(INTPIN_0);
#ifdef SERVER_1  
  saveData2NetServer(dataServer_1, INTPIN_0);
#endif
#ifdef SERVER_2  
  saveData2NetServer(dataServer_2, INTPIN_0);
#endif
#ifdef THINGSPEAK_1  
  saveData2ThingsSpeak(thingSpeak_data_1);
#endif
#endif  

#ifdef INTPIN_2
  bodenfeuchtemessung(INTPIN_2);
#ifdef SERVER_1  
  saveData2NetServer(dataServer_1, INTPIN_2);
#endif
#ifdef SERVER_2  
  saveData2NetServer(dataServer_2, INTPIN_2);
#endif
#ifdef THINGSPEAK_2  
  saveData2ThingsSpeak(thingSpeak_data_2);
#endif
#endif
  
  Serial.print("measuringDistance       = "); Serial.println(measuringDistance);

  unsigned long looptimeEnde = millis();           
  // whole loope time without delay 
  unsigned long looptime = looptimeEnde - looptimeStart;
 
  Serial.print("looptime          = "); Serial.println(looptimeEnde - looptimeStart);
  Serial.print("measuringDistance (neu) = "); Serial.println(measuringDistance - looptime);
  
  epoch_to_date_time(date, ulSecs2000_timer + (millis() + measuringDistance - looptime)/1000); // fresh up the date structure for current time
   
  Serial.printf(String("Nächste Messung: " + getDatum() + " :: " + getTime() + "\n").c_str());
  
  delay(measuringDistance - looptime);
}










