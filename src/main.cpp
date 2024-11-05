#include <TensorFlowLite.h>
#include "model.h" 

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

const int kTensorArenaSize = 2 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;

const tflite::Model* model = tflite::GetModel(model_tflite);
if (model->version() != TFLITE_SCHEMA_VERSION) {
  error_reporter->Report("Model schema version tidak kompatibel!");
  while (1);
}

tflite::MicroMutableOpResolver<5> resolver;
resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED, tflite::ops::micro::Register_FULLY_CONNECTED);
resolver.AddBuiltin(tflite::BuiltinOperator_RELU, tflite::ops::micro::Register_RELU);
resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX, tflite::ops::micro::Register_SOFTMAX);

tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);

// Menyediakan input dan output tensor
TfLiteTensor* input = interpreter.input(0);
TfLiteTensor* output = interpreter.output(0);

// Setup fungsi
void setup() {
  Serial.begin(115200);

  // Initialize interpreter
  TfLiteStatus allocate_status = interpreter.AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    while (1);
  }
}

// Fungsi untuk memasukkan data sensor ke dalam model
void readSensorData() {
  // Contoh data input, ganti dengan pembacaan sensor sebenarnya
  input->data.f[0] = 0.2;  // Nilai gas CO2
  input->data.f[1] = 0.5;  // Nilai gas CO
  input->data.f[2] = 0.7;  // Kualitas udara
  input->data.f[3] = 0.3;  // Pembacaan sensor debu
}

// Inferensi data
void runInference() {
  // Menjalankan inferensi
  TfLiteStatus invoke_status = interpreter.Invoke();
  if (invoke_status != kTfLiteOk) {
    error_reporter->Report("Invoke gagal!");
    return;
  }

  // Output: Mengambil nilai prediksi
  float low = output->data.f[0];
  float medium = output->data.f[1];
  float high = output->data.f[2];

  // Tampilkan hasil prediksi
  Serial.print("Kategori Rendah: "); Serial.println(low);
  Serial.print("Kategori Sedang: "); Serial.println(medium);
  Serial.print("Kategori Tinggi: "); Serial.println(high);

  // Tentukan kategori berdasarkan nilai tertinggi
  if (low > medium && low > high) {
    Serial.println("Prediksi: Rendah");
  } else if (medium > low && medium > high) {
    Serial.println("Prediksi: Sedang");
  } else {
    Serial.println("Prediksi: Tinggi");
  }
}

void loop() {
  // Membaca data dari sensor
  readSensorData();

  // Menjalankan inferensi
  runInference();

  // Tunggu sebelum pembacaan berikutnya
  delay(2000);
}
