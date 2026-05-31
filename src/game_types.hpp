#pragma once

#include "menu.hpp"

#include <pthread.h>
#include <semaphore.h>
#include <vector>

const int FILAS = 12;
const int COLUMNAS = 8;
const int META_PUNTAJE = 50;
const int VIDAS_INICIALES = 3;

const int TECLA_NADA = -1;
const int TECLA_ESC = 27;
const int TECLA_IZQUIERDA = 1001;
const int TECLA_DERECHA = 1002;
const int TECLA_ABAJO = 1003;
const int TECLA_ARRIBA = 1004;

const int CANTIDAD_HILOS = 5;

struct Pieza {
    int fila;
    int columna;
    int bloques[2][2];
};

struct EstadoJuego {
    std::vector<std::vector<int> > tablero;
    Pieza pieza;
    int puntaje;
    int vidas;
    int lineaTiempo;
    bool pausado;
    bool terminado;
    bool gano;
    bool salio;
};

struct DatosHilos {
    EstadoJuego* juego;
    GameMode mode;
    int delayCaida;
    int delayLinea;
    bool hayCambios;
    pthread_mutex_t mutex;
    pthread_cond_t condicionDibujo;
    pthread_barrier_t barreraInicio;
    sem_t semaforoEspecial;
};

struct ResultadoPartida {
    int puntaje;
    bool gano;
    bool salio;
};
