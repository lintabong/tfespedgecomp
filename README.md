# Sistem Prediksi Sensor dengan ESP32 dan Integrasi Firebase

Proyek ini menunjukkan pipeline pembelajaran mesin di mana ESP32 terhubung dengan empat sensor untuk pemantauan lingkungan secara real-time. ESP32 mengumpulkan data, memprosesnya menggunakan model TensorFlow Lite, dan mengirim prediksi ke Firebase untuk analisis dan visualisasi lebih lanjut. Model ML dibuat menggunakan Python dalam Jupyter notebook (.ipynb) dan dikonversi ke format TensorFlow Lite untuk deployment pada ESP32.

## Ringkasan Proyek

ESP32 terhubung dengan empat sensor:
- **Sensor CO2**
- **Sensor CO**
- **Sensor Kualitas Udara**
- **Sensor Debu**

ESP32 mengambil data dari masing-masing sensor, menggunakan model TensorFlow Lite untuk memprediksi tingkat kualitas udara (rendah, sedang, atau tinggi), dan kemudian mengirim data prediksi ke Firebase.

## Daftar Isi
- [Kebutuhan Perangkat Keras](#kebutuhan-perangkat-keras)
- [Kebutuhan Perangkat Lunak](#kebutuhan-perangkat-lunak)
- [Pelatihan dan Konversi Model](#pelatihan-dan-konversi-model)
- [Struktur Kode ESP32](#struktur-kode-esp32)
- [Integrasi Firebase](#integrasi-firebase)
- [Penggunaan](#penggunaan)
- [Struktur Folder](#struktur-folder)

## Kebutuhan Perangkat Keras
- **ESP32** mikrocontroller
- **Sensor CO2**
- **Sensor CO**
- **Sensor Kualitas Udara**
- **Sensor Debu**
- Kabel jumper dan breadboard

## Kebutuhan Perangkat Lunak
- **Python 3.x** untuk pelatihan model
- **Jupyter Notebook** (atau VS Code dengan ekstensi Jupyter)
- **TensorFlow** (untuk pelatihan dan konversi model)
- **Arduino IDE** atau **PlatformIO** untuk deploy model di ESP32
- Akun **Firebase** untuk penyimpanan cloud dan pembaruan secara real-time

### Library Python (untuk pelatihan model)
Untuk menginstal library yang dibutuhkan:
```bash
pip install numpy pandas tensorflow scikit-learn matplotlib
