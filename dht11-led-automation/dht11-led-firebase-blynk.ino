#define BLYNK_TEMPLATE_ID "XXXXX"
#define BLYNK_DEVICE_NAME "XXXXX"
#define BLYNK_AUTH_TOKEN "XXXXX"

#define FIREBASE_HOST "XXXXX"
#define FIREBASE_AUTH "XXXXX"

#define WIFI_SSID "XXXXX"
#define WIFI_PASSWORD "XXXXX"

#define DHTPIN D6
#define redLed D5

#define DHTTYPE DHT11

#include "FirebaseESP8266.h"

#include <ESP8266WiFi.h>
#include <DHT.h>

#include <BlynkSimpleEsp8266.h>

FirebaseData firebaseData;
FirebaseJson json;

DHT dht(DHTPIN, DHTTYPE);

int h;
float t;
float hTemp;
float tTemp;
bool dhtStatus = false;
bool ledStatus = false;
int secSS, minMM, hrsHH = 0;

unsigned long theTime;
long beforeTime;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

void setup() {
  Serial.begin(9600);

  dht.begin();
  pinMode(redLed, OUTPUT);
  
  Blynk.begin(auth, ssid, pass);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void sensorUpdate() { 
  h = dht.readHumidity();
  t = dht.readTemperature();

  Serial.printf("Hum  = %d%%", h);
  Serial.printf("\nTem  = %.1f°C", t);

  // Check if the temperature higher than 30°C then turn on the LED
  if (t >= 30) {
    digitalWrite(redLed, LOW);
    if (ledStatus == true) {
      ledStatus = false;
      Blynk.virtualWrite(V1, 1);
      Serial.printf("\nBlynk redLed turned on.");
    }
  } else {
    digitalWrite(redLed, HIGH);
    if (ledStatus == false) {
      ledStatus = true;
      Blynk.virtualWrite(V1, 0);
      Serial.printf("\nBlynk redLed turned off.");
    }
  }
  
  float f = dht.readTemperature(false);
  if (isnan(h) || isnan(t) || isnan(f))
  {
    if (dhtStatus == true) {
      dhtStatus = false;
      Blynk.virtualWrite(V0, 0);
      Serial.printf("\nBlynk dhtStatus failed.");
    }
    return;
  } else {
    if (dhtStatus == false) {
      dhtStatus = true;
      Blynk.virtualWrite(V0, 1);
      Serial.printf("\nBlynk dhtStatus reading.");
    }
  }

  // If the data updated, send to Blynk
  if (hTemp != h) {
    Blynk.virtualWrite(V5, h);
    Serial.printf("\nBlynk Hum updated.");
  }
  if (tTemp != t) {
    Blynk.virtualWrite(V4, t);
    Serial.printf("\nBlynk Tem updated.");
  }
  hTemp = h;
  tTemp = t;
}

void saveDataToFirebase() {
  Firebase.pushFloat(firebaseData, "/Logs/humidity", dht.readHumidity());
  Firebase.pushFloat(firebaseData, "/Logs/temperature", dht.readTemperature());
}

void loop() {
  sensorUpdate();
  Blynk.run();

  theTime = millis() / 1000;
  if (theTime > beforeTime) {
    secSS++;
    if (secSS >= 60) {
      secSS = 0;
      minMM++;
      if (minMM >= 60) {
        minMM = 0;
        hrsHH++;
        if (hrsHH >= 99) {
          secSS = 0;
          minMM = 0;
          hrsHH = 0;
        }
      }
    }
    beforeTime = theTime;
  }
  Serial.printf("\nTime = %02d:%02d:%02d\n\n", hrsHH, minMM, secSS);

  if (theTime % 600 == 0) { saveDataToFirebase(); }
  delay(1000);
}
