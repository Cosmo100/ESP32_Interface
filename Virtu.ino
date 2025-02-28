

//============================================================== onCommandReceived
//==============================================================
/* This function is called every time Virtuino app sends a request to server to change a Pin value
 * The 'variableType' can be a character like V, T, O  V=Virtual pin  T=Text Pin    O=PWM Pin
 * The 'variableIndex' is the pin number index of Virtuino app
 * The 'valueAsText' is the value that has sent from the app   */
void onReceived(char variableType, uint8_t variableIndex, String valueAsText) {
	if (variableType == 'V') {
		float value = valueAsText.toFloat();        // convert the value to float. The valueAsText have to be numerical
		if (variableIndex < V_memory_count) V[variableIndex] = value;              // copy the received value to arduino V memory array
	}
	else  if (variableType == 'T') {
		if (variableIndex == 0) Text_0 = valueAsText;          // Store the text to the text variable Text_0
		else if (variableIndex == 1) Text_1 = valueAsText;     // Store the text to the text variable Text_1
		//else if (variableIndex == 2) Text_2 = valueAsText;     // Store the text to the text variable T2
		//else if (variableIndex==3) T3=valueAsText;     // Store the text to the text variable T3

	}
}

//==============================================================
/* This function is called every time Virtuino app requests to read a pin value*/
String onRequested(char variableType, uint8_t variableIndex) {
	if (variableType == 'V') {
		if (variableIndex < V_memory_count) return  String(V[variableIndex]);   // return the value of the arduino V memory array

	}
	else if (variableType == 'T') {
		if (variableIndex == 0) return Text_0;
		else if (variableIndex == 1) return Text_1;
		//else if (variableIndex == 2) return Text_2;
		//else if (variableIndex==3) return T3;   
	}

	return "";
}

//==============================================================
void virtuinoRun() {
	WiFiClient client = server.available();
	if (!client) return;
	if (debug) Serial.println("Connected");
	unsigned long timeout = millis() + 3000;
	while (!client.available() && millis() < timeout) delay(1);
	if (millis() > timeout) {
		Serial.println("timeout");
		client.flush();
		client.stop();
		return;
	}
	virtuino.readBuffer = "";    // clear Virtuino input buffer. The inputBuffer stores the incoming characters
	while (client.available() > 0) {
		char c = client.read();         // read the incoming data
		virtuino.readBuffer += c;         // add the incoming character to Virtuino input buffer
		if (debug) Serial.write(c);
	}
	client.flush();
	if (debug) Serial.println("\nReceived data: " + virtuino.readBuffer);
	String* response = virtuino.getResponse();    // get the text that has to be sent to Virtuino as reply. The library will check the inptuBuffer and it will create the response text
	if (debug) Serial.println("Response : " + *response);
	client.print(*response);
	client.flush();
	delay(10);
	client.stop();
	if (debug) Serial.println("Disconnected");
}


//============================================================== vDelay
void vDelay(int delayInMillis)
{
	long t = millis() + delayInMillis; while (millis() < t) virtuinoRun();
}
