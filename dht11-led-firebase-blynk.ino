// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "XXXXX"
#define BLYNK_DEVICE_NAME "XXXXX"
#define BLYNK_AUTH_TOKEN "XXXXX"

// FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h" // Install Firebase ESP8266 library

#define FIREBASE_HOST "XXXXX" // Without http:// or https:// schemes
#define FIREBASE_AUTH "XXXXX"
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
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);

  //Uptime seconds
  int uptimeSeconds = millis() / 1000;

  // Uptime conversion from second to hour
  float uptimeHours = ((millis() / 1000) / 60) / 60;

  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/uptimeSeconds", uptimeSeconds))
  {
    Serial.println("+1s");
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
  }
  
  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/uptimeHours", uptimeHours))
  {
    Serial.println("+0.000277778h");
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
  }
}

void setup()
{

  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);

  dht.begin();
  pinMode(led, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

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
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if the temperature higher than 30°C then turn on the LED
  if (t >= 30)
  {
    digitalWrite(led, LOW);
    Blynk.virtualWrite(V1, 1);
  }
  else
  {
    digitalWrite(led, HIGH);
    Blynk.virtualWrite(V1, 0);
  }
  
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(false);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.println(h);
  Blynk.virtualWrite(V5, h);
  Serial.print(F("%  Temperature: "));
  Serial.println(t);
  Blynk.virtualWrite(V4, t);
  Serial.print(F("C  ,"));
  Serial.println(f);
  Serial.print(F("F  "));

  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/temperature", t))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/humidity", h))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}
void loop()
{
  // Run the sensor update function
  sensorUpdate();

  // Run the blynk
  Blynk.run();
  // Run the timer for uptime
  timer.run();

  delay(1000);
}
