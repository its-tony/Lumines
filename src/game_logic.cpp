#include "game_logic.hpp"

#include <cstdlib>
#include <semaphore.h>

void prepararNuevaPieza(DatosHilos& datos) {
    // crea una pieza de 2x2 con dos colores posibles
    EstadoJuego& juego = *datos.juego;

    juego.pieza.fila = 0;
    juego.pieza.columna = COLUMNAS / 2 - 1;

    for (int f = 0; f < 2; f++) {
        for (int c = 0; c < 2; c++) {
            juego.pieza.bloques[f][c] = rand() % 2 + 1;
        }
    }

    if (sem_trywait(&datos.semaforoEspecial) == 0 || rand() % 10 == 0) {
        // el semaforo permite que aparezca un bloque especial
        int filaEspecial = rand() % 2;
        int columnaEspecial = rand() % 2;
        juego.pieza.bloques[filaEspecial][columnaEspecial] = 3;
    }

    if (!puedeMover(juego, juego.pieza, juego.pieza.fila, juego.pieza.columna)) {
        // si la pieza ya no cabe arriba se pierde una vida
        juego.vidas--;

        for (int f = 0; f < 3 && f < FILAS; f++) {
            for (int c = 0; c < COLUMNAS; c++) {
                juego.tablero[f][c] = 0;
            }
        }
    }
}

bool puedeMover(const EstadoJuego& juego, const Pieza& pieza, int nuevaFila, int nuevaColumna) {
    // valida limites del tablero y choque con bloques fijos
    for (int f = 0; f < 2; f++) {
        for (int c = 0; c < 2; c++) {
            if (pieza.bloques[f][c] != 0) {
                int filaFinal = nuevaFila + f;
                int columnaFinal = nuevaColumna + c;

                if (filaFinal < 0 || filaFinal >= FILAS) {
                    return false;
                }

                if (columnaFinal < 0 || columnaFinal >= COLUMNAS) {
                    return false;
                }

                if (juego.tablero[filaFinal][columnaFinal] != 0) {
                    return false;
                }
            }
        }
    }

    return true;
}

void moverPieza(EstadoJuego& juego, int cambioColumna) {
    int nuevaColumna = juego.pieza.columna + cambioColumna;

    if (puedeMover(juego, juego.pieza, juego.pieza.fila, nuevaColumna)) {
        juego.pieza.columna = nuevaColumna;
    }
}

void bajarPieza(DatosHilos& datos) {
    EstadoJuego& juego = *datos.juego;
    int nuevaFila = juego.pieza.fila + 1;

    if (puedeMover(juego, juego.pieza, nuevaFila, juego.pieza.columna)) {
        juego.pieza.fila = nuevaFila;
    } else {
        fijarPieza(juego);
        prepararNuevaPieza(datos);
    }
}

void rotarPieza(EstadoJuego& juego, bool horario) {
    // se rota una copia y solo se aplica si cabe en el tablero
    Pieza copia = juego.pieza;

    if (horario) {
        copia.bloques[0][0] = juego.pieza.bloques[1][0];
        copia.bloques[0][1] = juego.pieza.bloques[0][0];
        copia.bloques[1][0] = juego.pieza.bloques[1][1];
        copia.bloques[1][1] = juego.pieza.bloques[0][1];
    } else {
        copia.bloques[0][0] = juego.pieza.bloques[0][1];
        copia.bloques[0][1] = juego.pieza.bloques[1][1];
        copia.bloques[1][0] = juego.pieza.bloques[0][0];
        copia.bloques[1][1] = juego.pieza.bloques[1][0];
    }

    if (puedeMover(juego, copia, copia.fila, copia.columna)) {
        juego.pieza = copia;
    }
}

void fijarPieza(EstadoJuego& juego) {
    // fija la pieza activa en el tablero
    for (int f = 0; f < 2; f++) {
        for (int c = 0; c < 2; c++) {
            int filaFinal = juego.pieza.fila + f;
            int columnaFinal = juego.pieza.columna + c;

            if (filaFinal >= 0 && filaFinal < FILAS && columnaFinal >= 0 && columnaFinal < COLUMNAS) {
                juego.tablero[filaFinal][columnaFinal] = juego.pieza.bloques[f][c];
            }
        }
    }
}

void revisarLineaDeTiempo(EstadoJuego& juego) {
    // la linea de tiempo elimina cuadrados cuando pasa sobre ellos
    juego.lineaTiempo++;

    if (juego.lineaTiempo >= COLUMNAS) {
        juego.lineaTiempo = 0;
    }

    int columna = juego.lineaTiempo;
    int eliminados = 0;

    for (int f = 0; f < FILAS; f++) {
        if (juego.tablero[f][columna] == 3) {
            aplicarBloqueEspecial(juego, f, columna);
            eliminados += 3;
        }
    }

    for (int f = 0; f < FILAS - 1; f++) {
        for (int c = 0; c < COLUMNAS - 1; c++) {
            int color = juego.tablero[f][c];

            if (color == 1 || color == 2) {
                bool cuadrado =
                    juego.tablero[f][c + 1] == color &&
                    juego.tablero[f + 1][c] == color &&
                    juego.tablero[f + 1][c + 1] == color;

                bool lineaPaso = columna == c || columna == c + 1;

                if (cuadrado && lineaPaso) {
                    juego.tablero[f][c] = 0;
                    juego.tablero[f][c + 1] = 0;
                    juego.tablero[f + 1][c] = 0;
                    juego.tablero[f + 1][c + 1] = 0;
                    eliminados += 4;
                }
            }
        }
    }

    if (eliminados > 0) {
        juego.puntaje += eliminados * 2;

        if (eliminados >= 8) {
            juego.puntaje += 5;
        }
    }
}

void aplicarBloqueEspecial(EstadoJuego& juego, int fila, int columna) {
    // el bloque especial limpia una zona pequena alrededor
    for (int f = fila - 1; f <= fila + 1; f++) {
        for (int c = columna - 1; c <= columna + 1; c++) {
            if (f >= 0 && f < FILAS && c >= 0 && c < COLUMNAS) {
                juego.tablero[f][c] = 0;
            }
        }
    }
}
