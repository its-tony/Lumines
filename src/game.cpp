#include "game.hpp"
#include "display.hpp"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;
using namespace Display;

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

struct Pieza {
    int fila;
    int columna;
    int bloques[2][2];
};

struct EstadoJuego {
    vector<vector<int> > tablero;
    Pieza pieza;
    int puntaje;
    int vidas;
    int lineaTiempo;
    bool pausado;
    bool terminado;
    bool gano;
};

int leerTeclaJuego();
void dibujarJuego(const EstadoJuego& juego, GameMode mode);
void prepararNuevaPieza(EstadoJuego& juego);
bool puedeMover(const EstadoJuego& juego, const Pieza& pieza, int nuevaFila, int nuevaColumna);
void moverPieza(EstadoJuego& juego, int cambioColumna);
void bajarPieza(EstadoJuego& juego);
void rotarPieza(EstadoJuego& juego, bool horario);
void fijarPieza(EstadoJuego& juego);
void revisarLineaDeTiempo(EstadoJuego& juego);
void aplicarBloqueEspecial(EstadoJuego& juego, int fila, int columna);
string celdaComoTexto(int valor);
char simboloBloque(int valor);
string pedirNombreJugador();

void runGame(GameMode mode, ScoreManager& scores) {
    srand((unsigned int)time(NULL));

    EstadoJuego juego;
    juego.tablero = vector<vector<int> >(FILAS, vector<int>(COLUMNAS, 0));
    juego.puntaje = 0;
    juego.vidas = VIDAS_INICIALES;
    juego.lineaTiempo = 0;
    juego.pausado = false;
    juego.terminado = false;
    juego.gano = false;

    prepararNuevaPieza(juego);

    int delayCaida = (mode == GameMode::FAST) ? 350 : 700;
    int delayLinea = (mode == GameMode::FAST) ? 280 : 500;

    auto siguienteCaida = chrono::steady_clock::now() + chrono::milliseconds(delayCaida);
    auto siguienteLinea = chrono::steady_clock::now() + chrono::milliseconds(delayLinea);

    hideCursor();
    setRawMode();
    usleep(80000);
    flushInput();

    while (!juego.terminado) {
        int tecla = leerTeclaJuego();

        if (tecla == 'q' || tecla == 'Q' || tecla == TECLA_ESC) {
            juego.terminado = true;
        } else if (tecla == 'p' || tecla == 'P') {
            juego.pausado = !juego.pausado;
        } else if (!juego.pausado) {
            if (tecla == TECLA_IZQUIERDA) {
                moverPieza(juego, -1);
            } else if (tecla == TECLA_DERECHA) {
                moverPieza(juego, 1);
            } else if (tecla == TECLA_ABAJO) {
                bajarPieza(juego);
            } else if (tecla == 'z' || tecla == 'Z') {
                rotarPieza(juego, false);
            } else if (tecla == 'x' || tecla == 'X') {
                rotarPieza(juego, true);
            }
        }

        auto ahora = chrono::steady_clock::now();

        if (!juego.pausado && ahora >= siguienteCaida) {
            bajarPieza(juego);
            siguienteCaida = ahora + chrono::milliseconds(delayCaida);
        }

        if (!juego.pausado && ahora >= siguienteLinea) {
            revisarLineaDeTiempo(juego);
            siguienteLinea = ahora + chrono::milliseconds(delayLinea);
        }

        if (juego.puntaje >= META_PUNTAJE) {
            juego.terminado = true;
            juego.gano = true;
        }

        if (juego.vidas <= 0) {
            juego.terminado = true;
            juego.gano = false;
        }

        dibujarJuego(juego, mode);
        usleep(35000);
    }

    restoreMode();
    showCursor();

    moveTo(4, 22);
    cout << "partida finalizada con " << juego.puntaje << " puntos" << endl;

    if (juego.puntaje > 0) {
        string nombre = pedirNombreJugador();
        scores.addScore(nombre, juego.puntaje);
        scores.saveToFile();
        cout << "puntaje guardado" << endl;
    }

    cout << "presiona enter para volver al menu";
    string pausaFinal;
    getline(cin, pausaFinal);
}

int leerTeclaJuego() {
    int tecla = getChNonBlocking();

    if (tecla == -1) {
        return TECLA_NADA;
    }

    if (tecla == TECLA_ESC) {
        usleep(1000);
        int segundo = getChNonBlocking();

        if (segundo == '[') {
            int tercero = getChNonBlocking();

            if (tercero == 'D') return TECLA_IZQUIERDA;
            if (tercero == 'C') return TECLA_DERECHA;
            if (tercero == 'B') return TECLA_ABAJO;
            if (tercero == 'A') return TECLA_ARRIBA;
        }

        return TECLA_ESC;
    }

    return tecla;
}

void dibujarJuego(const EstadoJuego& juego, GameMode mode) {
    // dibujo principal del tablero y la informacion
    clear();
    drawBox(1, 1, WIDTH, HEIGHT);

    string modoTexto = (mode == GameMode::FAST) ? "rapido" : "lento";

    moveTo(4, 2);
    cout << "lumines fase 2";

    moveTo(4, 4);
    cout << "modo: " << modoTexto;

    moveTo(20, 4);
    cout << "puntaje: " << juego.puntaje << "/" << META_PUNTAJE;

    moveTo(42, 4);
    cout << "vidas: " << juego.vidas;

    int inicioCol = 4;
    int inicioFila = 6;

    moveTo(inicioCol, inicioFila);
    cout << "+";
    for (int c = 0; c < COLUMNAS * 3; c++) {
        cout << "-";
    }
    cout << "+";

    for (int f = 0; f < FILAS; f++) {
        moveTo(inicioCol, inicioFila + f + 1);
        cout << "|";

        for (int c = 0; c < COLUMNAS; c++) {
            int valor = juego.tablero[f][c];

            for (int pf = 0; pf < 2; pf++) {
                for (int pc = 0; pc < 2; pc++) {
                    int filaPieza = juego.pieza.fila + pf;
                    int columnaPieza = juego.pieza.columna + pc;

                    if (filaPieza == f && columnaPieza == c && juego.pieza.bloques[pf][pc] != 0) {
                        valor = juego.pieza.bloques[pf][pc];
                    }
                }
            }

            if (c == juego.lineaTiempo) {
                if (valor == 0) {
                    cout << " | ";
                } else {
                    cout << "|" << simboloBloque(valor) << "|";
                }
            } else {
                cout << celdaComoTexto(valor);
            }
        }

        cout << "|";
    }

    moveTo(inicioCol, inicioFila + FILAS + 1);
    cout << "+";
    for (int c = 0; c < COLUMNAS * 3; c++) {
        cout << "-";
    }
    cout << "+";

    moveTo(36, 7);
    cout << "elementos:";
    moveTo(36, 9);
    cout << "[#] color 1";
    moveTo(36, 10);
    cout << "[O] color 2";
    moveTo(36, 11);
    cout << "[*] especial";
    moveTo(36, 12);
    cout << "|   linea de tiempo";

    moveTo(36, 15);
    cout << "controles:";
    moveTo(36, 16);
    cout << "flechas mover";
    moveTo(36, 17);
    cout << "z y x para rotar";
    moveTo(36, 18);
    cout << "p pausar";
    moveTo(36, 19);
    cout << "q salir";

    if (juego.pausado) {
        moveTo(4, 21);
        cout << "juego pausado";
    }

    cout.flush();
}

void prepararNuevaPieza(EstadoJuego& juego) {
    // crea una pieza de 2x2 con dos colores posibles
    juego.pieza.fila = 0;
    juego.pieza.columna = COLUMNAS / 2 - 1;

    for (int f = 0; f < 2; f++) {
        for (int c = 0; c < 2; c++) {
            juego.pieza.bloques[f][c] = rand() % 2 + 1;
        }
    }

    if (rand() % 8 == 0) {
        int filaEspecial = rand() % 2;
        int columnaEspecial = rand() % 2;
        juego.pieza.bloques[filaEspecial][columnaEspecial] = 3;
    }

    if (!puedeMover(juego, juego.pieza, juego.pieza.fila, juego.pieza.columna)) {
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

void bajarPieza(EstadoJuego& juego) {
    int nuevaFila = juego.pieza.fila + 1;

    if (puedeMover(juego, juego.pieza, nuevaFila, juego.pieza.columna)) {
        juego.pieza.fila = nuevaFila;
    } else {
        fijarPieza(juego);
        prepararNuevaPieza(juego);
    }
}

void rotarPieza(EstadoJuego& juego, bool horario) {
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

string celdaComoTexto(int valor) {
    if (valor == 1) return "[#]";
    if (valor == 2) return "[O]";
    if (valor == 3) return "[*]";
    return "   ";
}

char simboloBloque(int valor) {
    if (valor == 1) return '#';
    if (valor == 2) return 'O';
    if (valor == 3) return '*';
    return ' ';
}

string pedirNombreJugador() {
    string nombre;

    cout << "ingresa tu nombre sin espacios: ";
    getline(cin, nombre);

    if (nombre == "") {
        nombre = "jugador";
    }

    for (int i = 0; i < (int)nombre.size(); i++) {
        if (nombre[i] == ' ') {
            nombre[i] = '_';
        }
    }

    return nombre;
}
