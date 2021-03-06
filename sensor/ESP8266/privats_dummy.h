#ifndef ESP_PRIVATS
#define ESP_PRIVATS

/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +

   === README ===

[german]
1. Kopiere das File 'privats_dummy.h' nach 'privats.h'
2. Passe das File 'privats.h' an, (Zugriffsdaten (Platzhalter:<***>)  befüllen).
3. Speichern und compilieren

[english]
1. Copy the file 'privats_dummy.h' to 'privats.h' in the same directory.
2. Change the content of the File 'privates.h' (fill access data (placeholder: <***>))
3. Save and compile

  + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + */

/* ---------------------------------------------
   --- Wifi- access data
   ---------------------------------------------
   It can defined more than one (unlimited!) access point for better mobility:

   Example:
   struct accessPoint apList[] = { (accessPoint) { "fritzbox",  "1234567890" },
                                   (accessPoint) { "bluelan",   "myHomeIsMyCastle" }
                                   (accessPoint) { "firepoint", "hotIceWater" }
                                 };
*/
accessPoint apList[] = { (accessPoint) {"<your first AP name>",  "<pw of your first AP>"}
                        ,(accessPoint) {"<your second AP name>", "<pw of your second AP>"}
//                        ,(accessPoint) {"<your next AP name>",   "<pw of your next AP>"}
                       };


// ---------------------------------------------
// --- data server struct ---
// ---------------------------------------------
// Example: struct serverData dataServer_1 = { "192.168.1.14", 80, "/homecontrol/upd_feuchte.php" };
//          struct serverData dataServer_2 = { "soil.moisture.net", 8080, "/upd_feuchte.php" };
//
serverData dataServer[] = { (serverData) {"<1. Server_ip-or_hostname>", <port>, "<server script with path>"}
//                           ,(serverData) {"<2. Server_ip-or_hostname>", <port>, "<server script with path>"}
                           ...
                          };

// ---------------------------------------------
// --- additional data sources ---
// ---------------------------------------------
// struct collectDataSet {const char* connectionString; unsigned int sensorId; unsigned int tsFieldNo; String buffer};
// ---------------------------------------------
collectDataSet addDataSources[] = { (collectDataSet) {"http://192.168.178.152/weigth", 2, 3, ""} // Weight "Einblatt"
                                   ,(collectDataSet) {"http://192.168.178.152/temp",   2, 4, ""} // Temperature HX711
                              };


// ---------------------------------------------
// --- Thingspeak API connection data struct ---
// ---------------------------------------------
// tsServer   = Thingspeak domain name (normally fix)
// tsServerIP = Thingspeak server ip (default is a static ip)
// tsAPIKey   = individual API key for our channel
// tsFieldNo  = Number of field in our channel where the data will be shown
// ---------------------------------------------
tsData thingSpeakServer[] = { (tsData) {"api.thingspeak.com", "184.106.153.149", "<our API key>", "<our 1. field number>"}
                             ,(tsData) {"api.thingspeak.com", "184.106.153.149", "<our API key>", "<our 2. field number>"}
//                           ...
                            };


// ---------------------------------------------
// my sensor GPIO definition (must be adapt on your own hardeware configuration !
// ---------------------------------------------
// struct mySensor { unsigned int sensorId;            // sensor id
//                   unsigned int intGPIO;             // frequence signal line
//                   unsigned int powerGPIO;           // power line (in case of power off between mesurements)
//                   unsigned long soilMoistAveraged;  // internal average value, set herer to 0
//                 };
// here example data only (i have 2 sensors)
// ---------------------------------------------
mySensor sensors[] { (mySensor) { 1, 12, 14, 0 } // my Birkenfeige (Ficus benjamina)
                    ,(mySensor) { 2,  5,  4, 0 } // my Einblatt (Spathiphyllum)
};


// ---------------------------------------------
// --- Logserver (if needed) ---
// ---------------------------------------------
// (remove the comment of following line for activate)
#ifdef LOGSERVER
struct serverData logserver = { "<IP of log server>", <port>, "<server script>" };
#endif

#endif
