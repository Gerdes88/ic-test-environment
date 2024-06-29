#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

#define ONE_WIRE_BUS 13
#define ESP8266_RX 2
#define ESP8266_TX 3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
SoftwareSerial esp8266(ESP8266_RX, ESP8266_TX); // RX, TX

void setup() {
  esp8266.begin(9600);  // Start seriel kommunikation med computeren
  esp8266.begin(9600);    // Start seriel kommunikation med ESP8266 (ændret til 9600)
  sensors.begin();        // Start Dallas Temperature sensor

  Serial.println("ESP8266 AT Kommando Test og Temperatur Måling");

  // Initialiser ESP8266
  initESP8266();
}

void loop() {
  // Temperatur måling
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  if (temperatureC == DEVICE_DISCONNECTED_C) {
    Serial.println("Fejl: Kunne ikke læse temperaturdata");
    delay(1000);
    return;
  }

  float temperatureF = sensors.toFahrenheit(temperatureC);

  Serial.print("Temperatur: ");
  Serial.print(temperatureC);
  Serial.print(" C eller ");
  Serial.print(temperatureF);
  Serial.println(" F");

  // Send temperatur data til ESP8266
  sendTemperatureData(temperatureC);

  // Håndter ESP8266 kommunikation
  handleESP8266Communication();

  delay(2000);  // Vent 2 sekunder før næste måling
}

void initESP8266() {
  if (!sendCommandWithCheck("AT", "OK")) {
    Serial.println("ESP8266 ikke fundet. Kontroller forbindelser og baud rate.");
    while(1);  // Stop programmet hvis ESP8266 ikke svarer
  }
  //delay(5000);
  sendCommand("AT");
  sendCommand("AT+RST");
  delay(1000);
  sendCommand("AT+CWMODE=1");  // Sæt ESP8266 i station mode
  sendCommand("AT+CWJAP=\"Martins24\",\"12345678\"");  // Forbind til WiFi
  delay(5000);  // Vent på WiFi forbindelse
  sendCommand("AT+CIFSR");  // Få IP-adresse
  sendCommand("AT+CIPMUX=1");  // Konfigurer for multiple forbindelser
  sendCommand("AT+CIPSERVER=1,80");  // Start server på port 80
  if (!sendCommandWithCheck("AT+CWJAP=\"Martins24\",\"12345678\"", "OK")) {
    Serial.println("Kunne ikke forbinde til WiFi. Kontroller SSID og password.");
    while(1);
  }
  Serial.println("Forbundet til WiFi");
}

bool sendCommandWithCheck(String command, String expectedResponse) {
  sendCommand(command);
  return esp8266.find(expectedResponse.c_str());
}

void sendTemperatureData(float temperature) {
  String data = String(temperature, 2);  // Konverter float til string med 2 decimaler
  String command = "AT+CIPSEND=0," + String(data.length()) + "\r\n";
  sendCommand(command);
  delay(100);
  esp8266.print(data);
}

void handleESP8266Communication() {
  while (esp8266.available()) {
    Serial.write(esp8266.read());
  }
  
  while (Serial.available()) {
    esp8266.write(Serial.read());
  }
}

void sendCommand(String command) {
  esp8266.flush();
  while (esp8266.available()) {
    esp8266.read();
  }
  esp8266.println(command);
  delay(1000);
  while (esp8266.available()) {
    char c = esp8266.read();
    if (c == '\r' || c == '\n') {
      Serial.println();
    } else {
      Serial.write(c);
    }
  }
  Serial.println();
}