#include <SoftwareSerial.h>
#include <WiFi.h>

SoftwareSerial mySerial(16, 17);  // RX, TX

char ssid[] = "Sabbir";
char password[] = "19122918";

const char* server = "api.thingspeak.com";
const String apiKey = "YourThingSpeakAPIKey";

int DE = 2;
int RE = 4;
int Moisture;
int relay_pin = 5;

WiFiClient client;

void setup() {
  Serial.begin(9600);
  mySerial.begin(4800);
  pinMode(DE, OUTPUT);
  pinMode(RE, OUTPUT);
  pinMode(relay_pin, OUTPUT);
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  // Connect to WiFi
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void loop() {
  byte queryData[]{ 0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08 };
  byte receivedData[19];
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);

  mySerial.write(queryData, sizeof(queryData));  // send query data to NPK

  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  delay(1000);

  if (mySerial.available() >= sizeof(receivedData)) {        // Check if there are enough bytes available to read
    mySerial.readBytes(receivedData, sizeof(receivedData));  // Read the received data into the receivedData array
    processReceivedData(receivedData);
  }
}

void processReceivedData(byte receivedData[]) {
  unsigned int soilHumidity = (receivedData[3] << 8) | receivedData[4];
  unsigned int soilTemperature = (receivedData[5] << 8) | receivedData[6];
  unsigned int soilConductivity = (receivedData[7] << 8) | receivedData[8];
  unsigned int soilPH = (receivedData[9] << 8) | receivedData[10];
  unsigned int nitrogen = (receivedData[11] << 8) | receivedData[12];
  unsigned int phosphorus = (receivedData[13] << 8) | receivedData[14];
  unsigned int potassium = (receivedData[15] << 8) | receivedData[16];

  Serial.print("Soil Humidity: ");
  Serial.println((float)soilHumidity / 10.0);
  Moisture = ((float)soilHumidity / 10.0);
  Serial.print("Soil Temperature: ");
  Serial.println((float)soilTemperature / 10.0);
  Serial.print("Soil Conductivity: ");
  Serial.println(soilConductivity);
  Serial.print("Soil pH: ");
  Serial.println((float)soilPH / 10.0);
  Serial.print("Nitrogen: ");
  Serial.println(nitrogen);
  Serial.print("Phosphorus: ");
  Serial.println(phosphorus);
  Serial.print("Potassium: ");
  Serial.println(potassium);
  Serial.println("\n\n");

  controlRelay();
  sendToThingSpeak(soilHumidity, soilTemperature, soilConductivity, soilPH, nitrogen, phosphorus, potassium);
}

void controlRelay() {
  if (Moisture > 25) {
    Serial.println("Soil is wet");
    digitalWrite(relay_pin, HIGH);
  } else {
    Serial.println("Soil is DRY");
    digitalWrite(relay_pin, LOW);
  }
}

void sendToThingSpeak(unsigned int soilHumidity, unsigned int soilTemperature, unsigned int soilConductivity, unsigned int soilPH, unsigned int nitrogen, unsigned int phosphorus, unsigned int potassium) {
  if (client.connect(server, 80)) {
    String postData = "api_key=" + apiKey;
    postData += "&field1=" + String((float)soilHumidity / 10.0);
    postData += "&field2=" + String((float)soilTemperature / 10.0);
    postData += "&field3=" + String(soilConductivity);
    postData += "&field4=" + String((float)soilPH / 10.0);
    postData += "&field5=" + String(nitrogen);
    postData += "&field6=" + String(phosphorus);
    postData += "&field7=" + String(potassium);

    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.print(postData);
    Serial.println("Data sent to ThingSpeak");
  } else {
    Serial.println("Connection to ThingSpeak failed");
  }
  delay(1000);
}
