import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Cargar los datos desde el archivo CSV
df = pd.read_csv('data_bajasfrecuencias.csv', delimiter=',')
# Seleccionar la columna 'filtroE' como la señal filtrada
signal = df['filtroE']

# Tiempo total de muestreo y periodo de muestreo
total_time = 10  # segundos
sampling_period = 0.001  # segundos

# Calcular la frecuencia de muestreo
sampling_rate = 1 / sampling_period

# Calcular el número total de muestras
num_samples = int(total_time / sampling_period)

# Parámetros para el espectrograma
window_size = 256  # Tamaño de la ventana para calcular la transformada de Fourier
overlap = window_size // 2  # Superposición del 50% entre ventanas

# Calcular la Transformada de Fourier en ventanas
spectrogram, freqs, times, im = plt.specgram(signal, NFFT=window_size, Fs=sampling_rate, noverlap=overlap, cmap='viridis')

# Graficar el espectrograma
plt.title('Espectrograma de la Señal de Aceleración')
plt.xlabel('Tiempo (segundos)')
plt.ylabel('Frecuencia (Hz)')
plt.colorbar(label='Amplitud (dB)')

plt.savefig('espectrograma_aceleracion_bajasfrecuencias.png')
plt.close()
