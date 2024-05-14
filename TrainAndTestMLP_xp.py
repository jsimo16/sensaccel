import getopt
import sys
import numpy as np
import pandas as pd
from tensorflow.keras.models import Sequential, load_model
from tensorflow.keras.layers import Dense, Activation
from sklearn.model_selection import train_test_split, KFold
import itertools
import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix

def usage():
    print("\nUsage: TrainAndTestMLP_xp.py [-help] [-fileCSVtrain value] [-modelfileOutput_h5 value] [-epochs value] [-neurons_per_layer value]")
    print(" abreviated form: [h f: m: e: n:]\n")
    print(" Example:")
    print("           python3 ./TrainAndTestMLP_xp.py -m ./cvsfilename.csv -m ./modeltrained.h5 -e 400 -n 100 2>/dev/null  | grep \"Success ratio\"")
    print("\n")

def getOpts(argv):
    inCSVfile = "./DatasetDump21.csv"
    modelH5file = "07_modelo_boquillas_features.h5"
    epochs = 400
    neurons_per_layer = 100
    try:
        opts, args = getopt.getopt(sys.argv[1:], "h f:m:e:n:", ["help", "fileCSV=", "modelH5file=", "epochs=", "neurons_per_layer="])
    except getopt.GetoptError as err:
        usage()
        sys.exit(2)
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-f", "--fileCSV"):
            inCSVfile = a
        elif o in ("-m", "--modelH5file"):
            modelH5file = a
        elif o in ("-e", "--epochs"):
            epochs = int(a)
        elif o in ("-n", "--neurons_per_layer"):
            neurons_per_layer = int(a)
        else:
            assert False, "unhandled option"
    return (inCSVfile, modelH5file, epochs, neurons_per_layer)

plt.rcParams["font.family"] = 'DejaVu Sans'
def plot_confusion_matrix(cm, classes,
                          normalize=True,
                          title='Confusion matrix',
                          cmap=plt.cm.Blues):
    if normalize:
        cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]*100

    plt.imshow(cm, interpolation='nearest', cmap=cmap)
    plt.title(title)
    plt.colorbar()
    tick_marks = np.arange(len(classes))
    plt.xticks(tick_marks, classes, rotation=90)
    plt.yticks(tick_marks, classes)

    fmt = '.2f' if normalize else 'd'
    thresh = cm.max() / 2.
    for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
        plt.text(j, i, format(cm[i, j], fmt),
                 horizontalalignment="center",
                 color="white" if cm[i, j] > thresh else "black")

    plt.tight_layout()
    plt.ylabel('True label')
    plt.xlabel('Predicted label')
    del cm, tick_marks, i, j, fmt

if __name__ == "__main__":
    if len(sys.argv) == 1:
        usage()
        exit(0)

    (dataFilename, modelFilename, EPOCHS, NEURONS_HIDDEN_LAYER) = getOpts(sys.argv)
    print("\n\n")
    print("**************** Running information ************************")
    print("Data file name: ", dataFilename)
    print("Model file name: ", modelFilename)
    print("Epochs: ", EPOCHS)
    print("Neurons per hidden layer: ", NEURONS_HIDDEN_LAYER)

    df = pd.read_csv(dataFilename, sep=',')
    NCOLUMNAS = len(df.columns)
    NFEATURES = NCOLUMNAS - 1

    np_y = df.iloc[:, [-1]].to_numpy().reshape(len(df))
    print("npy",np_y)
    np_X = df.iloc[:, :-1].to_numpy()
    print("npx",np_X)

    X_train, X_test, y_train, y_test = train_test_split(np_X, np_y, test_size=0.30, random_state=100)

    NDATA = len(X_train)
    NCLASSES = len(np.unique(np_y))

    print("\n\n")
    print("**************** Dataframe information ************************")
    print("Number of columns: ", NCOLUMNAS)
    print("Dataframe labels: ", df.columns.to_numpy())
    print("Feature labels: ", df.columns[0 :NCOLUMNAS - 1].to_numpy())
    print("Data records in file: ", NDATA)
    print("Features: ", NFEATURES)
    print("Number of classes: ", NCLASSES)
    print("***************************************************************")

    y_tr = []
    for i in y_train:
        if i==0:
            y_tr.append([1.0,0.0]);
        if i==1:    # CUIDADO NO PONGAS ELSE!!!!
            y_tr.append([0.0,1.0]);
    X_tr = X_train.tolist()
    print("********************** NN model compilation ****************************")
    modelo_keras = Sequential()
    modelo_keras.add(Dense(NEURONS_HIDDEN_LAYER, input_shape=[NFEATURES], activation='relu'))
    modelo_keras.add(Dense(NEURONS_HIDDEN_LAYER, activation='relu'))
    modelo_keras.add(Dense(NCLASSES, activation='softmax'))
    modelo_keras.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])

    print("Compilation succesful. Model summary:")
    modelo_keras.summary()

    ### Training
    modelo_keras.fit(X_tr, y_tr, epochs=EPOCHS)
    print("End fit.");

    ### Guardando modelo h5
    print("\n********************** Writing NN model in file ************************");
    print("**** ", modelFilename);

    modelo_keras.save(modelFilename, include_optimizer=False)

    print(".Done! ");
    #############################################################################
    # Cargando modelo h5
    print("\n********************** Loading NN model from file ************************");
    print("**** ", modelFilename);
    modelo_keras = load_model(modelFilename)

    print("\n************************ Testing NN model ******************************")
    predicciones = modelo_keras.predict(X_test)
    aciertos = 0
    res = 0
    predict_y = []
    indx = 0
    filep = open("resModelB.txt", "a")
    for p in predicciones:
        if p[0] >= p[1]:
            res = 0
        else:
            res = 1
        predict_y.append(res)
        print("prediccion:", p[0], "  ", p[1])
        elemento = "elemento(class,pNN): " + str(y_test[indx]) + " " + str(res)
        print(elemento)
        filep.write("%s\n" % elemento)
        if y_test[indx] == res:
            aciertos = aciertos + 1
        indx = indx + 1
    filep.close()
    ratio = aciertos / len(predicciones)
    print("***************************************************************")
    print("Using model ", modelFilename, "over the dataset ", dataFilename, " Success ratio: ", ratio,
          " in ", len(predicciones), " processed.")
    print("***************************************************************")

    cm = confusion_matrix(y_test, predict_y)
    print('--------------------')
    print('| Confusion Matrix |')
    print('--------------------')
    print('\n {}'.format(cm))

    plt.figure(figsize=(7, 6))
    plt.rcParams.update({'font.size': 16})
    plt.grid(False)
    cm_cmap = plt.cm.Greens
    plot_confusion_matrix(cm, classes=np.unique(np_y), normalize=True, title='Normalized confusion matrix', cmap=cm_cmap)
    plt.savefig("confusion_matrix")
