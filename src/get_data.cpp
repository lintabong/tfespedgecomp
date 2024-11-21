#include <Arduino.h>
#include <FirebaseClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include "Adafruit_BME680.h"

#define FIREBASE_HOST "psdatabase-fb5fe-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyAeCOk_JF2QCtSxUsRS6gYeESK0Q6zNNnw"
#define FIREBASE_ROOT "/esptflite/"
#define FIREBASE_SEND_DATA_INTERVAL 3000
#define MQ7_READ_SENSOR_INTERVAL 3000
#define WIFI_SSID "Default"
#define WIFI_PASSWORD "default234"
#define MQ7_CHANNEL 2
#define MQ135_CHANNEL 1
#define GP2Y1014_CHANNEL 0
#define GP2Y1014_LED_PIN 25

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

WiFiClientSecure ssl;
DefaultNetwork network;
AsyncClientClass client(ssl, getNetwork(network));

FirebaseApp app;
RealtimeDatabase Database;
AsyncResult result;
LegacyToken dbSecret(FIREBASE_AUTH);

Adafruit_ADS1115 ads;
Adafruit_BME680 bme;


unsigned long previousFirebaseTime = 0;
unsigned long previousMQ7Time = 0;
int16_t mq7Value = 0;
int16_t mq135Value = 0;

void printError(int code, const String &msg){
    Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

void setup(){
  Serial.begin(115200);
  delay(1500);

  if (!ads.begin()) {
    Serial.println("ADS1115 tidak terdeteksi. Cek koneksi!");
    while (1);
  }
  
  Serial.println("ADS1115 terdeteksi!");

  if (!bme.begin()) {
    Serial.println("BME680 tidak terdeteksi!");
    while (1);
  }
  Serial.println("BME680 terdeteksi!");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  ssl.setInsecure();
  initializeApp(client, app, getAuth(dbSecret));
  app.getApp<RealtimeDatabase>(Database);
  Database.url(FIREBASE_HOST);
  client.setAsyncResult(result);

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320°C for 150 ms

  timeClient.begin();
  timeClient.update();

  pinMode(GP2Y1014_LED_PIN, OUTPUT);
}

void loop(){
  timeClient.update();

  time_t epochTime = timeClient.getEpochTime();
  struct tm timeinfo;
  localtime_r(&epochTime, &timeinfo);

  char dateTimeString[20];
  snprintf(dateTimeString, sizeof(dateTimeString), "%04d-%02d-%02d %02d:%02d:%02d",
            timeinfo.tm_year + 1900,
            timeinfo.tm_mon + 1,
            timeinfo.tm_mday,
            timeinfo.tm_hour,
            timeinfo.tm_min,
            timeinfo.tm_sec);

  if (millis() - previousMQ7Time >= MQ7_READ_SENSOR_INTERVAL) {
    mq7Value = ads.readADC_SingleEnded(MQ7_CHANNEL);
    previousMQ7Time = millis();
  }

  mq135Value = ads.readADC_SingleEnded(MQ135_CHANNEL);
  bool bmeValue = bme.performReading();

  digitalWrite(GP2Y1014_LED_PIN, LOW);
  delay(280);
  int16_t dustValue = ads.readADC_SingleEnded(GP2Y1014_CHANNEL);
  digitalWrite(GP2Y1014_LED_PIN, HIGH);
  delay(40);

  String dateString = String(dateTimeString).substring(0, 10);
  String timeString = String(dateTimeString).substring(11, 19);

  Serial.print("Date (YYYY-MM-DD) : "); Serial.println(dateString);
  Serial.print("Time (HH:MM:SS)   : "); Serial.println(timeString);
  // Serial.print(dateTimeString);Serial.println(" ==> Send data");
  Serial.print("epoch             : "); Serial.println(String(epochTime));
  Serial.print("mq7Value          : "); Serial.println(mq7Value);
  Serial.print("mq135Value        : "); Serial.println(mq135Value);
  Serial.print("Dust Sensor Value : "); Serial.println(dustValue);

  if (bmeValue) {
    float temperature = bme.temperature;
    float humidity = bme.humidity;
    float pressure = bme.pressure / 100.0;
    float voc = bme.gas_resistance / 1000.0;

    Serial.print("Temperature       : "); Serial.print(temperature); Serial.println(" C");
    Serial.print("Humidity          : "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Pressure          : "); Serial.print(pressure); Serial.println(" hPa");

    String firebasePathDataSave = String(FIREBASE_ROOT) + "history/" + dateString +"/"+ timeString + "/";

    if (millis() - previousFirebaseTime >= FIREBASE_SEND_DATA_INTERVAL) {
      Serial.println("Trying to send to firebase");
      if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        delay(2000);
      }
      Database.set<float>(client, firebasePathDataSave + "temperature", temperature);
      delay(20);
      Database.set<float>(client, firebasePathDataSave + "humidity", humidity);
      delay(20);
      Database.set<float>(client, firebasePathDataSave + "pressure", pressure);
      delay(20);
      Database.set<int>(client, firebasePathDataSave + "mq135", mq135Value);
      delay(20);
      Database.set<int>(client, firebasePathDataSave + "mq7", mq7Value);
      delay(20);
      Database.set<int>(client, firebasePathDataSave + "dust", dustValue);
      delay(20);
      Database.set<int>(client, firebasePathDataSave + "voc", voc);
      delay(20);
      Database.set<String>(client, firebasePathDataSave + "epoch", String(epochTime));

      // Database.set<float>(client, String(FIREBASE_ROOT) + "temperature", temperature);
      // Database.set<float>(client, String(FIREBASE_ROOT) + "humidity", humidity);
      // Database.set<float>(client, String(FIREBASE_ROOT) + "pressure", pressure);
      // Database.set<int>(client, String(FIREBASE_ROOT) + "mq135", mq135Value);
      // Database.set<int>(client, String(FIREBASE_ROOT) + "mq7", mq7Value);
      // Database.set<int>(client, String(FIREBASE_ROOT) + "dust", dustValue);
      // Database.set<String>(client, String(FIREBASE_ROOT) + "epoch", String(epochTime));

      previousFirebaseTime = millis();
    }
  } 
  Serial.println("");

  // String message = "";
  // if (CO2value < 30){
  //   message = "LOW";
  // } else if (CO2value >= 30 && CO2value < 60){
  //   message = "NORMAL";
  // } else {
  //   message = "HIGH";
  // }

  // bool status = Database.set<String>(client, "/esptflite/status", message);
  delay(1000);
}
