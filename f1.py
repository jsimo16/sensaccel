import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Cargar los datos desde el archivo CSV
df = pd.read_csv('data_bajasfrecuencias.csv', delimiter=',')

signal = df['z']

# Parámetros del filtro exponencial
alphas = np.arange(0, 1.1, 0.1)  # Valores de alpha de 0 a 1 con incrementos de 0.1
alpha=0.3
# Aplicar el filtro exponencial para diferentes valores de alpha
# Inicializar variables de salida
filt = np.copy(signal)
    # Aplicar el filtro exponencial
for i in range(1, len(signal)):
    filt[i] = alpha * filt[i - 1] + signal[i] * (1 - alpha)
# Agregar la columna 'filtroE' al DataFrame
df['filtroE'] = filt

# Guardar el DataFrame actualizado en el archivo CSV
df.to_csv('data_bajasfrecuencias.csv', index=False, sep=',')

# Graficar la señal original y las señales filtradas
plt.figure(figsize=(12, 6))
plt.plot(df['inicio'], signal, label='Señal Original', alpha=0.5)
plt.plot(df['inicio'], filt, label=f'Filtro 1 Alpha={alpha:.2f}', linewidth=2)
plt.title('Filtrado Exponencial Sucesivo')
plt.xlabel('Tiempo (msegundos)')
plt.ylabel('Aceleración')
plt.legend()
plt.savefig(f'filtro_FFT_alpha_{alpha:.2f}.png')
plt.close()
print("Proceso completado.")

#Tomamos la señal de aceleración del acelerómetro parado y luego con el acelerometro parado pero el motorcillo moviéndose, aplicamos el motor y así eliminamos el ruido ambiental (queremos filtrar altas frecuencias).
#Luego probare con varias velocidades y con distintas alphas.
#La inyección de bajas frecuencias simula el mal funcionamiento de un engranaje (holgura en una transmisión) y la inyección de altas frecuencias simula el mal funcionamiento de alguno de los motores.

#TOMAR VALOR DE ALPHA O.3 y APLICAR UN UNICO FILTRO
#APLICAR LA TRANSFORMADA DE FOURIER SOBRE LA SEÑAL CON LA MÁQUINA EN MOVIMIENTO E IR VARIANDO LA VELOCIDAD DE GIRO DEL MOTOR (VARIAR VOLTIOS) PARA INDICAR LOS PICOS DE FRECUENCIA PRODUCIDOS Y ASÍ, COMPROBAMOS SI SOMOS CAPACES DE AUN ASI IDENTIFICAR SI SE ESTÁN PRODUCIENDO PERTURBACIONES.
