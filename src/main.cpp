#include <Arduino.h>
#include <FirebaseClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#define FIREBASE_HOST "psdatabase-fb5fe-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyAeCOk_JF2QCtSxUsRS6gYeESK0Q6zNNnw"
#define WIFI_SSID "Didik"
#define WIFI_PASSWORD "22des*92"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

WiFiClientSecure ssl;
DefaultNetwork network;
AsyncClientClass client(ssl, getNetwork(network));

FirebaseApp app;
RealtimeDatabase Database;
AsyncResult result;
LegacyToken dbSecret(FIREBASE_AUTH);

void printError(int code, const String &msg)
{
    Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

void setup(){
  Serial.begin(115200);
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

  // Inisialisasi NTP
  timeClient.begin();
  timeClient.update();


  pinMode(2, OUTPUT);
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

  Serial.println(dateTimeString);

  int CO2value = random(0, 100);
  int COvalue = random(0, 100);
  int airQualityvalue = random(0, 100);
  int dustvalue = random(0, 100);

  Serial.println("Send data");

  String jsonData = "{\"CO2\": " + String(CO2value) +
                  ", \"CO\": " + String(COvalue) +
                  ", \"airQuality\": " + String(airQualityvalue) +
                  ", \"dust\": " + String(dustvalue) +
                  ", \"timestamp\": " + String(epochTime) +
                  ", \"datetime\": \"" + String(dateTimeString) + "\"}";

  bool status = Database.push<String>(client, "/esptflite/data", jsonData);

  // String pushCO2 = Database.push<int>(client, "/esptflite/CO2", CO2value);
  // String pushCO = Database.push<int>(client, "/esptflite/CO", COvalue);
  // String pushAirQuality = Database.push<int>(client, "/esptflite/airQuality", airQualityvalue);
  // String pushDust = Database.push<int>(client, "/esptflite/dust", dustvalue);

  String message = "";
  if (CO2value < 30){
    message = "LOW";
  } else if (CO2value >= 30 && CO2value < 60){
    message = "NORMAL";
  } else {
    message = "HIGH";
  }

  status = Database.set<String>(client, "/esptflite/status", message);

  delay(10000);

  digitalWrite(2, HIGH);
  delay(1000);
  digitalWrite(2, LOW);
  delay(1000);
}
