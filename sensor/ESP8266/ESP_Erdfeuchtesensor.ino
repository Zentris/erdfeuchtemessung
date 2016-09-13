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
    - code for ntp adopted from Michael Margolis
    - code for time_ntp adopted from by Stefan Thesen

  @Known_bugs
    * important recommentation *
    - the power supply must be buffered with a 100nF capacitiy near
      by the chip (best: between the Vcc and Grd connectors on PCB)
      If not, in some cases it can be throw a core dump while
      measurement (Interrrupt loop) !
    * Logserver implementation currently not finished..! Do not switch on!  

  @Open_issues
    - finishing LogServer implementation
    - switching time stamp to MESZ and back
    - watchdog signal (get a blinking LED)
    - translate variables to english
    - translate comments to english

  @Releases: Feature
   1.2 :
    - Sensor can be set to go into deep sleep state (ESP8266-12(e) only)
      and waked up after a configurable time (via reset on GPIO-PIN 16)

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

 */
// ------------------------------------

#define PRG_NAME_SHORT  "MSM-ESP8266"
#define PRG_VERSION     "1.2.0"


/* -------------------------
 * Configuration section
 * ------------------------- */

#define RELEASE         // switch on release mode..
#define DEBUG            // debug mode on/off

#define DEEP_SLEEP_ON    // for using the deep sleep mode pls look into the README.md

//#define LOGSERVER      // uncomment for using the logserver (currently unfinished!)

//#define SERIAL_OUT_ON_MEASURING   // special feature: (test phase!)
                                  // switch out the serial interface
                                  // for measure time:
                                  // it will be increase precision

#define TTY_USE 1        // Switch on/off output of TTY (off = 0)
#define TTY_SPEED 57600  // Serial speed

// - - - - - - - - - - - - -
// use input of following GPIOs (pls read befor set!)
// - - - - - - - - - - - - -
// :: GPIO 0 and GPIO 2 usable on ESP8266-01
// :: GPIO 2 _not_ (!!) usable on ESP8266-12(e)
// ::-> if use the GPIO_2 with ESP8266-12(e), the deep-sleep mode
// ::-> does't work and the sketch will break with a core dump!

//#define INTPIN_0 0     // uncomment if using
#define INTPIN_2 5       // uncomment if using

#define ESPNO 1          // single ESP Number (deprecated (!) )

/* === End of Configuration section === */


#ifdef RELEASE
#undef DEBUG
#endif


/* -------------------------
 * Include section
 * ------------------------- */
#include <Streaming.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

// Expose Espressif SDK functionality
extern "C" {
#include "user_interface.h"
}

#include "time_ntp.h"
#include "privats.h"


/* -------------------------
 * Declaration section
 * ------------------------- */

// Time in milliSec
#define MEASURING_DISTANCE_DEFAULT  120*1000
#define MEASURING_TIME                  1000
#define NTP_REFRESH_AFTER          3600*1000

#define MEASURING_INTERVALLS              10

// default HTTP requestcodes for "ok"
#define HTTP_CODE_OK 200


/* -------------------------
 * Global variables
 * ------------------------- */

unsigned long measuringDistance = MEASURING_DISTANCE_DEFAULT;           // default = 30sec
unsigned long ntpdRefreshTime = 0;    // time if the get the ntp time re-read

uint8_t MACArray[6];                  // return type of mac request
String macID;                         // readable mac address

volatile unsigned long counter = 0;   // interrupt loop counter
unsigned long soilMoistAveraged;  

unsigned long ulSecs2000_timer = 0;   // normalised timestamp in sec from 1.1.2000 00:00:00
date_time_t *date = new date_time_t;  // date structure for output calculation


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
  return String("\n" + String(F(PRG_NAME_SHORT)) + " - " + String(F(PRG_VERSION)) + " / CompTime:"  + __DATE__ + " - " + __TIME__ + "\n");
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
 * Logging function over web interface
 * Write a log string to log server.
 */
#ifdef LOGSERVER
void writeWebLog(String s) {
  HTTPClient http;
  epoch_to_date_time(date, ulSecs2000_timer + millis()/1000); // fresh up the date structure for current time
  String request= String(logserver.serverScript) + "?espno=" + ESPNO + "&date=" + getDatum() + "&time=" + getTime() + "&logtext=" + s;
  Serial.println(request);
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
    Serial << F("Current Time UTC from NTP server: ");
    Serial.println(epoch_to_string(ulSecs2000_timer).c_str());
  }
  ntpdRefreshTime = ulSecs2000_timer + NTP_REFRESH_AFTER; // setze erneutes Zeitholen
}

/**
 *  Starts the WiFi client and get the current time from a ntp server
 */
void WiFiStart() {
    Serial << getSketchVersion();

  int connectionAttempts = 0;
//  while (WiFi.status() != WL_CONNECTED)
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;
    if (connectionAttempts > 120) {
      Serial << endl << F("[Warning]: could not connect to WLAN") << endl;
      Serial << F("  ==> reset now!") << endl;
      ESP.restart();
      delay(1000);     // only for depricating the next print out.
    }
  }
  Serial << endl << F("WiFi connected to ") << WiFi.localIP() << endl;

#ifdef LOGSERVER
  writeWebLog(F("[WiFiStart:010] Wifi connected to " + WiFi.localIP()));
  writeWebLog(F("[WiFiStart:010] Wifi connected"));
#endif

  // connect to NTP and get time, fill dateTime struct
  if (ulSecs2000_timer + millis()/1000 >= ntpdRefreshTime) {
    Serial.print(F("... get the current time from NTP Server "));
    Serial.println(ipAddrNTPServer);
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
 * Measure soil moisture and build the average of measurement values
 */
void bodenfeuchtemessung(int gpioPin) {
  unsigned long speicherArray[MEASURING_INTERVALLS];

  for (int i=0; i<MEASURING_INTERVALLS; i++) {
    Serial.print(F("Measuring No.: ")); Serial.print(i);
    Serial.print(F(" with GPIO-Pin ")); Serial.print(gpioPin);
    Serial.print(F(" = "));
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
  Serial.print(F("average frequence: ")); Serial.println(soilMoistAveraged);

#ifdef LOGSERVER
  writeWebLog("[bodenfeuchtemessung:010] Median=" + String(soilMoistAveraged));
#endif
}

/**
 * Get a special token of http get request string for re-setting the
 * mesurement distance time
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
 *   http://192.168.178.23/datalogger/upd_feuchte.php?espno=1&date=2015-08-13_14-22-01&feuchte=123456789&time=16:00:00'
 */
void saveData2NetServer(serverData aServer, int gpio=0) {
  HTTPClient http;

  epoch_to_date_time(date, ulSecs2000_timer + millis()/1000); // fresh up the date structure for current time

  String request = String(aServer.serverScript) +
                   "?espno=" + ESPNO +
                   "&date=" + getDatum() +
                   "&time=" + getTime() +
                   "&feuchte=" + String(soilMoistAveraged) +
                   "&mac=" + macID +
                   "&gpio=" + String(gpio);

  Serial.print(F("Try to get http request: "));
  Serial.println(request);

  http.begin(aServer.serverAdress, aServer.serverPort, request);
  int httpCode = http.GET();
  if (httpCode) {
    // HTTP header has been send and Server response header has been handled
    Serial.print(F("[HTTP] GET... code: "));
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK) {
      Serial.println(F("[HTTP] GET... successfull"));
      // file found at server
      String payload = http.getString();
#ifdef DEBUG
      Serial.println(payload);
#endif
      getMeasuringDistanceFromHttpRequest(payload, "measuringDistance");
    }
    else Serial.println(F("[HTTP] GET... error"));
  }
}

/**
 * Send data to a valid ThingsSpeak account
*/
void saveData2ThingsSpeak(tsData aTS) {
  Serial.println(F("Try to send to ThingsSpeak..."));
  Serial.print(F("Try to get REST request: "));
  String request= "/update?api_key=" + aTS.tsAPIKey + "&field" + aTS.tsFieldNo + "=" + String(soilMoistAveraged);
  Serial.println(request);

  HTTPClient http;
  http.begin(aTS.tsServerIP.c_str(), 80, request);
  int httpCode = http.GET();
  if (httpCode) {
    // HTTP header has been send and Server response header has been handled
    Serial.print(F("[HTTP] GET... code: "));
    Serial.println(httpCode);

    // don't found at server
    if (httpCode != 200) {
      Serial.print(F("[HTTP] GET... failed, no connection or no HTTP server\n"));
      Serial.println(http.getString());
    }
    else Serial.println(F("[HTTP] GET... successfull"));
  }
}


/* ------------------------------------
 *  S E T U P
 * ------------------------------------ */
void setup() {
  Serial.begin(TTY_SPEED);
  while (!Serial) {};  // wait for serial port to connect. Needed for native USB
  delay(100);          // wait for synchronising
  Serial << getSketchVersion();

// @todo: print out the current version to the web log

//  WiFi.begin(ssid, password);
  WiFiMulti.addAP(ccSSID, ccPASSWORD);
  WiFi.macAddress(MACArray);

  // get the mac address for identifying of the node.
  macID = String(MACArray[0], HEX) + String(MACArray[1], HEX) +
          String(MACArray[2], HEX) + String(MACArray[3], HEX);
  macID.toUpperCase();
  Serial << macID << endl;

//  system_deep_sleep_set_option(0);   // @todo: currently unsecure
}

/* ------------------------------------
 *  M A I N - L O O P
 * ------------------------------------ */
void loop() {
  unsigned long looptimeStart = millis();

  WiFiStart();
  Serial << endl << F("--START-MESSUNG--") << endl;

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

  unsigned long looptimeEnde = millis();
  unsigned long looptime = looptimeEnde - looptimeStart;
  
  if (looptime > measuringDistance) {
    Serial << F("[Warning]: loop time bigger than measuring distance time!") << endl;
    Serial << F("   ==> redefine measuring distance time to 10 * default") << endl;
    Serial << F("   ==> new measuring distance time is ") << String(10*MEASURING_DISTANCE_DEFAULT/1000) << F(" sec. from now") << endl;
    measuringDistance = 10 * MEASURING_DISTANCE_DEFAULT; 
  }
  
  epoch_to_date_time(date, ulSecs2000_timer + (millis() + measuringDistance - looptime)/1000); // fresh up the date structure for current time
  
  Serial << F("measuringDistance (def) = ") << measuringDistance << endl;
  Serial << F("looptime                = ") << looptime << endl;
  Serial << F("measuringDistance (new) = ") << measuringDistance - looptime << endl << endl;
  Serial << F("Next measure time is now: ") << getDatum() << F(" - ") << getTime() << endl;

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










