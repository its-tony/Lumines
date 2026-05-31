#include "game_threads.hpp"
#include "display.hpp"
#include "game_logic.hpp"

#include <semaphore.h>
#include <unistd.h>


// hilos del juego: jugador, caida, linea de tiempo, puntaje y bloques especiales

void* hiloJugador(void* arg) {
    DatosHilos* datos = (DatosHilos*)arg;
    // espera a que todos los hilos esten listos
    pthread_barrier_wait(&datos->barreraInicio);

    while (!juegoTerminado(*datos)) {
        int tecla = leerTeclaJuego();
        bool cambio = false;

        pthread_mutex_lock(&datos->mutex);

        if (tecla == 'q' || tecla == 'Q' || tecla == TECLA_ESC) {
            datos->juego->terminado = true;
            datos->juego->salio = true;
            cambio = true;
        } else if (tecla == 'p' || tecla == 'P') {
            datos->juego->pausado = !datos->juego->pausado;
            cambio = true;
        } else if (!datos->juego->pausado) {
            if (tecla == TECLA_IZQUIERDA) {
                moverPieza(*datos->juego, -1);
                cambio = true;
            } else if (tecla == TECLA_DERECHA) {
                moverPieza(*datos->juego, 1);
                cambio = true;
            } else if (tecla == TECLA_ABAJO) {
                bajarPieza(*datos);
                cambio = true;
            } else if (tecla == 'z' || tecla == 'Z') {
                rotarPieza(*datos->juego, false);
                cambio = true;
            } else if (tecla == 'x' || tecla == 'X') {
                rotarPieza(*datos->juego, true);
                cambio = true;
            }
        }

        if (cambio) {
            avisarCambio(*datos);
        }

        pthread_mutex_unlock(&datos->mutex);
        usleep(25000);
    }

    return NULL;
}


void* hiloCaida(void* arg) {
    DatosHilos* datos = (DatosHilos*)arg;
    // este hilo baja la pieza cada cierto tiempo
    pthread_barrier_wait(&datos->barreraInicio);

    while (!esperarConSalida(*datos, datos->delayCaida)) {
        pthread_mutex_lock(&datos->mutex);

        if (!datos->juego->pausado && !datos->juego->terminado) {
            bajarPieza(*datos);
            avisarCambio(*datos);
        }

        pthread_mutex_unlock(&datos->mutex);
    }

    return NULL;
}

void* hiloLineaTiempo(void* arg) {
    DatosHilos* datos = (DatosHilos*)arg;
    // este hilo mueve la linea de tiempo
    pthread_barrier_wait(&datos->barreraInicio);

    while (!esperarConSalida(*datos, datos->delayLinea)) {
        pthread_mutex_lock(&datos->mutex);

        if (!datos->juego->pausado && !datos->juego->terminado) {
            revisarLineaDeTiempo(*datos->juego);
            avisarCambio(*datos);
        }

        pthread_mutex_unlock(&datos->mutex);
    }

    return NULL;
}

void* hiloPuntaje(void* arg) {
    DatosHilos* datos = (DatosHilos*)arg;
    // este hilo revisa si el jugador gana o pierde
    pthread_barrier_wait(&datos->barreraInicio);

    while (!esperarConSalida(*datos, 120)) {
        pthread_mutex_lock(&datos->mutex);

        if (datos->juego->puntaje >= META_PUNTAJE) {
            datos->juego->terminado = true;
            datos->juego->gano = true;
            avisarCambio(*datos);
        }

        if (datos->juego->vidas <= 0) {
            datos->juego->terminado = true;
            datos->juego->gano = false;
            avisarCambio(*datos);
        }

        pthread_mutex_unlock(&datos->mutex);
    }

    return NULL;
}

void* hiloBloquesEspeciales(void* arg) {
    DatosHilos* datos = (DatosHilos*)arg;
    // este hilo activa la posibilidad de bloques especiales
    pthread_barrier_wait(&datos->barreraInicio);

    int esperaEspecial = (datos->mode == GameMode::FAST) ? 6000 : 8000;

    while (!esperarConSalida(*datos, esperaEspecial)) {
        sem_post(&datos->semaforoEspecial);

        pthread_mutex_lock(&datos->mutex);
        avisarCambio(*datos);
        pthread_mutex_unlock(&datos->mutex);
    }

    return NULL;
}

void avisarCambio(DatosHilos& datos) {
    // despierta al hilo principal para redibujar la pantalla
    datos.hayCambios = true;
    pthread_cond_signal(&datos.condicionDibujo);
}
// funciones auxiliares para los hilos
bool juegoTerminado(DatosHilos& datos) {
    pthread_mutex_lock(&datos.mutex);
    bool terminado = datos.juego->terminado;
    pthread_mutex_unlock(&datos.mutex);
    return terminado;
}

bool esperarConSalida(DatosHilos& datos, int milisegundos) {
    // espera en partes pequenas para que los hilos puedan cerrar rapido
    int tiempoPasado = 0;

    while (tiempoPasado < milisegundos) {
        if (juegoTerminado(datos)) {
            return true;
        }

        usleep(50000);
        tiempoPasado += 50;
    }

    return juegoTerminado(datos);
}

int leerTeclaJuego() {
    int tecla = Display::getChNonBlocking();

    if (tecla == -1) {
        return TECLA_NADA;
    }

    if (tecla == TECLA_ESC) {
        // las flechas llegan como una secuencia que empieza con esc
        usleep(1000);
        int segundo = Display::getChNonBlocking();

        if (segundo == '[') {
            int tercero = Display::getChNonBlocking();

            if (tercero == 'D') return TECLA_IZQUIERDA;
            if (tercero == 'C') return TECLA_DERECHA;
            if (tercero == 'B') return TECLA_ABAJO;
            if (tercero == 'A') return TECLA_ARRIBA;
        }

        return TECLA_ESC;
    }

    return tecla;
}
