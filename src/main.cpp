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
// #include "Adafruit_BME680.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model.h"

#define FIREBASE_HOST "psdatabase-fb5fe-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyAeCOk_JF2QCtSxUsRS6gYeESK0Q6zNNnw"
#define FIREBASE_ROOT "/esptflite/"
#define RO_MQ7 1.42
#define RO_MQ135 12.15
#define ADC_CONVERSION 0.1875 / 1000.0
#define FIREBASE_SEND_DATA_INTERVAL 3000
#define MQ7_READ_SENSOR_INTERVAL 5000
#define MQ135_READ_SENSOR_INTERVAL 5000
// #define WIFI_SSID "ZTE_2.4G_7S9tEA"
// #define WIFI_PASSWORD "MUKTI234"
#define WIFI_SSID "Jawir Creative Space"
#define WIFI_PASSWORD "jawirspace"
// #define WIFI_SSID "Default"
// #define WIFI_PASSWORD "default234"
#define MQ7_CHANNEL 2
#define MQ135_CHANNEL 1
#define GP2Y1014_CHANNEL 0
#define GP2Y1014_LED_PIN 25

namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;

  constexpr int kTensorArenaSize = 4096;
  uint8_t tensor_arena[kTensorArenaSize];

  const char* categories[] = {"aktivitas", "kimia", "makanminum", "normal"};
  constexpr int kCategoryCount = sizeof(categories) / sizeof(categories[0]);
}

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
unsigned long previousMQ135Time = 0;
int16_t mq7Value = 0;
int16_t mq135Value = 0;

float temperature = 0.00;
float humidity = 0.00;
float pressure = 0.00;
float voc = 0.00;

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

  // setting model tensorflow
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Load model
  model = tflite::GetModel(model_feedforward_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model versi %d tidak cocok dengan versi %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Inisialisasi resolver dan interpreter
  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Alokasikan memori untuk tensor
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Gagal AllocateTensors()");
    return;
  }

  // Ambil input dan output tensor
  input = interpreter->input(0);
  output = interpreter->output(0);
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

  if (millis() - previousMQ135Time >= MQ135_READ_SENSOR_INTERVAL) {
    mq135Value = ads.readADC_SingleEnded(MQ135_CHANNEL);
    previousMQ135Time = millis();
  }
  // const float ADC_CONVERSION = 0.1875 / 1000.0;
  
  // MQ7 (CO)
  float voltage_mq7 = mq7Value * ADC_CONVERSION;
  float Rs_mq7 = (10.0 * (3.3 - voltage_mq7)) / voltage_mq7;
  float co_ppm = 1000.0 / (Rs_mq7 / RO_MQ7);
  
  // MQ135 (CO2)
  float voltage_mq135 = mq135Value * ADC_CONVERSION;
  float Rs_mq135 = (10.0 * (3.3 - voltage_mq135)) / voltage_mq135;
  float co2_ppm = 116.6020682 * pow((Rs_mq135 / RO_MQ135), -2.769034857);

  digitalWrite(GP2Y1014_LED_PIN, LOW);
  delay(280);
  int16_t dustValue = ads.readADC_SingleEnded(GP2Y1014_CHANNEL);
  digitalWrite(GP2Y1014_LED_PIN, HIGH);
  delay(40);

  String dateString = String(dateTimeString).substring(0, 10);
  String timeString = String(dateTimeString).substring(11, 19);

  Serial.print("Date (YYYY-MM-DD) : "); Serial.println(dateString);
  Serial.print("Time (HH:MM:SS)   : "); Serial.println(timeString);
  Serial.print("epoch             : "); Serial.println(String(epochTime));
  Serial.print("Co                : "); Serial.println(co_ppm);
  Serial.print("CO2               : "); Serial.println(co2_ppm);
  Serial.print("Dust Sensor Value : "); Serial.println(dustValue);

  float input_data[5] = {
    static_cast<float>(dustValue), 
    static_cast<float>(mq135Value), 
    static_cast<float>(mq7Value), 
    static_cast<float>(temperature), 
    static_cast<float>(voc)
  };

  for (int i = 0; i < 5; i++) {
    input->data.f[i] = input_data[i];
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Gagal menjalankan model");
    return;
  }

  float max_value = -1.0;
  int max_index = -1;
  for (int i = 0; i < kCategoryCount; i++) {
    float probability = output->data.f[i];
    if (probability > max_value) {
      max_value = probability;
      max_index = i;
    }
  }

  if (max_index != -1) {
    Serial.printf("Prediksi: %s (%.2f%%)\n", categories[max_index], max_value * 100);
  }
  Serial.println("");

  if (bme.performReading()) {
    temperature = bme.temperature;
    humidity = bme.humidity;
    pressure = bme.pressure / 100.0;
    voc = bme.gas_resistance / 1000.0;

    Serial.print("Temperature       : "); Serial.print(temperature); Serial.println(" C");
    Serial.print("Humidity          : "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Pressure          : "); Serial.print(pressure); Serial.println(" hPa");
    Serial.print("VOC               : "); Serial.print(voc); Serial.println(" Ohm");

    if (millis() - previousFirebaseTime >= FIREBASE_SEND_DATA_INTERVAL) {
      Serial.println("Trying to send to firebase");

      Database.set<float>(client, String(FIREBASE_ROOT) + "/temperature", temperature);
      delay(20);
      Database.set<float>(client, String(FIREBASE_ROOT) + "/humidity", humidity);
      delay(20);
      Database.set<float>(client, String(FIREBASE_ROOT) + "/pressure", pressure);
      delay(20);
      Database.set<float>(client, String(FIREBASE_ROOT) + "/CO2", co2_ppm);
      delay(20);
      Database.set<float>(client, String(FIREBASE_ROOT) + "/CO", co_ppm);
      delay(20);
      Database.set<int>(client, String(FIREBASE_ROOT) + "/dust", dustValue);
      delay(20);
      Database.set<float>(client, String(FIREBASE_ROOT) + "/voc", voc);
      delay(20);
      Database.set<String>(client, String(FIREBASE_ROOT) + "/status", categories[max_index]);

      previousFirebaseTime = millis();
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(2000);
  }

  delay(1000);
}
