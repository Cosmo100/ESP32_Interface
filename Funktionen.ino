
// Funktion zum Parsen des Byte-Strings
void parseByteString(const char *input, uint8_t *output, size_t *output_len) {
	char *token;
	char *input_copy = strdup(input); // Kopie des Strings erstellen
	size_t index = 0;

	token = strtok(input_copy, ", "); // Zerlege den String an Kommas und Leerzeichen
	while (token != NULL) {
		output[index++] = (uint8_t)atoi(token); // Konvertiere zu uint8_t
		token = strtok(NULL, ", ");
	}

	*output_len = index; // Speichere die Länge des Bytearrays
	free(input_copy);    // Speicher freigeben
}
//==============================================================
void DatenAbholen()
{	//Daten werden vom Raspberry bei jedem Durchgang gesendet
	int packetSize = udp.parsePacket(); // Prüfen, ob ein Paket empfangen wurde

	if (packetSize) {
		Serial.print("Paketgroesse[");
		Serial.print(packetSize);
		Serial.println("]");

		int len = udp.read(packetBuffer, 1500);

		// Paket ausgeben
		//Serial.print("Empfangenes Paket: ");
		//Serial.println(packetBuffer);

		// Paket in Bytearray umwandeln
		parseByteString(packetBuffer, Byts, &BytsLen);

		// Bytearray ausgeben
		/*
		Serial.print("Bytearray: ");
		for (size_t i = 0; i < BytsLen; i++) {
			Serial.print(Byts[i]);
			Serial.print(" ");
		}
		*/
	
		Serial.print("Anz Bytes[");
		Serial.print(BytsLen);
		Serial.println("]");
	
	}
}

//==============================================================
float ZweiByteStrom(int ST)
{
	int16_t result = (int16_t(Byts[ST]) << 8) | Byts[ST + 1];
	return (float) result / 10;
	}

//###############################################################################################
void VituinoAbfragen()
{
	static unsigned long AltLadenbefehl;
	static unsigned long AltBefehl;

	//Werte zu Android-Virtuino schicken
	V[0] = Wert(0);		//Solarkollektor
	V[1] = Wert(4);		//Warmwasser
	V[2] = Wert(101);	//Aussentemp
	V[3] = Wert(116);	//Kessel
	V[4] = Wert(45);  //Temp Wohnzimmer
	V[5] = Wert(47);  //Temp Schlafzimmer
	V[6] = Wert(49);  //Temp Bad1
	V[7] = Wert(51);  //Temp Bad2
	V[8] = Wert(10); //Temp Arbeitszimmer
	V[9] = Wert(196); //Temp Poolkollektor
	V[10] = Wert(198); //Poolwasser Saugseite
	V[11] = Wert(200); //Poolwasser Druckseite	
	V[12] = Wert(202);  //Vorlauftemp Heizung Pool
	V[13] = Wert(204);   //Ruecklauftemp Heizung Pool

	V[14] = Wert(18) * 10; //Einstrahlung
	V[15] = 10 * Wert(192); //Druck Filter
	V[16] = Wert(160) / 10; //Druck Tauchpumpe
	V[17] = Wert(162) / 10; //Wassserdruck
	V[18] = Wert(184) * 10; //Niveau Pool in mm
	V[19] = 10 * Wert(186); //Niveau Becken 1
	V[20] = 10 * Wert(188); //Niveau Becken 2
	V[21] = 10 * Wert(190); //Niveau Becken 3

	//Aenderung der Niveaus
	V[22] = V[19] - NiveauAlt1;
	V[23] = V[20] - NiveauAlt2;
	V[24] = V[21] - NiveauAlt3;
	V[25] = V[18] - NiveauAltPool;

	//V[26] = 10 * Wert(182); //Öffnung Garagenladen
	//V[27] = Wert(122) * 10; //Aktuelle PV-Leistung PV1
	//V[28] = Wert(130) * 10; //Aktuelle PV-Leistung PV2
	//V[29] = Wert(304) * 10; //Aktuelle Leistung PV2; Modul1
	//V[30] = Wert(310) * 10; //Aktuelle Leistung PV2; Modul2
	//V[31] = V[27] + V[28];

	int K = Byts[175];	//Behälter und Strömungswächter
	bitWrite(K, 3, bitRead(Byts[177], 7));//Melder Behälter3 leer
	V[32] = K;

	//Einzelbits für Virtuino zusammenstellen (siehe Tabelle Variablen)
	K = Byts[179]; //Relais Tauchpumpe 1u.2, Zusatzpumpe Heizkörper Arbeitszimmer (Bit3)
	//bitWrite(K, 2, xxx);  Relais 3 ist noch frei!
	bitWrite(K, 4, Byts[111]); //Heizkreispumpe
	bitWrite(K, 5, Byts[1118]); //Speicherladepumpe
	bitWrite(K, 6, bitRead(Byts[179], 6)); //Rückmeldung Garage
	bitWrite(K, 7, bitRead(Byts[179], 7)); //Rückmeldung Keller, kommt von Raspberry --> überprüfen
	V[33] = K;

	K = Byts[178];  					   //Filterpumpe, Heizventil, Heizpumpe Pool, Tauchpumpe 3
	bitWrite(K, 4, bitRead(Byts[176], 6)); //Lese Zustand des Bewässerungsventils
	V[34] = K;  				

	V[36] = GasHeute;
	V[37] = GasDiff;
	V[38] = WasserHeute;
	V[39] = WasserDiff;
	V[40] = BrauchwasserHeute;
	V[41] = BrauchwasserDiff;

	//Strom
	for (int i = 0; i < 18; i++)
	{
		V[i + 42] = ZweiByteStrom(i * 2 + 238);  //V[41] bis V[59]
	}
	V[60] = V[56] + V[58];	//Pool Leistung B + PV2Leistung
	V[61] = -(V[60]) + Stromwert(341);	//PV-Leistung gesamt + aktuelle Leistung Hauptstromzähler
	V[62] = StromDiff;
	V[63] = EingespeistDiff;
	V[64] = ZweiByteStrom(274);
	V[79] = Neustarts;

	/*
	Serial.print("Byts[128]");
	Serial.print(" - ");
	Serial.print(Byts[128]);
	Serial.print(" - ");
	Serial.println(Byts[129]);
	Serial.print(" - ");
	Serial.println(Wert(128));
	*/

	BefehlVonAndroid();  // Achtung: V[80] wird als Empfang des Befehls genutzt

	if (NiveauAlt1==0)  //ErsteVirtu muss jeden Tag auf true gesetzt werden
		{
		NiveauAltPool = V[18];
		NiveauAlt1 = V[19];
		NiveauAlt2 = V[20];
		NiveauAlt3 = V[21];
		}


//	if (hour() == 0 && minute() == 0)  NiveauAlt1 = 0;  //Um 0.00 Uhr AltNiveaus zurücksetzen
}
//###############################################################################################
void BefehlVonAndroid()
{	//Bei jedem Durchlauf wird Überprüft, ob ein Befehl von Android vorliegt
	unsigned int Befehl = V[80];	//Befehl von Android
	switch (Befehl)
	{
	case 1:
		//Text_2 = "Neustart";
		Serial.print("Befehl von Android: Reset ESP Interface");
		Reset();
		break;
	case 2:
		
		LoescheNeustarts();
		break;
	}
}

//*********************************************************
float Wert(int BytNr)
{
	float WE = word(Byts[BytNr + 1], Byts[BytNr]);

	if (Byts[BytNr + 1] > 128) WE = WE - pow(2, 16);

	WE /= 10;

	return WE;
}
//*********************************************************
float Stromwert(int BytNr)
{
	float WE = word(Byts[BytNr], Byts[BytNr + 1]);
	if (Byts[BytNr ] > 128) WE = WE - pow(2, 16);
	return WE;
}
//*********************************************************
void StandGesternVonRaspberryLesen()
{
	Serial.println("Lese Stand gestern");
	//Liest Datei von Raspberry aus mnt/ramdisk
	const char*	fileUrl = "http://heisopi:8080/Zaehlergestern.heis";
	HTTPClient http;
	http.begin(fileUrl);

	int httpCode = http.GET();
	if (httpCode > 0) {
		String Inhalt = http.getString();

		Serial.println(Inhalt);
		GasGestern = SelektiereWert(Inhalt, "ZSGas=");
		WasserGestern = SelektiereWert(Inhalt, "ZSWasser=");
		BrauchwasserGestern = SelektiereWert(Inhalt, "ZSBrauchwasser=");
		StromGestern = SelektiereWert(Inhalt, "ZSHauptZ=");
		EingespeistGestern = SelektiereWert(Inhalt, "ZSEingesp=");
		Serial.print("GasGestern = ");
		Serial.println(GasGestern);
		Serial.print("WasserGestern = ");
		Serial.println(WasserGestern);
		Serial.print("BrauchwasserGestern = ");
		Serial.println(BrauchwasserGestern);
		Serial.print("StromGestern = ");
		Serial.println(StromGestern);
		Serial.print("EingespeistGestern = ");
		Serial.println(EingespeistGestern);
	}
	else {
		Serial.println("Fehler beim Abrufen der Datei");
	}
	http.end();
}

//*********************************************************
float StandHeute(int ST)
{
	uint32_t result = (uint32_t(Byts[ST+2]) << 16) | (uint32_t(Byts[ST + 1]) << 8) | Byts[ST];

	return (float) result / 1000;
}
//*********************************************************

float  SelektiereWert(String Inhalt, String Wert)
{
	String Erg;
	int Laenge = Wert.length();
	int Pos = Inhalt.indexOf(Wert) + Laenge;		//Wert suchen
	int Endpos = Inhalt.indexOf("\n", Pos);

	/*Serial.print("Pos = ");
	Serial.println(Pos);
	Serial.print("Endpos = ");
	Serial.println(Endpos);*/

	Erg = Inhalt.substring(Pos, Endpos);
	float floatValue = atof(Erg.c_str());

	return floatValue;
}
//==================================================================================================
void AktuellerZaehlerstand()
{	//Ruft aktuellen Zaehlerstand Gas, Wasser, Brauchwasser, Strom vom Raspberry  ab
	
	if (Byts[298] == 0 && Byts[299] == 0) return;
		
	GasHeute = StandHeute(298);
	WasserHeute=StandHeute(301);
	BrauchwasserHeute=StandHeute(295);
	StromHeute = StandStrom(331);
	EingespeistHeute= StandStrom(335);

	GasDiff = (GasHeute - GasGestern) * 1000;
	WasserDiff = (WasserHeute - WasserGestern) * 1000;
	BrauchwasserDiff = (BrauchwasserHeute - BrauchwasserGestern) * 1000;
	StromDiff = (StromHeute -StromGestern) * 1000;
	EingespeistDiff = (EingespeistHeute - EingespeistGestern) * 1000;
	
/*	
	Serial.print("StromHeute = ");
	Serial.println(StromHeute);
	Serial.print("StromGestern = ");
	Serial.println(StromGestern);
	Serial.print("StromDiff = ");
	Serial.println(StromDiff);

	Serial.print("Strom[");
	Serial.print(String(Neustarts));
	Serial.println("]");

	Serial.print("GasHeute = ");
	Serial.println(GasHeute);
	Serial.print("WasserHeute = ");
	Serial.println(WasserHeute);
	Serial.print("BrauchwasserHeute = ");
	Serial.println(BrauchwasserHeute);
	
	
	Serial.print("GasDiff = ");
	Serial.println(GasDiff);
	Serial.print("WasserDiff = ");
	Serial.println(WasserDiff);*/
	
}

//********************************************************************
float StandStrom (int BytNr)
{
uint32_t zahl = (uint32_t)Byts[BytNr+3] |
((uint32_t)Byts[BytNr+2] << 8) |
((uint32_t)Byts[BytNr+1] << 16) |
((uint32_t)Byts[BytNr] << 24);

// Division durch 10000
float Erg = zahl / 10000.0;

return Erg;

}

//********************************************************************
void SendeWert(String Bez,String Wert)
{
	String Sende = Bez + "[" + Wert + "]";
	Serial.println(Sende);
}

//###############################################################################################
//####################### Befehl von PC lesen ###################################################
//###############################################################################################
void PCBefehlAbfragen()
{ //serialEvent ist bei ESP32 nicht möglich

	String Bef;

	/*siehe
	http://startingelectronics.org/software/arduino/learn-to-program-course/18-strings/
	*/

	if (Serial.available() > 1) {
		Bef = Serial.readStringUntil(13);
		if (Bef.length() < 20) Serial.println(Bef);
	}
	else return;

	if (Bef == "SendeDaten")
	{


	}

	else if (Bef == "Reset") //Reset des Kellerrechners
	{
		//Text_2 = "Neustart";
		Reset();
	}
	else if (Bef == "CLRNeustart") //Löschen der Neustarts im Eprom des ESP
	{
		LoescheNeustarts();
	}

	else
	{
		Serial.println("kein bekannter Befehl...");
		Serial.println(Bef);
	}
}
//==============================================================
void LoescheNeustarts()
{
	Serial.println("Loesche Anzahl Neustart");

	EEPROM.write(0, 0); //Setze Adresse 0 auf 0 Neustarts
	EEPROM.commit(); // Damit die Daten tatsächlich in den Flash geschrieben werden
	Neustarts = EEPROM.read(0); //Lese neue Anzahl von Neustarts Neustarts
}
//==============================================================
void ZeitSetzen()
{
	Serial.println("Hole Zeit");
	configTime(0, 0, "pool.ntp.org");
	setenv("TZ", TZ_INFO, 1);
	tzset();
	AktuelleZeit();
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
String AktuelleZeit()
{
	char Zeit[9];  // "hh:mm:ss\0" -> 9 Zeichen
		
	if (getLocalTime(&timeinfo))
	{
		strftime(Zeit, sizeof(Zeit), "%H:%M:%S", &timeinfo);
		Text_0 = Zeit;  //ohne Klammern an Virtuino übergeben

		//Um 01:01 Stand von gestern neu lesen
		if (timeinfo.tm_hour == 1 && timeinfo.tm_min < 5)  StandGesternVonRaspberryLesen();

		return Zeit;
	}
	else {
		Serial.println("Fehler beim Abrufen der Zeit");
		return "00";
	}

}
//==============================================================
String AktuellesDatum()
{
	char Datum[11];  // Format: "TT.MM.JJJJ" -> 10 Zeichen + Nullterminator
	if (getLocalTime(&timeinfo)) {
		// Datum als String formatieren

		strftime(Datum, sizeof(Datum), "%d.%m.%Y", &timeinfo);
		Text_1 = Datum;  //ohne Klammern an Virtuino übergeben
		return Datum;
	}
}

//==============================================================
