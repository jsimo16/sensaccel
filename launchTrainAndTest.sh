#!/bin/bash
DATASETCVSFILE=../../00_Dataset/final.csv
MODELFILE=./07_modelo_aceleraciones_features
python3 ./TrainAndTestMLP_xp.py -f $DATASETCVSFILE -m $MODELFILE.h5 -e 5 -n 10
python3 ../keras_export/convert_model.py $MODELFILE.h5 $MODELFILE.json
