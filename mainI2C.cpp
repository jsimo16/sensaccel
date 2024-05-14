#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include "MPU9250Lite.h"

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cmath>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
// #include <chrono>

const int NSAMPLES = 10000;
const int ESPECTRO_DIM = 10000;

using namespace std;
using namespace BBB;

struct Buffer {
    unsigned long data_buffer_t0[NSAMPLES];
    unsigned long data_buffer_t1[NSAMPLES];
    float data_buffer_value_X[NSAMPLES];
    float data_buffer_value_Y[NSAMPLES];
    float data_buffer_value_Z[NSAMPLES];
};

class BufferSet {
	public:
		Buffer buffer1;
		Buffer buffer2;
		Buffer buffer3;
		bool datosListos = false;
		std::mutex mtx;
		std::condition_variable cond;
};

long getCurrentMicroseconds() {
	struct timespec currentTime;
	clock_gettime(CLOCK_MONOTONIC, &currentTime);
	return (currentTime.tv_sec)*1000000 + (currentTime.tv_nsec) / 1000;
}

/**
* Adds "delay" microseconds to timespecs and sleeps until that new time
* This function is intended to implement periodic processes with absolute
* activation times.
*/
void sleep_until(struct timespec *ts, int delay)
{
	long oneSecond = 1000*1000*1000;
	ts->tv_nsec += delay * 1000;
	if(ts->tv_nsec >= oneSecond){
		ts->tv_nsec -= oneSecond;
		ts->tv_sec++;
	}
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, ts,  NULL);
}

void setRealTimeCurrentThread()
{
  // If pid equals zero, the scheduling policy and attributes
  // of the calling thread will be set.
  int my_pid = 0;
  struct sched_param param;
  int retval;
  int low_priority, high_priority;

  // Get min and max priority values of SCHED_FIFO policy
  high_priority = sched_get_priority_max(SCHED_FIFO);
  if (high_priority == -1) {
  	perror("Error in sched_get_priority_max"); _exit(-1);
  }
  low_priority = sched_get_priority_min(SCHED_FIFO);
  if (low_priority == -1) {
  	perror("Error in sched_get_priority_min"); _exit(-1);
  }

  // Change to SCHED_FIFO and medium priority.
  param.sched_priority = 50; //(high_priority + low_priority) / 2;
  retval = sched_setscheduler(my_pid, SCHED_FIFO, &param);
  if (retval == -1) {
  	perror("Error in sched_setscheduler"); _exit(-1);
  }
} 

void calibrateAccelOffsets(BBB::MPU9250Lite *theIMU) {
	int i = 0;
	int Nsamples = 100;
	float sumAccX = 0;
	float sumAccY = 0;
	float sumAccZ = 0;
	for (i=0;i<Nsamples;i++) {
		printf(".");
		int16_t acx, acy, acz;
		theIMU->getAccelerationRAW(&acx,&acy,&acz);
		sumAccX += acx;
		sumAccY += acy;
		sumAccZ += acz;
		if (i%10==0) {
			printf("\n");
		}
		printf(".");
		usleep(100000);
	}
	float offsetAccX = sumAccX / Nsamples;
	float offsetAccY = sumAccY / Nsamples;
	float offsetAccZ = sumAccZ / Nsamples;
	if (offsetAccZ > 0) {
		offsetAccZ -= 65536/4;
	} else {
		offsetAccZ += 65536/4;
	}
	printf("\nOffsets:  X:%f  Y:%f  Z:%f\n",offsetAccX,offsetAccY,offsetAccZ);
	theIMU->setAccelOffsets(offsetAccX,offsetAccY,offsetAccZ);
}

void sensorDataProducer(BufferSet& buffers, BBB::MPU9250Lite& imu) {
    setRealTimeCurrentThread();
    struct timespec ts;
    while(true){
        clock_gettime(CLOCK_MONOTONIC, &ts);
        int i = 0;        
        while (i < NSAMPLES) {
            unsigned long t1 = getCurrentMicroseconds();
            float accx, accy, accz;
            imu.getAccelerationXYZ(&accx, &accy, &accz);
            unsigned long t2 = getCurrentMicroseconds();
            buffers.buffer1.data_buffer_t0[i] = t1;
            buffers.buffer1.data_buffer_t1[i] = t2;
            buffers.buffer1.data_buffer_value_X[i] = accx;
            buffers.buffer1.data_buffer_value_Y[i] = accy;
            buffers.buffer1.data_buffer_value_Z[i] = accz;

            i++;
            sleep_until(&ts, 1000);
        }
            {
                std::unique_lock<std::mutex> lock(buffers.mtx);
                Buffer temp = buffers.buffer1;
                buffers.buffer1 = buffers.buffer2;
                buffers.buffer2 = temp;
                buffers.datosListos = true;
            }
            buffers.cond.notify_all();
           
        }
}


void sensorDataConsumer(BufferSet& buffers, std::vector<std::vector<double>>& cos_table, std::vector<std::vector<double>>& sin_table, int tipoFallo) {
    int TF = tipoFallo;
    int count = 0;
    double velocity_x = 0; // Supongo velocidad inicial cero.
    double velocity_y = 0;
    ofstream aceleracion;
    ofstream velocidad;
    ofstream modulos;
    ofstream espectro;
    unsigned long t1, t2;
    float accx, accy, accz;
    double xEspectroX[ESPECTRO_DIM-1];
    double xEspectroY[ESPECTRO_DIM-1];
    double yEspectroX[ESPECTRO_DIM-1];
    double yEspectroY[ESPECTRO_DIM-1];
    double zEspectroX[ESPECTRO_DIM-1];
    double zEspectroY[ESPECTRO_DIM-1];
    float acx[ESPECTRO_DIM-1];
    float acy[ESPECTRO_DIM-1];
    double vx[ESPECTRO_DIM-1];
    double vy[ESPECTRO_DIM-1];
    unsigned long xs[ESPECTRO_DIM-1];
    double mx, my, mz;
	while(true){
        unique_lock<std::mutex> lock(buffers.mtx);
        buffers.cond.wait(lock, [&]() { return buffers.datosListos; });
        Buffer temp = buffers.buffer2;
        buffers.buffer2 = buffers.buffer3;
        buffers.buffer3 = temp;
        lock.unlock();
        buffers.datosListos = false;
        if (ESPECTRO_DIM > 0) {
            auto t = time(nullptr);
            auto tm = *localtime(&t);
            ostringstream oss;
            oss << put_time(&tm, "%d%m%Y%H%M%S");
            auto date = oss.str();
            modulos.open ("E"+date+"modulos.txt");        
            espectro.open ("E"+date+"espectro.txt");
            aceleracion.open("E"+date+"aceleracion.txt");
            velocidad.open("E"+date+"velocidad.txt");
            for(int i=1; i<ESPECTRO_DIM; i++) {
                xEspectroX[i]=0; xEspectroY[i]=0;
                yEspectroX[i]=0; yEspectroY[i]=0;
                zEspectroX[i]=0; zEspectroY[i]=0;
                for (int j=0; j<NSAMPLES; j++) {
                        xEspectroX[i]+=buffers.buffer3.data_buffer_value_X[j] * cos_table[i][j];
                        xEspectroY[i]+=(-1) * buffers.buffer3.data_buffer_value_X[j] * sin_table[i][j];
                        yEspectroX[i]+=buffers.buffer3.data_buffer_value_Y[j] * cos_table[i][j];
                        yEspectroY[i]+=(-1) * buffers.buffer3.data_buffer_value_Y[j] * sin_table[i][j];
                        zEspectroX[i]+=buffers.buffer3.data_buffer_value_Z[j] * cos_table[i][j];
                        zEspectroY[i]+=(-1) * buffers.buffer3.data_buffer_value_Z[j] * sin_table[i][j];
                }
                xEspectroX[i]=xEspectroX[i]/NSAMPLES; ///modulo_max;
                xEspectroY[i]=xEspectroY[i]/NSAMPLES; ///modulo_max;
                //
                yEspectroX[i]=yEspectroX[i]/NSAMPLES;
                yEspectroY[i]=yEspectroY[i]/NSAMPLES;
                //
                zEspectroX[i]=zEspectroX[i]/NSAMPLES;
                zEspectroY[i]=zEspectroY[i]/NSAMPLES;
                
                mx = sqrt(xEspectroX[i]*xEspectroX[i] + xEspectroY[i]*xEspectroY[i]);
                my = sqrt(yEspectroX[i]*yEspectroX[i] + yEspectroY[i]*yEspectroY[i]);
                mz = sqrt(zEspectroX[i]*zEspectroX[i] + zEspectroY[i]*zEspectroY[i]);
                
                modulos << mx << " " << my << " " << mz << " "<< TF << endl;
                espectro << xEspectroX[i] << " " << xEspectroY[i] << " " << yEspectroX[i] << " " << yEspectroY[i] << " "<< zEspectroX[i] << " " << zEspectroY[i] << " "<< TF << endl;
                
                t1 = buffers.buffer3.data_buffer_t0[i];
                xs[i] = t1;
                t2 = buffers.buffer3.data_buffer_t1[i];
                accx = buffers.buffer3.data_buffer_value_X[i];
                acx[i] = accx;
                accy = buffers.buffer3.data_buffer_value_Y[i];
                acy[i] = accy;
                accz = buffers.buffer3.data_buffer_value_Z[i];
                aceleracion << t1 << " " << t2 << " " << accx << " " << accy << " "<< accz << " "<< TF << endl;
                
                velocity_x += 9.81 * accx * (t1 - buffers.buffer3.data_buffer_t0[i-1]) / 1000000.0; // Si multipicamos por 9.81, el valor saldrá en m/s.
                vx[i] = velocity_x; 
                velocity_y += 9.81 * accy * (t1 - buffers.buffer3.data_buffer_t0[i-1]) / 1000000.0;
                vy[i] = velocity_y;
                velocidad << t1 << " " << t2 << " " << velocity_x << " " << velocity_y << " "<< TF << endl;
                
            }
            modulos.close();
            espectro.close();
            aceleracion.close();
            velocidad.close();
        }
        
    }
}

void lookuptable(std::vector<std::vector<double>>& cos_table, std::vector<std::vector<double>>& sin_table) {
	const double PI=3.141592653589793;
    for (int i = 0; i < ESPECTRO_DIM; ++i) {
        for (int j = 0; j < NSAMPLES; ++j) {
            double angle = 2 * PI / NSAMPLES * i * j;
			cos_table[i][j] = cos(angle);
            sin_table[i][j] = sin(angle);
        }
    } 
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{	
    // Procesar los argumentos de la línea de comandos usando getopt
    int opt;
    int tipoFallo=0;
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            tipoFallo = atoi(optarg); // Convertir el argumento a entero y almacenarlo
            break;
        default:
            fprintf(stderr, "Uso: %s [-t tipo_fallo]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
	BBB::I2C i2cBus(4);
    BBB::MPU9250Lite imu(&i2cBus, 0x68);
    BufferSet buffers;

    cout << "Get Connection: 0x" << hex << (int)imu.getConnection() << endl;
    imu.initialize();
    calibrateAccelOffsets(&imu);
    std::vector<std::vector<double>> cos_table(ESPECTRO_DIM, std::vector<double>(NSAMPLES));
    std::vector<std::vector<double>> sin_table(ESPECTRO_DIM, std::vector<double>(NSAMPLES));

    lookuptable(cos_table, sin_table);
    
    std::thread sensorPThread(sensorDataProducer, std::ref(buffers), std::ref(imu));
	std::thread sensorCThread(sensorDataConsumer, std::ref(buffers), std::ref(cos_table), std::ref(sin_table), tipoFallo);
    sensorPThread.join();
	sensorCThread.join();

    cout << "IMU sampling finished" << endl;

    return 0;
}
