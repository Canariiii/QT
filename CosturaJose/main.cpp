#include <QCoreApplication>
#include <QtCore>
#include <iostream>

constexpr int NumMaxMangas = 100;
constexpr int NumMaxCuerpos = 50;

QSemaphore semMangas(0);
QSemaphore semCuerpos(0);
QSemaphore semEnsamblados(0);
QSemaphore sacarPantalla(1);

int contadorCamisas = 0;

QMutex mutex;

class CostureraManga : public QThread
{
public:
    void run() override {
        for (int i = 0; i < NumMaxMangas; ++i) {
            semMangas.release();
            sacarPantalla.acquire();
            {
                QMutexLocker locker(&mutex);
                std::cout << "Se creó una manga. Quedan " << semMangas.available() << " mangas en el cesto." << std::endl;
            }
            sacarPantalla.release();
        }
    }
};

class CostureraCuerpo : public QThread
{
public:
    void run() override {
        for (int i = 0; i < NumMaxCuerpos; ++i) {
            semCuerpos.release();
            sacarPantalla.acquire();
            std::cout << "Se creó un cuerpo. Quedan " << semCuerpos.available() << " cuerpos en el cesto." << std::endl;
            sacarPantalla.release();
        }
    }
};

class CostureraEnsambladora : public QThread
{
public:
    void run() override {
        while (true) {
            if (semMangas.available() >= 2 && semCuerpos.available() >= 1) {
                semMangas.acquire(2);
                semCuerpos.acquire();
                sacarPantalla.acquire();
                std::cout << "El ensamblador creó una camisa. Quedan " << semMangas.available()
                          << " mangas en el cesto y " << semCuerpos.available() << " cuerpos en el cesto." << std::endl;
                sacarPantalla.release();
                ++contadorCamisas;
                semEnsamblados.release();
            } else {
                break;
            }
        }
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    CostureraManga mangas;
    CostureraCuerpo cuerpos;
    CostureraEnsambladora ensamblador;

    mangas.start();
    cuerpos.start();
    ensamblador.start();

    mangas.wait();
    cuerpos.wait();
    ensamblador.wait();

    std::cout << "Recuento total de camisas: " << contadorCamisas << std::endl;

    return a.exec();
}
