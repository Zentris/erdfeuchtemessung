#ifndef ESP_PRIVATS
#define ESP_PRIVATS

/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
   
   === README ===

[german]
1. Kopiere das File 'privats_dummy.h' nach 'privats.h'
2. Passe das File 'privats.h' an, (Zugriffsdaten (Platzhalter:<***>)  befüllen und Funktionalität zuschalten).
3. Speichern und compilieren

[english]
1. Copy the file 'privats_dummy.h' to 'privats.h' in the same directory.
2. Change the content of the File 'privates.h' (fill access data (placeholder: <***>), uncomment the #define lines if you need the functionality).
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
struct accessPoint {const char* SSID; const char* PASSWORD;};
struct accessPoint apList[] = { (accessPoint) {"<your first AP name>",  "<pw of your first AP>"} 
                               ,(accessPoint) {"<your second AP name>", "<pw of your second AP>"} 
//                             ,(accessPoint) {"<your next AP name>",   "<pw of your next AP>"} 
                              };

// ---------------------------------------------
// --- data server struct ---
// ---------------------------------------------
struct serverData {const char* serverAdress; const int serverPort; const char* serverScript; };
// ---------------------------------------------
// --- first data server (if needed) ---
// ---------------------------------------------
// (remove the comment of following line for activate)
//
// Example: struct serverData dataServer_1 = { "192.168.1.14", 80, "/homecontrol/upd_feuchte.php" };
//          struct serverData dataServer_2 = { "soil.moisture.net", 80, "/upd_feuchte.php" };
//
//#define SERVER_1
#ifdef SERVER_1
struct serverData dataServer_1 = { "<IP of data server 1>", <port>, "<server script>" };
#endif

// ---------------------------------------------
// --- second data server (if needed) ---
// ---------------------------------------------
// (remove the comment of following line for activate)
//#define SERVER_2
#ifdef SERVER_2
struct serverData dataServer_2 = { "<IP of data server 2>", <port>, "<server script>" };
#endif

// ---------------------------------------------
// --- Logserver (if needed) ---
// ---------------------------------------------
// (remove the comment of following line for activate)
//#define LOGSERVER
#ifdef LOGSERVER
struct serverData logserver = { "<IP of log server>", <port>, "<server script>" };
#endif

// ---------------------------------------------
// --- NTP server ip (b1.b2.b3.b4)
// ---------------------------------------------
struct { int b1 = 129;  int b2 =   6;  int b3 =  15;  int b4 =  28; } ntpIP; // time.nist.gov NTP server

// ---------------------------------------------
// --- Thingspeak API connection data struct ---
// ---------------------------------------------
// tsServer   = Thingspeak domain name (normally fix)
// tsServerIP = Thingspeak server ip (default is a static ip)
// tsAPIKey   = individual API key for our channel
// tsFieldNo  = Number of field in our channel where the data will be shown
//
struct tsData {String tsServer; String tsServerIP; String tsAPIKey; String tsFieldNo;};

// (remove the comment of following line for activate)
//#define THINGSPEAK_1
#ifdef THINGSPEAK_1
struct tsData thingSpeak_data_1 = { "api.thingspeak.com", "184.106.153.149", "<our API key>", "<our field number>" };
#endif

// (remove the comment of following line for activate)
//#define THINGSPEAK_2
#ifdef THINGSPEAK_2
struct tsData thingSpeak_data_2 = { "api.thingspeak.com", "184.106.153.149", "<our API key>", "<our field number>" };
#endif



#endif
