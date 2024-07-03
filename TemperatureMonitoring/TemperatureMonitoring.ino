#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

#define ONE_WIRE_BUS 13  // DS18B20 data wire er tilsluttet til digital pin 2
#define ESP8266_RX 3
#define ESP8266_TX 2
#define ESP32_RX 0
#define ESP32_RX 1

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
SoftwareSerial esp8266(ESP8266_RX, ESP8266_TX);
SoftwareSerial espSerial(ESP32_RX, ESP32_RX);

//const char* SSID = "MeyerGerdesNet";
//const char* PASSWORD = "3jbyd4lsv3j172+2600";
const char* SSID = "Martins24";
const char* PASSWORD = "12345678";
const char* API_KEY = "H3C2WB9XWVSR0BE8";

void setup() {
  Serial.begin(115200);
  esp8266.begin(115200);
  espSerial.begin(115200);
  sensors.begin();
  connectWiFi();
}

void loop() {
  //communication with ESP32
  if (Serial.available()) {
    espSerial.write(Serial.read());
  }
  if (espSerial.available()) {
    Serial.write(espSerial.read());
  }

  //read temperature
   float temp = getTemperature();

  //send temperature to ESP8266
  if (sendToThingSpeak(temp)) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Failed to send data");
  }

  //ThingSpeak only allows one upload per 15 seconds
  delay(20000); 
}

float getTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.println(" C");
  return tempC;
}

void connectWiFi() {
  int wifiConnectRetries=20;
  Serial.println("Connecting to WiFi...");
  //Serial.println("List of available wifi: "+esp8266.println("AT+CWLAP"));
  esp8266.println("AT+RST");
  delay(2000);
  esp8266.println("AT+CWMODE=1");
  delay(5000);
  String cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASSWORD;
  cmd += "\"";
  
  while (wifiConnectRetries>0)
  {
    esp8266.println(cmd);
    delay(2000);
    if (esp8266.find("OK")) {
      Serial.println("Connected to WiFi with this cmd: "+cmd);
      wifiConnectRetries = 0;
    } else {
      Serial.println("Failed to connect to WiFi, attempting again with: "+ String(wifiConnectRetries) + " wifiConnectRetries left, using this cmd: "+cmd);
      wifiConnectRetries = wifiConnectRetries-1;
    }
  } 
}

bool sendToThingSpeak(float temp) {
  String cmd = "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80";
  esp8266.println(cmd);
  delay(2000);
  
  String getData = "GET /update?api_key=";
  getData += API_KEY;
  getData += "&field1=";
  getData += String(temp);
  getData += " HTTP/1.1\r\n";
  getData += "Host: api.thingspeak.com\r\n\r\n";
  
  cmd = "AT+CIPSEND=";
  cmd += String(getData.length());
  esp8266.println(cmd);
  Serial.println("Sending API data: "+ getData);
  delay(1000);
  
  esp8266.print(getData);
  delay(1000);
  
  esp8266.println("AT+CIPCLOSE");
  return true;
}

void clearSerialBuffer() {
  while (esp8266.available()) {
    esp8266.read();
  }
}