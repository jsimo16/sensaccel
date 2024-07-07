# sensaccel
# Proyecto de Fin de Máster: Detección de Anomalías en Máquinas de Corte con Láser

Este repositorio contiene los códigos y datos utilizados en el proyecto de fin de máster dedicado a la evaluación de diferentes técnicas de procesamiento de información obtenida mediante instrumentación. El objetivo principal es la detección de anomalías de funcionamiento mecánico.

## Descripción del Proyecto

Como punto de partida, se han utilizado datos obtenidos mediante el muestreo de un acelerómetro montado en un pórtico XYZ, cuyo movimiento está controlado por motores paso a paso. Los datos recolectados por la instrumentación se han procesado utilizando técnicas de procesamiento digital y redes neuronales. La meta es determinar la capacidad de estas técnicas para predecir la aparición de fallos en el funcionamiento de la máquina.

### Inyección de Fallos

Para la evaluación experimental de las técnicas de procesamiento de información, se han inyectado diferentes tipos de fallos bajo diversos escenarios de funcionamiento:
- Ruido de alta frecuencia
- Ruido de baja frecuencia
- Cabeceos

### Evaluación de Resultados

Los resultados obtenidos se comparan tanto cuantitativa como cualitativamente, evaluando la eficacia de cada enfoque. Las conclusiones derivadas de este trabajo contribuyen al avance en la detección temprana de anomalías en máquinas de corte con láser, brindando aplicaciones prácticas para el mantenimiento predictivo y la mejora de la eficiencia operativa.

## Contenido del Repositorio

- Contiene los datos de muestreo del acelerómetro bajo diferentes condiciones de funcionamiento.
- Incluye los scripts utilizados para el procesamiento de datos, análisis y entrenamiento de modelos.
- `README.md`: Este archivo con la descripción del proyecto y la estructura del repositorio.
