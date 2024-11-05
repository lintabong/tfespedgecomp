import os
import numpy
import pandas
import tensorflow
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, BatchNormalization, Dropout
from tensorflow.keras.utils import to_categorical
from sklearn.preprocessing import LabelEncoder
import matplotlib.pyplot as plt

numpy.random.seed(42)

n_samples = 500

X = numpy.random.rand(n_samples, 4)

y_labels = numpy.random.choice(['rendah', 'sedang', 'tinggi'], n_samples)

df = pandas.DataFrame(X, columns=['gas_CO2', 'CO', 'kualitas_udara', 'sensor_debu'])
df['kategori'] = y_labels

encoder = LabelEncoder()
y_encoded = encoder.fit_transform(y_labels)
y_categorical = to_categorical(y_encoded)

model = Sequential([
    Dense(64, input_shape=(4,), activation='relu'),  # Layer pertama dengan 64 neuron
    BatchNormalization(),
    Dropout(0.3),  # Dropout untuk mencegah overfitting
    
    Dense(32, activation='relu'),  # Hidden layer kedua dengan 32 neuron
    BatchNormalization(),
    Dropout(0.3),
    
    Dense(16, activation='relu'),  # Hidden layer ketiga dengan 16 neuron
    BatchNormalization(),
    Dropout(0.2),
    
    Dense(3, activation='softmax')  # Output layer dengan 3 neuron (3 kelas)
])

model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])

history = model.fit(X, y_categorical, epochs=50, batch_size=8, verbose=0, validation_split=0.2)

plt.figure(figsize=(12, 5))
plt.subplot(1, 2, 1)
plt.plot(history.history['accuracy'], 'b', label='Training accuracy')
plt.plot(history.history['val_accuracy'], 'r', label='Validation accuracy')
plt.title('Training and validation accuracy')
plt.xlabel('Epochs')
plt.ylabel('Accuracy')
plt.legend()

plt.subplot(1, 2, 2)
plt.plot(history.history['loss'], 'b', label='Training loss')
plt.plot(history.history['val_loss'], 'r', label='Validation loss')
plt.title('Training and validation loss')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()

plt.tight_layout()
plt.show()

loss, accuracy = model.evaluate(X, y_categorical, verbose=0)
print(f"Loss: {loss:.4f}, Accuracy: {accuracy:.4f}")

predictions = model.predict(X[:5])  
predicted_labels = encoder.inverse_transform(numpy.argmax(predictions, axis=1))
print("Prediksi kelas untuk sampel pertama:")
for i, prediction in enumerate(predicted_labels):
    print(f"Sampel {i+1}: {prediction}")

converter = tensorflow.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

with open('model.tflite', 'wb') as f:
    f.write(tflite_model)