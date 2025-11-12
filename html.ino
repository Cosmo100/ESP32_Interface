// Port 80: einfacher Webserver
String Bezeichnung[] = {"Solarkollektor","Warmwasser","Au&szlig;entemperatur","Kesseltemperatur","Temp Wohnzimmer","Temp Schlafzimmer",
						"Temp Bad1","Temp Bad2","Temp Arb.Zimmer","Poolkollektor","Poolwasser Saugseite","Poolwasser Druckseite",
						"Vorlauf Heizung Pool","R&uuml;cklauf Heizung Pool","Einstrahlung","Flterdruck Pool","Pumpendruck","Wasserdruck",
						"Niveau Pool","Niveau 1","Niveau 2","Niveau 3","Delta Niveau1","Delta Niveau2","Delta Niveau3","Delta Niveau Pool",
						"&Ouml;ffnung Garagenladen","Aktuelle PV-Leistung1","Aktuelle PV-Leistung2","Aktuelle Leistung PV2, Mod1","Aktuelle Leistung PV2, Mod2",
						"Summe PV1+PV2","Anzeige Bits der LEDs","Anzeige Bits der LEDs","Zust&auml;nde Pool","Gesamte PV-Leistung aus Shelly",
						"Stand Gasz&auml;hler heute","Gasverbrauch seit gestern","Stand Wasserz&auml;hler heut","Wasserverbrauch seit gestern",
						"Stand Brauchwasserz&auml;hler heut","Brauchwasser seit gestern","Heizung Leistung","Waschen Leistung",
						"B&uuml;ro Leistung","K&uuml;che Leistung","Sp&uuml;len/Gefrieren Leistung","K&uuml;hlschrank Leistung","Herd rechts Leistung",
						"Herd links Leistung","Backofen Leistung","Wohnen Leistung","Kellerrechner Leistung","Bad 1 Leistung",
						"Garage Leistung","Pool Leistung A","Pool Leistung B","Pool Leistung C","PV2 Leistung","Bad2/Zimmer vorne Leistung",
						"GesamtPV","aktueller Gesamtverbrauch","Stromverbrauch seit gestern","Eingespeist seit gestern","K&uuml;hlschrank Keller Leistung",
						"Smartmeter Phase A","Smartmeter Phase B","Smartmeter Phase C","AllePhasen Wirkleitung","AllePhasen Scheinleitung"
						};

String Einheit[] = {"&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","&deg;C","lux","mbar","bar","bar","mm","l","l",
					"l","l","l","l","l","%","W","W","W","W","W","","","","W","m&sup3;","l","m&sup3;","l","m&sup3;","W","W",
					"W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W","W"
					};
void HTMLServer()
{
WiFiClient client = server80.available();
if (client) {
	Serial.println("Neuer Client");
	String request = "";
	while (client.connected()) {
		if (client.available()) {
			char c = client.read();
			request += c;
			if (c == '\n') break;
		}
	}

	// HTML-Antwort
	client.println(HTML());
	client.println();
	delay(1);
	client.stop();
}
}

//==================================================================================================
String HTML()
{
	
	String html = "<html><head>";
	html += "<meta http-equiv='refresh' content='15'></head>";
	html += "<style>";
	html += "body{font-family: verdana, arial; background-color: #4A6D84; padding-top: 20px}";
	html += ".Zeit{background: #FFFFCC;font-weight: 350;font-size:120% ;font-style: italic;text-align : right;color: #0033CC; border: 10px ridge #39F}";

	html += "th{background: #FFFFCC;font-weight: bold;font-size:160% ;font-style: italic;text-align : center; border: 10px ridge #39F;}";
	html += "TABLE.Daten{background: #FFFFCC; border: 10px ridge #39F;}";
	html += "TD.Dat0{border: 1px solid rgb(200,200,200); font-weight:bold; width: 50px;text-align:right;vertical-align: middle;}";
	html += "TD.Dat1{border: 1px solid rgb(200,200,200); font-weight:bold; width: 400px;text-align:right;vertical-align: middle;}";
	html += "TD.Dat2{border: 1px solid rgb(200,200,200); font-weight:bold; width: 200px;text-align:right;vertical-align: middle;}";
	html += "</style>";

	html += "</head><body><div align='center'>";
	html += "<table width = '500px' ><tr><th>Antwort vom Interface</th></tr>";

	html += "<tr><td width = '350px' align='left' valign='top'>";
	html += "<table class='Zeit'><tr><td width = '350px' >Aktuelle Zeit : </td><td width = '150px'>" + AktuelleZeit() + "</td></tr>";
	html += "<tr><td width = '350px' >Aktuelle Datum : </td><td width = '150px'>" + AktuellesDatum() + "</td></tr>";
	html += "</table>";

	html += "<table class='Daten'>";
	for (int i = 0; i < 70; i++)
		{
		html += "<tr>";
		html += "<td class = 'Dat0'>" + String(i) + "</td>";
		html += "<td class = 'Dat1'>" + Bezeichnung[i] + "</td>";
		html += "<td class = 'Dat2'>" +  String(V[i],1) + " " + Einheit[i]+"</td>";
		//html += "<td class = 'Dat2'>" + String(S[i],2) + " kWh</td>";
		html += "</tr>";
		}

	html += "</table></table>";
	html += "</div></body></html>";

	return html;
	
}

