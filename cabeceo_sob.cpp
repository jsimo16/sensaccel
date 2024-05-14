#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <limits>
#include <cmath>
const int ESPECTRO_DIM = 10000;
const int NSAMPLES = 10000;
using namespace std;
int main() {
    ofstream aceleracion;
    ofstream velocidad;
    float acx[NSAMPLES];
    double vx[NSAMPLES-1];
    unsigned long xs[NSAMPLES];
    unsigned long xsv[NSAMPLES-1];
    vector<int> indices_sobreoscilacion_x;

    // Abrir archivos de aceleración y velocidad
    ifstream aceleracion_file("aceleracion1.txt");
    ifstream velocidad_file("velocidad1.txt");

    if (!aceleracion_file.is_open() || !velocidad_file.is_open()) {
        cerr << "Error al abrir los archivos de aceleración y velocidad." << endl;
    }

    float suma_aceleracion = 0.0f; // Variable para almacenar la suma de las aceleraciones
    // Leer valores de aceleración y almacenar en xs[] y acx[]
    for (int i = 0; i < NSAMPLES; ++i) {
        string line;
        if (getline(aceleracion_file, line)) {
            istringstream iss(line);
            if (!(iss >> xs[i])) {
                continue;
            }
            if (!(iss >> acx[i])) {
                continue;
            }
            suma_aceleracion += acx[i]; // Agregar el valor de aceleración a la suma
        } else {
            break;
        }
    }
    aceleracion_file.close();

    // Leer valores de velocidad y almacenar en vx[]
    for (int i = 0; i < NSAMPLES-1; ++i) {
        string line;
        if (getline(velocidad_file, line)) {
            istringstream iss(line);
            if (!(iss >> xsv[i])) {
                continue;
            }
            if (!(iss >> vx[i])) {
                continue;
            }
        } else {
            break;
        }
    }
    velocidad_file.close();

    // Calcular la aceleración media
    float aceleracion_media = suma_aceleracion / ESPECTRO_DIM;
    cout<<"Aceleracion media "<<aceleracion_media<<endl;
    for (int i = 0; i < ESPECTRO_DIM; ++i) {
        //Detectar sobreoscilación
        if (abs(acx[i]-aceleracion_media) > 0.5) {
            // Guardar el índice de la sobreoscilación
            if(indices_sobreoscilacion_x.empty() || (!indices_sobreoscilacion_x.empty() && xs[i]-xs[indices_sobreoscilacion_x.back()]>2000000)){  //Numero magico
                indices_sobreoscilacion_x.push_back(i);
                cout<<"posible sobreoscilacion en "<<xs[i]<<" "<<i+1<<endl;
            }
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////
    int delta = NSAMPLES/30; //jsimo: uso delta inferior (333 puntos aprox.)
    ofstream outputFile("diferencias_sigma.txt"); // Archivo de salida
    for(int i=0; i< indices_sobreoscilacion_x.size(); i++){
        // Calcular el intervalo alrededor del índice de sobreoscilación
        int lower_bound = indices_sobreoscilacion_x[i] -  delta/2; // Límite inferior del intervalo
        int upper_bound = indices_sobreoscilacion_x[i] +  delta/2; // Límite superior del intervalo
        // Asegurarse de que los límites están dentro de los límites del vector
        lower_bound = max(0, lower_bound); // No debe ser menor que 0
        upper_bound = min(NSAMPLES - 1, upper_bound); // No debe ser mayor que NSAMPLES - 1

        cout<<"********************************INTERVALO*************"<<endl;

        double vEntrada = vx[lower_bound]; //valor de velocidad en principio de intervalo
        double vSalida = vx[upper_bound];  //valor de velocidad en final de intervalo
        //////jsimo: archivos con fenomeno de cambio aislado
        ofstream fDifEntrada, fDifSalida;
        string fDifEntradaFilename = string("difentrada")+std::to_string(i)+string(".txt");
        string fDifSalidaFilename = string("difsalida")+std::to_string(i)+string(".txt");
        fDifEntrada.open(fDifEntradaFilename.c_str());
        fDifSalida.open(fDifSalidaFilename);
        //////////////////////////////////////////////////////////////////////////
        //// Calculo las diferencias con respecto al principio y fin del intervalo
        double vectorDifSalida[delta];
        int j = lower_bound;
        int k = 0;
        while((j+1)<=upper_bound){
            j++;
            k++;
            //////jsimo:
            double dvEntrada = (vx[j] - vEntrada);
            double dvSalida = (vx[j] - vSalida);
            vectorDifSalida[k] = dvSalida; // me guardo solo la difsalida para calcular la desviacion tipica
            //a archivo... para visualizar
            fDifEntrada << dvEntrada << endl;
            fDifSalida << dvSalida << endl;
            //////
        }
        fDifEntrada.close(); //jsimo:
        fDifSalida.close(); //jsimo:
        //////////////////////////////////////////////////////////////////////////
        //// Analisis simple de las diferencias con respecto final del intervalo
        //// solo vector de difsalida (vectorDifSalida)
        //Calculo media
        double mediaAntes = 0;
        double mediaLuego = 0;
        for (int ndx=0; ndx<delta; ndx++) {
            if (ndx < delta/2) {
                mediaAntes += vectorDifSalida[ndx];
            }
            if (ndx > delta/2) {
                mediaLuego += vectorDifSalida[ndx];
            }
        }
        mediaAntes /= (delta/2);
        mediaLuego /= (delta/2);
        cout<< "MediaAntes: " << mediaAntes << endl;
        cout<< "MediaLuego: " << mediaLuego << endl;
        //Calculo desviacion tipica
        double sigmaAntes = 0;
        double sigmaLuego = 0;
        for (int ndx=0; ndx<delta; ndx++) {
            if (ndx < delta/2) {
                sigmaAntes += (vectorDifSalida[ndx] - mediaAntes) * (vectorDifSalida[ndx] - mediaAntes);
            }
            if (ndx > delta/2) {
                sigmaLuego += (vectorDifSalida[ndx] - mediaLuego) * (vectorDifSalida[ndx] - mediaLuego);
            }
        }
        sigmaAntes = sqrt(sigmaAntes/(delta/2));
        sigmaLuego = sqrt(sigmaLuego/(delta/2));
        cout<< "SigmaAntes: " << sigmaAntes << endl;
        cout<< "SigmaLuego: " << sigmaLuego << endl;

        // Calcular la diferencia entre sigmaAntes y sigmaLuego
        double diferencia_sigma = sigmaLuego - sigmaAntes;

        // Escribir los datos en el archivo de salida
        outputFile << "Índice de sobreoscilación: " << xs[indices_sobreoscilacion_x[i]] << ", Diferencia entre sigmaAntes y sigmaLuego: " << diferencia_sigma << endl;
    }
    outputFile.close(); // Cerrar el archivo de salida
    return 0;
}
