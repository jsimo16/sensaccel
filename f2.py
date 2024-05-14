import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Cargar los datos desde el archivo CSV
df = pd.read_csv('data_quieto_altas.csv', delimiter=',')

out = df['z']

# Valores de M entre 11 y 41 con incrementos de 10
window_sizes = range(11, 42, 10)

# Bucle para aplicar el filtro de promedio para diferentes valores de M
for M in window_sizes:
    print("M ",M)
    half_M = (M - 1) // 2

    # Inicializar un array para la señal filtrada
    filt = np.zeros_like(out)

    # Inicialización de la primera ventana temporal
    suma = np.sum(out[:M])

    # Bucle de cálculo optimizado con recursión al valor anterior de la suma
    for i in range((M-1)//2, len(out)-(M-1)//2):
        if i + (M-1)//2 + 1 < len(out) and i - (M-1)//2 >= 0:
            filt[i] = suma / M
            suma = suma + out[i + (M-1)//2 + 1] - out[i - (M-1)//2]

    # Graficar la señal original y la señal filtrada
    plt.figure(figsize=(12, 6))
    plt.plot(df['inicio'], out, label='Señal Original', alpha=0.5)
    plt.plot(df['inicio'], filt, label=f'Filtro M={M}', linewidth=2)
    plt.title('Filtro de Promedio de Ventana Deslizante')
    plt.xlabel('Tiempo (msegundos)')
    plt.ylabel('Aceleración')
    plt.legend()
    plt.savefig(f'filtro_quieto_altas_M_{M}.png')
    plt.close()

print("Proceso completado.")


