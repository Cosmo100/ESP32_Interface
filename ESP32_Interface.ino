
//Empfängt den Datenstrom des Raspberry und gibt diesen an Virtuino weiter

#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include "esp_task_wdt.h"
#include "VirtuinoCM.h"
#include "Zugang.h"


#define WDT_TIMEOUT 10		//Timeout in Sekunden (Watchdog)
#define EEPROM_SIZE 10		// Anzahl der Bytes, die im Flash reserviert werden sollen

// UDP-Parameter
WiFiUDP udp;
const int localUdpPort = 1902; // Port, auf dem der ESP32 hört
char incomingPacket[1500];      // Puffer für eingehende Nachrichten
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

float GasDiff;
float WasserDiff;
float BrauchwasserDiff;
float GasHeute;
float WasserHeute;
float BrauchwasserHeute;

WiFiUDP ntpUDP;
WiFiServer server(6222);                   // Default Virtuino Server port 
VirtuinoCM virtuino;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
unsigned long epochTime;
String Zeitstring;
String Startzeit;
int Neustarts;

bool debug = false;
unsigned long Sendezeit = millis();
const int Sendepause = 3000;  //Sendepause in Sekunden zur Datenuebertragungin msec


void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

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

    AktuellerZaehlerstand(); //Gas und Wasser
	StandGesternVonRaspberryLesen();

  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL); //add current thread to WDT watch
}

void loop() {

  virtuinoRun();        // Necessary function to communicate with Virtuino. Client handler
  if (millis() > Sendezeit + Sendepause)  //wenn die Zeit sich ge�ndert hat
  {
	  Sendezeit = millis();
	  DatenAbholen();
	  if (GasGestern < 9000 || WasserGestern < 175 || BrauchwasserGestern < 1900)  StandGesternVonRaspberryLesen();

	  SendeNeustarts();
	  AktuellerZaehlerstand(); //Gas und Wasser, Brauchwasser
	/*
	  Serial.print(Byts[122]);
	  Serial.print(" - ");
	  Serial.println(Byts[123]);
	  Serial.print(Byts[130]);
	  Serial.print(" - ");
	  Serial.println(Byts[131]);
		
	  Serial.print("Byts[339]");
	  Serial.print(" - ");
	  Serial.print(Byts[339]);
	  Serial.print(" - ");
	  Serial.print(Byts[340]);
	  Serial.print(" - ");
	  Serial.print(Byts[341]);
	  Serial.print(" - ");
	  Serial.print(Byts[342]);
	  Serial.print(" - ");
	  Serial.println(Stromwert(341));
	  */
	  Serial.println();
 }

  VituinoAbfragen();
  PCBefehlAbfragen();

  esp_task_wdt_reset(); //Reset watchdog

  if (epochTime < 1000)  ZeitSetzen(); // ZeitPruefen
}
