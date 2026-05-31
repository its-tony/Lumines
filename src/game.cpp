// universidad del valle de guatemala
// cc3086 - programacion de microprocesadores
// proyecto 2 - lumines en consola
// fase 3 - programacion concurrente
//
// integrantes:
// antony portillo 25615
// sergio lopez 25848
// daniel vasquez 25582
// jordi cardona 251142

#include "game.hpp"
#include "display.hpp"
#include "game_logic.hpp"
#include "game_screen.hpp"
#include "game_threads.hpp"

#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <vector>

using namespace std;

ResultadoPartida jugarPartida(GameMode mode);
// muestra la pantalla final con el resultado de la partida y opciones para reiniciar o volver al menu :D
void runGame(GameMode mode, ScoreManager& scores) {
    srand((unsigned int)time(NULL));

    bool reiniciar = true;

    while (reiniciar) {
        ResultadoPartida resultado = jugarPartida(mode);
        reiniciar = mostrarPantallaFinal(resultado, scores);
    }
}

ResultadoPartida jugarPartida(GameMode mode) {
    // estado inicial de una partida
    EstadoJuego juego;
    juego.tablero = vector<vector<int> >(FILAS, vector<int>(COLUMNAS, 0));
    juego.puntaje = 0;
    juego.vidas = VIDAS_INICIALES;
    juego.lineaTiempo = 0;
    juego.pausado = false;
    juego.terminado = false;
    juego.gano = false;
    juego.salio = false;

    // datos compartidos entre los hilos
    DatosHilos datos;
    datos.juego = &juego;
    datos.mode = mode;
    datos.delayCaida = (mode == GameMode::FAST) ? 350 : 700;
    datos.delayLinea = (mode == GameMode::FAST) ? 280 : 500;
    datos.hayCambios = true;

    // se preparan los mecanismos de sincronizacion
    pthread_mutex_init(&datos.mutex, NULL);
    pthread_cond_init(&datos.condicionDibujo, NULL);
    pthread_barrier_init(&datos.barreraInicio, NULL, CANTIDAD_HILOS);
    sem_init(&datos.semaforoEspecial, 0, 0);

    // la primera pieza se crea con el mutex tomado
    pthread_mutex_lock(&datos.mutex);
    prepararNuevaPieza(datos);
    pthread_mutex_unlock(&datos.mutex);

    pthread_t jugador;
    pthread_t caida;
    pthread_t linea;
    pthread_t puntaje;
    pthread_t especiales;

    Display::hideCursor();
    Display::setRawMode();
    usleep(80000);
    Display::flushInput();

    // cada hilo se encarga de una parte distinta del juego
    pthread_create(&jugador, NULL, hiloJugador, &datos);
    pthread_create(&caida, NULL, hiloCaida, &datos);
    pthread_create(&linea, NULL, hiloLineaTiempo, &datos);
    pthread_create(&puntaje, NULL, hiloPuntaje, &datos);
    pthread_create(&especiales, NULL, hiloBloquesEspeciales, &datos);

    while (true) {
        pthread_mutex_lock(&datos.mutex);

        // espera hasta que algun hilo avise que hay cambios
        while (!datos.hayCambios && !juego.terminado) {
            pthread_cond_wait(&datos.condicionDibujo, &datos.mutex);
        }

        // se copia el estado para dibujar sin bloquear a los hilos
        EstadoJuego copia = juego;
        datos.hayCambios = false;

        pthread_mutex_unlock(&datos.mutex);

        dibujarJuego(copia, mode);

        if (copia.terminado) {
            break;
        }
    }

    // se espera a que todos los hilos terminen antes de cerrar la partida
    pthread_join(jugador, NULL);
    pthread_join(caida, NULL);
    pthread_join(linea, NULL);
    pthread_join(puntaje, NULL);
    pthread_join(especiales, NULL);

    Display::restoreMode();
    Display::showCursor();
    Display::flushInput();

    ResultadoPartida resultado;
    resultado.puntaje = juego.puntaje;
    resultado.gano = juego.gano;
    resultado.salio = juego.salio;

    // se liberan los recursos usados para sincronizacion
    sem_destroy(&datos.semaforoEspecial);
    pthread_barrier_destroy(&datos.barreraInicio);
    pthread_cond_destroy(&datos.condicionDibujo);
    pthread_mutex_destroy(&datos.mutex);

    return resultado;
}
