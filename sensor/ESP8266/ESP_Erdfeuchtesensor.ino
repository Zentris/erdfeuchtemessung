/*
                 GNU GENERAL PUBLIC LICENSE
                  Version 3, 29 June 2007

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  Erdfeuchtemessung mit ESP8266-01 / ESP8266-12(e)
  Measure soil moisture with ESP8266-01 / ESP8266-12(e)

  (2016)

  @Autors:
    - Ralf Wießner (aka Zentris)

  @Credits:
    - ntp adopted from Michael Margolis
    - time_ntp adopted from by Stefan Thesen

  @Known_bugs
    * important recommentation *
    - the power supply must be buffered with a 100nF capacitiy near
      by the chip (best: between the Vcc and Grd connectors on PCB)
      If not, in some cases it can be throw a core dump while
      measurement (Interrrupt loop) !
    * Logserver implementation currently not finished..! Do not switch on!

  @Open_issues
    - finishing LogServer implementation and testing
    - switching time stamp to MESZ and back
    - translate all variables to english
    - translate all comments to english

  @Releases: Feature
   2.0.0 : 
    - ESPNO define removed, the MAC and sensorID will be used instead
      - this will be necessary changes into 
        * the database (add sensorid field and change indices)
        * the upd_feuchte.php file (REST IF)
        * the benjamin.php file (sql queries)
    - watchdog signal (Onboard-LED on GPIO 2) will be used for signaling 
      the measurement loop
    - changed privats.h format (only structures now, more simplify)
    - Sensor will be switched off to powerless state if no measurement
      ongoing. This feature is only tested with the ESP8266-12, it cut be 
      possible to work also with the ESP8266-01 (currently not tested)
    - Support of 2 ore more (depends of free GPIO pins on our ESP: 
      * each sensor needs 2 GPIO: frequencey counter and power line
    - small correction into the median calculation
   
   1.2 :
    - Sensor can be set to go into deep sleep state (ESP8266-12(e) only)
      and waked up after a configurable time (via reset on GPIO-PIN 16)
    - ask the system for free ram
    - retry sending to Thingspeak if currently not reachable up to 10 times

   1.1 :
    - move private data to private.h file (and add a default dummy: private_dummy.h)
    - correct a counter bug in time_ntp for precise time calculation
    - rename many variable and constants for proper english
    - use MAC for ESP identifiyer

   1.0 :
    - setting the measurement intervals via HTTP request answer
    - 2 measurment canals (GPIO0 and GPIO2)
    - data transfer to local data server
    - data transfer to remote Thingsspeak server (optional)
    - 10 probs on each measurement loop
    - create median and create the mean value over 5 media values
    - use a local timeserver (here a fritzbox)
      (adoption of time_ntp)

  @My Arduino V.1.6.10 programmer settings
  * Board: ESPino (ESP-12 Module)
  * Flash-Mode: DIO
  * CPU-Frequency: 80MHz 
  * Flash-Size: 4M (1M SPIFFS)
  * Reset Methos: ck
  * Upload-Speed: 115200 (recommented)
  * Programmer: AVRISP mkII (for my PL2303 USB2Serial Adapter) 

 */
// ------------------------------------

#define PRG_NAME_SHORT  "MSM-ESP8266"
#define PRG_VERSION     "2.0.0"

/* -------------------------
 * First include section
 * ------------------------- */

#include <Streaming.h>      // from: http://arduiniana.org/libraries/streaming/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

// Expose Espressif SDK functionality
extern "C" {
#include "user_interface.h"
}

#include "time_ntp.h"


/* -------------------------
 * Configuration section
 * ------------------------- */
#define ESP8266_12       // recomment if your target device a ESP8266-12
//#define ESP8266_01       // recomment if your target device a ESP8266-01

#define RELEASE          // switch on release mode.. (no debug mode, small foortprint)
#define DEBUG            // debug mode on/off

#ifdef RELEASE           // RELEASE deactivates the DEBUG mode!
#undef DEBUG
#endif

#define DEEP_SLEEP_ON    // for using the deep sleep mode pls look into the README.md

//#define LOGSERVER       // uncomment for using the logserver (currently untested!)

//#define SERIAL_OUT_ON_MEASURING   // special feature: (test phase!)
                         // switch out the serial interface
                         // for measure time:
                         // it will be increase precision

#define TTY_USE 1        // Switch on/off output of TTY (off = 0)
#define TTY_SPEED 57600  // Serial speed

#define LED_GPIO 2       // internal LED ! (blue)


#define MEASURING_DISTANCE_DEFAULT  300*1000  // all times in milliSec
#define MEASURING_TIME                   100  // how long a sample lasts


#define NTP_REFRESH_AFTER          3600*1000  // after what time the ntp 
                                              // timestamp will be refreshed
                                              // (only continous mode)

#define MEASURING_INTERVALLS              9   // how many samples are taken?

// - - - - - - - - - - - - -
// use input of following GPIOs (pls read befor set!)
// - - - - - - - - - - - - -
// :: GPIO 0 and GPIO 2 usable on ESP8266-01
// :: GPIO 2 _not_ (!!) usable on ESP8266-12(e)
// ::-> if use the GPIO_2 with ESP8266-12(e), the deep-sleep mode
// ::-> does't work and the sketch will break with a core dump!

//#define ESPNO 1          // single ESP Number (deprecated (!) )


/* ---------------------------------------------
   --- Sensor definition data ---
   ---------------------------------------------
   :: (?) Sensor data 
   sensorId          = the sensor id and at the same time the field number
                       in your Thingspeak account
   intGPIO           = the measurement GPIO where the frequecy will be determine
   powerGPIO         = the power pin where the sensor switched on or off (not for ESP8266-01!)
   soilMoistAveraged = the current soil moist value after measurement
*/
struct mySensor {unsigned int sensorId; unsigned int intGPIO; unsigned int powerGPIO; unsigned long soilMoistAveraged;};

// my sensor GPIO definition (must be adapt on your own hardeware configuration !
struct mySensor sensors[] { (mySensor) { 2,  5,  4, 0 } 
                           ,(mySensor) { 1, 12, 14, 0 } 
                          };

/* ---------------------------------------------
   --- NTP server ip ---
   ---------------------------------------------
*/
struct { int b1 = 192;  int b2 = 168;  int b3 = 178;  int b4 =  1; } ntpIP; // Fritzbox
//struct { int b1 = 129;  int b2 =   6;  int b3 =  15;  int b4 =  28; } ntpIP; // time.nist.gov NTP server


/* === End of Configuration section === */


/* -------------------------
 * Structure definition section for using in privats.h
 * ------------------------- */

/* ---------------------------------------------
   --- Wifi- access data ---
   ---------------------------------------------
   :: (?) It can be stored more than one WiFi connection - the device check 
          for availability and connect to the first was found
   :: (!) This structure should be defined into the privats.h file!
   SSID      = the WiFi SSID name to connect
   PASSWORD  = the WiFi identification password
*/
struct accessPoint {const char* SSID; const char* PASSWORD;};

/* ---------------------------------------------
   --- Data server ---
   ---------------------------------------------
   :: (?) Server to receive measurement data via primitive REST 
          interface implementation
   :: (!) This structure should be defined into the privats.h file!
   serverAdress  = server IP or Server name without port
   serverPort    = specified port number of the server where the service
                   will be offered
   serverScript  = server script path/script-name are be called 
*/
struct serverData {const char* serverAdress; const int serverPort; const char* serverScript; };

/* ---------------------------------------------
   --- Thingspeak API connection data ---
   ---------------------------------------------
   :: (?) Connection data for a Thingspeak account to sent the measurement
          data to one or more data representation fields
   :: (!) This structure should be defined into the privats.h file!
   tsServer   = Thingspeak domain name
   tsServerIP = Thingspeak server ip
   tsAPIKey   = individual API key for our channel
   tsFieldNo  = number of field in our channel where the data will be shown
                The field number refereced with the sensorId number from 
                the Sensor definition data structure (!)
                It means, the number sensorId number is the field number 
                off Thingspeak fields.
*/
struct tsData {String tsServer; String tsServerIP; String tsAPIKey; unsigned int tsFieldNo;};


/* -------------------------
 * Declaration section
 * ------------------------- */
#include "privats.h"

#define HTTP_CODE_OK 200    // default HTTP requestcodes for "ok"


/* -------------------------
 * Global variables
 * ------------------------- */

unsigned long measuringDistance = MEASURING_DISTANCE_DEFAULT;           // default = 300sec
unsigned long ntpdRefreshTime = 0;    // time if the get the ntp time re-read

uint8_t MACArray[6];                  // return type of mac request
String macID;                         // readable mac address

volatile unsigned long counter = 0;   // interrupt loop counter

unsigned long ulSecs2000_timer = 0;   // normalised timestamp in sec from 1.1.2000 00:00:00
date_time_t *date = new date_time_t;  // date structure for output calculation

unsigned long looptimeStart = 0;      // loop timer: start time

/* -------------------------
 *  define/init classes
 * ------------------------- */
ESP8266WiFiMulti WiFiMulti;
IPAddress ipAddrNTPServer(ntpIP.b1, ntpIP.b2, ntpIP.b3, ntpIP.b4);    // local fritz box

/* -------------------------
 *  declare classes and methods
 * ------------------------- */

/**
 * Get back the current Sketch name and version.
 */
String getSketchVersion() {
  return String("\n" + String(F(PRG_NAME_SHORT)) + " - " +
         String(F(PRG_VERSION)) + F(" / CompTime:")  +
         __DATE__ + F(" - ") + __TIME__ + F("\n"));
}

/**
 * Create a date string with format "YYYY-MM-DD" from global 'date' variable.
 * @return: String Format: "YYYY-MM-DD"
 */
String getDatum() {
  char s[12];
  sprintf(s, "%04d.%02d.%02d", date->year+2000, date->month, date->day);
  return(String(s));
}

/**
 * Create a time string with format"hh:mm:ss" from global 'date' variable.
 * @return: String Format: "hh:mm:ss"
 */
String getTime() {
  char s[12];
  sprintf(s, "%02d:%02d:%02d", date->hour, date->minute, date->second);
  return(String(s));
}

/**
 * Convert the given  address array into a printable format
 * @return: "111.222.333.444" - Format (decimal)
 */
const String getCurrentIpAsString(IPAddress aIp) {
  char s[18];
  sprintf(s, "%d.%d.%d.%d", aIp[0], aIp[1], aIp[2], aIp[3]);
  return(String(s));
}

/**
 * Logging function over web interface
 * Write a log string to log server.
 */
#ifdef LOGSERVER
void writeWebLog(String s) {
  HTTPClient http;
  epoch_to_date_time(date, ulSecs2000_timer + millis()/1000); // fresh up the date structure for current time
  String request= String(logserver.serverScript) + "?mac=" + macID + "&date=" + getDatum() + "&time=" + getTime() + "&logtext=" + s;
  Serial << request << endl;
  http.begin(logserver.serverAdress, logserver.serverPort, request);
  http.GET(); // fire and forget...
}
#endif


/**
 * Get the current time (UTC) from NTP server and fill the 'date" structure
 */
void getDateTimeNTP() {
  ulSecs2000_timer=getNTPTimestamp(ipAddrNTPServer)+3600;    // add +3600 sec (1h) for MET
  epoch_to_date_time(date, ulSecs2000_timer); // fresh up the date structure
  ulSecs2000_timer -= millis()/1000;          // keep distance to millis counter at now
  if (TTY_USE) {
    Serial << F("Current Time UTC from NTP server: ") << epoch_to_string(ulSecs2000_timer) << endl;
  }
  ntpdRefreshTime = ulSecs2000_timer + NTP_REFRESH_AFTER; // setze erneutes Zeitholen
}

/**
 *  Starts the WiFi client and get the current time from a ntp server
 */
void WiFiStart() {
  Serial << getSketchVersion();

  int connectionAttempts = 0;
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial << F(".");
    connectionAttempts++;
    if (connectionAttempts > 120) {
      Serial << endl << F("[Warning]: could not connect to WLAN") << endl;
      Serial << F("  ==> reset now!") << endl;
      ESP.restart();
      delay(1000);     // only for depricating the next print out.
    }
  }
  Serial << endl << F("WiFi connected, local IP: ") << WiFi.localIP() << endl;

#ifdef LOGSERVER
  String s = "[WiFiStart:010] Wifi connected to ";
  s += getCurrentIpAsString(WiFi.localIP());
  writeWebLog(s);
#endif

  // connect to NTP and get time, fill dateTime struct
  if (ulSecs2000_timer + millis()/1000 >= ntpdRefreshTime) {
    Serial << F("... get the current time from NTP Server ") << ipAddrNTPServer << endl;
    getDateTimeNTP();

#ifdef LOGSERVER
    char sl[100];
    sprintf(sl, "[WiFiStart:020] get time from NTP Server %d.%d.%d.%d", ntpIP.b1, ntpIP.b2, ntpIP.b3, ntpIP.b4);
 //   String s = F("[WiFiStart:020] get time from NTP Server ") + ipAddrNTPServer;
    writeWebLog(String(sl));
#endif
  }
}

/**
 * Simple median/average calculation
 * ------------------------------------
 * Getting array will be sorted and now get the average over the middle values
 */
unsigned long median(unsigned long *values, size_t arraySize) {
  unsigned long tmp = 0;     // set to 0, make the compiler happy :-)
  const size_t relVal = 2;   // +- 2 Werte + 1 für die Mittelwertberechnung

  for (size_t i=0; i < arraySize-2; i++) {
    for (size_t j=arraySize-1; j > i; j--) {
      if ( values[j] < values[j-1] ) {
        tmp = values[j];
        values[j] = values[j-1];
        values[j-1] = tmp;
      }
    }
  }

#ifdef DEBUG
  for (size_t x=0; x<arraySize; x++) {
    Serial << F("sorted array[") << x << F("] = ") << values[x] << endl;
  }
#endif  
  
  tmp = 0;
  for (size_t i=arraySize/2-relVal; i<arraySize/2+relVal+1; tmp +=values[i++]) {}
  return tmp/(relVal*2+1) * 1000/MEASURING_TIME;
}

/**
 * Interrupt function: increment the counter
 */
void intfunc() {
  counter++;
}

/**
 * Measure soil moisture and build the average of measurement values
 * struct mySensor {unsigned int sensorId; unsigned int intGPIO; unsigned int powerGPIO;};
 */
void bodenfeuchtemessung(mySensor &sensordata) {
  unsigned long dataArray[MEASURING_INTERVALLS];
  
  Serial << F("power on GPIO: ") << sensordata.powerGPIO << endl;
  digitalWrite(sensordata.powerGPIO, HIGH);  // switch sensor power on
  delay(500);                           // wait for stabilizing sensor

  for (size_t i=0; i < MEASURING_INTERVALLS; i++) {
    Serial << F("SensorID: ") << sensordata.sensorId << F(" - Measuring No.: ") << i << F(" on GPIO-Pin ") << sensordata.intGPIO << F(" = ");
    counter = 0;
    Serial.flush();

#ifdef SERIAL_OUT_ON_MEASURING
    Serial.end();
#endif
    digitalWrite(LED_GPIO, HIGH);  // signaling measurent

    attachInterrupt(sensordata.intGPIO, intfunc, RISING);
    delay(MEASURING_TIME);
    detachInterrupt(sensordata.intGPIO);
    dataArray[i] = counter;

    digitalWrite(LED_GPIO, LOW);   // singnaling out
    
#ifdef SERIAL_OUT_ON_MEASURING
    Serial.begin(TTY_SPEED);
    while (!Serial) {};  // wait for serial port to connect. Needed for native USB
#endif

    Serial << counter << endl;
    delay(100);         // wait a small time
  }

  Serial << F("power off GPIO: ") << sensordata.powerGPIO << endl;
  digitalWrite(sensordata.powerGPIO, LOW);   // switch sensor power off

  sensordata.soilMoistAveraged = median(dataArray, MEASURING_INTERVALLS);
  
  Serial << F("average frequence: ") << sensordata.soilMoistAveraged << endl;

#ifdef LOGSERVER
  writeWebLog("[bodenfeuchtemessung:010] Median=" + String(sensordata.soilMoistAveraged));
#endif
}

/**
 * Get a special token of http get request string for re-setting the
 * mesurement distance time in ms
 *
 * Token: @@@measuringDistance=20000;
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
          Serial << F("new measuring distance in ms: ") << value << endl;
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
 *   http://192.168.178.23/datalogger/upd_feuchte.php?date=2015-08-13_14-22-01&feuchte=123456789&time=16:00:00'
 */
void saveData2NetServer(serverData aServer, mySensor aSensor) {
  HTTPClient http;

  epoch_to_date_time(date, ulSecs2000_timer + millis()/1000); // fresh up the date structure for current time

  String request = String(aServer.serverScript) +
                   "?mac=" + macID +
                   "&sensorid=" + String(aSensor.sensorId) +
                   "&gpio=" + String(aSensor.intGPIO) +
                   "&date=" + getDatum() +
                   "&time=" + getTime() +
                   "&feuchte=" + String(aSensor.soilMoistAveraged);

  Serial << F("Connect and send REST request for server ") << aServer.serverAdress << endl;
#ifdef DEBUG
  Serial << F("Request: ") << request << endl;
#endif

  http.begin(aServer.serverAdress, aServer.serverPort, request);
  int httpCode = http.GET();
  if (httpCode) {
    // HTTP header has been send and Server response header has been handled
    Serial << F("[HTTP] GET... code: ") << httpCode;
    String payload = http.getString();

    if (httpCode == HTTP_CODE_OK) {
      Serial << F("   ... successfull") << endl;
      // file found at server
#ifdef DEBUG
      Serial << payload << endl;
#endif
      getMeasuringDistanceFromHttpRequest(payload, "measuringDistance");
    }
    else {
      Serial << F("   ... error: ") << endl;
      Serial << payload << endl;
    }
    
  }
  Serial << endl;
}

/**
 * Send data to a valid ThingsSpeak account
*/
void saveData2ThingsSpeak() {
  const int maxSendRepeats = 10;
  const int waitTimeBetweenSend_ms = 5000; // 5 sec.
  
  String request= "/update?api_key=" + thingSpeakServer[0].tsAPIKey; 
  
  char buf[20];
  for (unsigned int sensor=0; sensor < sizeof(sensors)/sizeof(struct mySensor); sensor++) {
    for (unsigned int tss=0; tss < sizeof(thingSpeakServer)/sizeof(struct tsData); tss++) {

      // send only once for field number == sensor id 
      if (thingSpeakServer[tss].tsFieldNo == sensors[sensor].sensorId) {
        sprintf(buf, "&field%d=%d", thingSpeakServer[tss].tsFieldNo, sensors[sensor].soilMoistAveraged);
        request += String(buf);
      }
    }
  }
  
//  String request= "/update?api_key=" + aTS.tsAPIKey + "&field" + aTS.tsFieldNo + "=" + String(soilMoistAveraged);
  
  Serial << F("Try to send to ThingsSpeak ") << endl;
#ifdef DEBUG
  Serial << F("REST request: ") << request << endl;
#endif

  HTTPClient http;
  int sendLoop = 0;
  do {
    http.begin(thingSpeakServer[0].tsServerIP.c_str(), 80, request);
    int httpCode = http.GET();
    if (httpCode) {
      // HTTP header has been send and Server response header has been handled
      Serial << F("[HTTP] GET... code: ") << httpCode;

      // don't found at server ?
      if (httpCode != 200) {
        Serial << F("   ... failed, no connection or no HTTP server\n") << http.getString() << endl;
      }
      else {
        Serial << F("   ... successfull") << endl;
        return;
      }
    }
    sendLoop++;
    delay(waitTimeBetweenSend_ms);
  } while ( sendLoop < maxSendRepeats);

  if (sendLoop >= maxSendRepeats) {
    Serial << F("Can't sent datagram to Thingspeak server, not reachable.") << endl;
  }
  Serial << endl;
}


/* ------------------------------------
 *  S E T U P
 * ------------------------------------ */
void setup() {
#ifdef DEEP_SLEEP_ON
  looptimeStart = millis();
#endif

  Serial.begin(TTY_SPEED);
  while (!Serial) {};  // wait for serial port to connect. Needed for native USB
  delay(100);          // wait for synchronising
  Serial << getSketchVersion();

#ifdef RELEASE
  Serial << F("Release version") << endl;
#endif

#ifdef DEEP_SLEEP_ON
  Serial << F("deep sleep mode on") << endl;
#else
  Serial << F("deep sleep mode off") << endl;
#endif

  // set multiple access points for better mobility
  for (unsigned int i=0; i < sizeof(apList)/sizeof(struct accessPoint); i++) {
    Serial << F("add AP(") << i << F(") : ") << apList[i].SSID << endl;
    WiFiMulti.addAP(apList[i].SSID, apList[i].PASSWORD);
  };

  // get the mac address for identifying of the node.
  WiFi.macAddress(MACArray);
  macID = String(MACArray[0], HEX) + String(MACArray[1], HEX) +
          String(MACArray[2], HEX) + String(MACArray[3], HEX);
  macID.toUpperCase();
  Serial << F("MAC: ") << macID << F("   ====   ChipId: ") << ESP.getChipId() << endl;
  
  // initialise the GPIO pins for all sensors are configured
  Serial << F("Initialise GPIOs") << endl;
  for (unsigned int i=0; i < sizeof(sensors)/sizeof(struct mySensor); i++) {
    pinMode(sensors[i].intGPIO, INPUT_PULLUP);
    pinMode(sensors[i].powerGPIO, OUTPUT);
#ifdef DEBUG
    Serial << i << F(" pwr: ") << sensors[i].powerGPIO << endl;
    Serial << i << F(" int: ") << sensors[i].intGPIO << endl; 
#endif
    digitalWrite(sensors[i].powerGPIO, LOW);
  }  

  // Signaling LED ready for take of ...
  pinMode(LED_GPIO, OUTPUT);
  for (int i= 0; i < 10; i++) {
    digitalWrite(LED_GPIO, HIGH);   delay(100);
    digitalWrite(LED_GPIO, LOW);    delay(100);
  }
}

/* ------------------------------------
 *  M A I N - L O O P
 * ------------------------------------ */
void loop() {
#ifndef DEEP_SLEEP_ON
  looptimeStart = millis();
#endif
  delay(1000);  // short wait for WiFi

  WiFiStart();

//  Serial << endl << F("free heap:") << system_get_free_heap_size() << F(" byte") << endl;
  Serial << F("free heap:") << ESP.getFreeHeap() << F(" byte") << endl;
  
  delay(1000);  // short wait for WiFi

  for (unsigned int sensor=0; sensor < sizeof(sensors)/sizeof(struct mySensor); sensor++) {
    Serial << endl << F("---BEGIN MEASURMENT Sensor No.: ") << sensors[sensor].sensorId << F(" ---") << endl;
    bodenfeuchtemessung(sensors[sensor]);
  
    for (unsigned int server=0; server < sizeof(dataServer)/sizeof(struct serverData); server++) {
      saveData2NetServer(dataServer[server], sensors[sensor]);
    }
  }
   
  //  
  saveData2ThingsSpeak();
   
    
  unsigned long looptimeEnde = millis();
  unsigned long looptime = looptimeEnde - looptimeStart;

  if (looptime > measuringDistance) {
    Serial << F("[Warning]: loop time bigger than measuring distance time!") << endl;
    Serial << F("   ==> redefine measuring distance time to 5 * default") << endl;
    Serial << F("   ==> new measuring distance time is ") << String(5*MEASURING_DISTANCE_DEFAULT/1000) << F(" sec. from now") << endl;
    measuringDistance = 5 * MEASURING_DISTANCE_DEFAULT;
  }

  epoch_to_date_time(date, ulSecs2000_timer + (millis() + measuringDistance - looptime)/1000); // fresh up the date structure for current time

  Serial << F("measuringDistance (def) = ") << measuringDistance << endl;
  Serial << F("looptime                = ") << looptime << endl;
  Serial << F("measuringDistance (new) = ") << measuringDistance - looptime << endl << endl;
  Serial << F("Next measure time is now: ") << getDatum() << F(" - ") << getTime() << F(" (UTC)") << endl;

#ifdef DEEP_SLEEP_ON
  Serial << F("Going into deep sleep for around ") << ((measuringDistance-looptime)/1000) << F(" sec") << endl;

// mode is one of `WAKE_RF_DEFAULT`, `WAKE_RFCAL`, `WAKE_NO_RFCAL`, `WAKE_RF_DISABLED`
  ESP.deepSleep((measuringDistance - looptime)*1000, WAKE_RF_DEFAULT);   // need usec!

//  ESP.deepSleep(10*1000000);
//  system_deep_sleep((measuringDistance - looptime)*1000); // use us !
#else
  Serial << F("Wait now....") << endl;
  delay(measuringDistance - looptime);
#endif

}










