#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#define PRESSAO_NIVEL_DO_MAR (1013.25)

Adafruit_BME280 bme;

float temperatura, umidade, pressao, altitude;

const char* SSID = "<YOUR SSID HERE>";
const char* PASSWORD = "<YOUR PASSWORD HERE>";

const char* BROKER_MQTT = "<YOUR BROKER'S IP HERE>";
int BROKER_PORT = 1883;

const int wakeTimeInSeconds = 25200; //07:00:00
const int sleepTimeInSeconds = 68400; //19:00:00
int lastStatus = 0;

const long utcOffsetInSeconds = -10800;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void initWifi();
void initMQTT();

WiFiClient espClient;
PubSubClient MQTT(espClient);

void setup() {
  pinMode(0, OUTPUT);
  digitalWrite(0, LOW);
  
  Serial.begin(115200);
  initWifi();
  initMQTT();
  bme.begin(0x76);
  timeClient.begin();
}

void loop() {
  timeClient.update();

  int totalSeconds = (timeClient.getHours() * 3600) + (timeClient.getMinutes() * 60) + timeClient.getSeconds();
 
  if(totalSeconds > wakeTimeInSeconds && totalSeconds < sleepTimeInSeconds){
    if(lastStatus != HIGH) digitalWrite(0, HIGH);
    lastStatus = HIGH;
  } else {
    if(lastStatus != LOW) digitalWrite(0, LOW);
    lastStatus = LOW;
  }
  
  temperatura = bme.readTemperature();
  umidade = bme.readHumidity();
  pressao = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(PRESSAO_NIVEL_DO_MAR);

  if(!MQTT.connected()){
    MQTT.connect("ESP8266-BM280");
    Serial.println("MQTT Conectado!");
  }
  MQTT.loop();

  const int objSize = JSON_OBJECT_SIZE(4);
  StaticJsonDocument<objSize> post;
  post["temperatura"] = temperatura;
  post["umidade"] = umidade;
  post["pressao"] = pressao;
  post["altitude"] = altitude;
  
  char buffer[512];
  serializeJson(post, buffer);
  MQTT.publish("bme280", buffer);
  
  delay(1000);
}

void initWifi() {
  delay(10);
  Serial.println("Conectando-se em: " + String(SSID));

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado na Rede " + String(SSID) + " | IP => ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}
