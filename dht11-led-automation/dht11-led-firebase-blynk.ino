// Blynk config authentication
#define BLYNK_TEMPLATE_ID "XXXXX"
#define BLYNK_DEVICE_NAME "XXXXX"
#define BLYNK_AUTH_TOKEN "XXXXX"

// FirebaseESP8266.h must be included before ESP8266WiFi.h
// Install Firebase ESP8266 library
#include "FirebaseESP8266.h"

// Firebase host and auth without http:// or https:// schemes
#define FIREBASE_HOST "XXXXX"
#define FIREBASE_AUTH "XXXXX"

// Wi-Fi Config
#define WIFI_SSID "XXXXX"
#define WIFI_PASSWORD "XXXXX"

// Define FirebaseESP8266 data object
FirebaseData firebaseData;

// Parse the json
FirebaseJson json;

// Connect Data pin of DHT to D6
#define DHTPIN D6
// Connect LED to D5
int led = D5;

// Install the ESP8266 Wi-Fi library
#include <ESP8266WiFi.h>
// Install DHT11 Library and Adafruit Unified Sensor Library
#include <DHT.h>

// Define the DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

// Include the Blynk simple ESP8266
#include <BlynkSimpleEsp8266.h>

// Auth token Blynk
char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

BlynkTimer timer;

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  Serial.printf("\nBlynk Connected Successfully!\n");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);

  //Uptime seconds
  int uptimeInSeconds = millis() / 1000;

  // Uptime conversion from second to minutes
  int uptimeInMinutes = (millis() / 1000) / 60;

  // Uptime conversion from second to hour
  int uptimeInHours = ((millis() / 1000) / 60) / 60;

  if (Firebase.setInt(firebaseData, "/FirebaseIOT/uptimeInSeconds", uptimeInSeconds))
  {
    Serial.printf("\n+%ds", 1);
  }
  else
  {
    Serial.println("\nERROR (uptimeInSeconds): " + firebaseData.errorReason());
  }

  if (Firebase.setInt(firebaseData, "/FirebaseIOT/uptimeInMinutes", uptimeInMinutes))
  {
    Serial.printf("\n+%.2fm", 0.02);
  }
  else
  {
    Serial.println("\nERROR (uptimeInMinutes): " + firebaseData.errorReason());
  }
  
  if (Firebase.setInt(firebaseData, "/FirebaseIOT/uptimeInHours", uptimeInHours))
  {
    Serial.printf("\n+%.4fh\n", 0.0003);
  }
  else
  {
    Serial.println("\nERROR (uptimeInHours): " + firebaseData.errorReason());
  }
}

void setup()
{

  Serial.begin(9600);

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);

  dht.begin();
  pinMode(led, OUTPUT);
  
  Blynk.begin(auth, ssid, pass);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Set the LED first setup to LOW
  digitalWrite(led, LOW);
}

void sensorUpdate()
{
  // Reading temperature or humidity takes about 1000 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();

  // Uptime in seconds
  int everyTenMinutes = millis() / 1000;

  // Save the data to firebase every 10 minutes 
  if (everyTenMinutes % 600 == 0)
  {
    if (Firebase.pushFloat(firebaseData, "/Logs/humidity", h))
    {}
    else
    {
      Serial.println("\nERROR (Humidity logs): " + firebaseData.errorReason());
    }
  }
  
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Save the data to firebase every 10 minutes 
  if (everyTenMinutes % 600 == 0)
  {
    if (Firebase.pushFloat(firebaseData, "/Logs/temperature", t))
    {}
    else
    {
      Serial.println("\nERROR (Temperature logs): " + firebaseData.errorReason());
    }
  }

  // Check if the temperature higher than 30°C then turn on the LED
  if (t >= 30)
  {
    digitalWrite(led, LOW);
    Blynk.virtualWrite(V1, 1);
    Firebase.setInt(firebaseData, "/FirebaseIOT/led", 1);
  }
  else
  {
    digitalWrite(led, HIGH);
    Blynk.virtualWrite(V1, 0);
    Firebase.setInt(firebaseData, "/FirebaseIOT/led", 0);
  }
  
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(false);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Send data of humidity to Blynk  
  Blynk.virtualWrite(V5, h);

  // Send data of temperature to Blynk
  Blynk.virtualWrite(V4, t);
  
  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/temperature", t))
  {
    Serial.printf("\nTemperature : %.1f°C", t);
  }
  else
  {
    Serial.println("\nERROR (Temperature): " + firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/humidity", h))
  {
    Serial.printf("\nHumidity    : %.1f%%", h);
  }
  else
  {
    Serial.println("\nERROR (Humidity): " + firebaseData.errorReason());
  }
}
void loop()
{
  // Run the sensor update function
  sensorUpdate();

  // Run the Blynk
  Blynk.run();
  
  // Run the timer for uptime
  timer.run();

  // Optional delay
  delay(300);
}
