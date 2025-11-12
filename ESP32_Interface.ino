
//Empfängt den Datenstrom des Raspberry und gibt diesen an Virtuino weiter

#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include "VirtuinoCM.h"
#include "Zugang.h"
#include "time.h"
#include "esp_task_wdt.h"

#define WDT_TIMEOUT 30		//Timeout in Sekunden (Watchdog)
#define EEPROM_SIZE 10		// Anzahl der Bytes, die im Flash reserviert werden sollen

// UDP-Parameter
WiFiUDP udp;
const int localUdpPort = 1902; // Port, auf dem der ESP32 hört

uint8_t Byts[360];
size_t BytsLen = 0;
char packetBuffer[1500]; // Puffer für eingehende Pakete

#define V_memory_count 82         // the size of V memory. You can change it to a number <=255)
float V[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.
String Text_0 = "";                      // This text variable is synchronized with the Virtuino pin Text_0 
String Text_1 = "";                      // This text variable is synchronized with the Virtuino pin Text_1
float S[V_memory_count];           // This array is synchronized with Virtuino V memory. You can change the type to int, long etc.

float NiveauAlt1;
float NiveauAlt2;
float NiveauAlt3;
float NiveauAltPool;

float GasGestern;
float WasserGestern;
float BrauchwasserGestern;
float StromGestern;
float EingespeistGestern;

float GasDiff;
float WasserDiff;
float BrauchwasserDiff;
float StromDiff;
float EingespeistDiff;

float GasHeute;
float WasserHeute;
float BrauchwasserHeute;
float StromHeute;
float EingespeistHeute;

WiFiServer server(6222);                   // Default Virtuino Server port 
WiFiServer server80(80);

VirtuinoCM virtuino;

const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";    //Wird benötigt für NTP-Zeit
struct tm timeinfo;

int Neustarts;

bool debug = false;
unsigned long Sendezeit = millis();
const int Sendepause = 3000;  //Sendepause in Sekunden zur Datenuebertragungin msec
String StandPV1 = "";

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  virtuino.begin(onReceived, onRequested, 256);  //Start Virtuino. Set the buffer to 256. With this buffer Virtuino can control about 28 pins (1 command = 9bytes) The T(text) commands with 20 characters need 20+6 bytes
 //virtuino.key="1234";        //This is the Virtuino password. Only requests the start with this key are accepted from the library
  Serial.println("Starte UDP-Empfänger");

  // WLAN-Verbindung herstellen
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden.");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  server.begin();
  server80.begin();	//für die HTML-Seite
 

ZeitSetzen();			//NTP Server abfragen

  // UDP starten
  udp.begin(localUdpPort);
  Serial.print("Lausche am UDP port ");
  Serial.println(localUdpPort);

  //Neustarts um 1 erhöhen
  Neustarts = EEPROM.read(0);
  Neustarts++;
  Serial.print("Neustart des Interface mit Watchdog: ");
  Serial.println(Neustarts);
  EEPROM.write(0, Neustarts);
  EEPROM.commit(); // Damit die Daten tatsächlich in den Flash geschrieben werden

  //CheckServer();  //festlegen ob Raspberry-Host speicher oder heisopi ist

  StandGesternVonRaspberryLesen();
  AktuellerZaehlerstand(); //Gas und Wasser
 
}

void loop() {
  
  
  virtuinoRun();        // Necessary function to communicate with Virtuino. Client handler
  HTMLServer();

  if (millis() > Sendezeit + Sendepause)  //wenn die Zeit sich ge�ndert hat
  {
	  Sendezeit = millis();
	  SchreibeWert("Zeit", AktuelleZeit());
	  SchreibeWert("Datum", AktuellesDatum());
	  SchreibeWert("Neustarts",  String(Neustarts));

	  DatenAbholen();
	 if (GasGestern < 9300 || WasserGestern < 180 || BrauchwasserGestern < 1950 )  StandGesternVonRaspberryLesen();
	
	
	  AktuellerZaehlerstand(); //Gas und Wasser, Brauchwasser

	  SchreibeWert("Gas", String(GasDiff));
	  SchreibeWert("Wasser", String(WasserDiff));
	  SchreibeWert("Strom", String(StromDiff));
	  SchreibeWert("Eingespeist", String(EingespeistDiff));
	  SchreibeWert("Brauchwasser", String(BrauchwasserDiff));

	  ZaehlerstandPV1Lesen();
	  Serial.println();
 }

  VituinoAbfragen();
  PCBefehlAbfragen();

  if (timeinfo.tm_year < (2020 - 1900)) {
	 Serial.println("❌ Zeit ungültig (vermutlich 1970-01-01)!");
	 ZeitSetzen();
 }

 esp_task_wdt_reset(); //Reset watchdog
}
