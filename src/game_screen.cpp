#include "game_screen.hpp"
#include "display.hpp"

#include <iostream>
#include <string>

using namespace std;
using namespace Display;

int columnaCentrada(int ancho, const string& texto);
void escribirCentrado(int fila, int ancho, const string& texto);
void escribirCentradoColor(int fila, int ancho, const string& color, const string& texto);
bool pantallaMuyPequena(int ancho, int alto);
void mostrarAvisoPantallaPequena(int ancho, int alto);
string colorBloque(int valor);

void dibujarJuego(const EstadoJuego& juego, GameMode mode) {
    // dibujo principal del tablero y la informacion
    int ancho = WIDTH;
    int alto = HEIGHT;
    getTerminalSize(ancho, alto);

    // si la terminal es muy pequena se muestra un aviso simple
    if (pantallaMuyPequena(ancho, alto)) {
        mostrarAvisoPantallaPequena(ancho, alto);
        return;
    }

    clear();
    drawBox(1, 1, ancho, alto);

    string modoTexto = (mode == GameMode::FAST) ? "rapido" : "lento";
// se calculan las posiciones para centrar el tablero y el panel de informacion
    int tableroAncho = COLUMNAS * 3 + 2;
    int tableroAlto = FILAS + 2;
    int panelAncho = 34;
    int espacio = 5;
    int contenidoAncho = tableroAncho + espacio + panelAncho;
    int inicioCol = (ancho - contenidoAncho) / 2 + 1;
    int inicioFila = (alto - tableroAlto) / 2 + 1;

    // se ajusta la posicion para que no se salga de la pantalla
    if (inicioCol < 3) {
        inicioCol = 3;
    }

    if (inicioFila < 6) {
        inicioFila = 6;
    }

    if (inicioFila + tableroAlto + 2 > alto) {
        inicioFila = alto - tableroAlto - 2;
    }

    int panelCol = inicioCol + tableroAncho + espacio;
    // se dibuja el tablero y la informacion lateral

    moveTo(inicioCol, 2);
    cout << Color::BOLD << "lumines fase 3" << Color::RESET;

    moveTo(inicioCol, 4);
    cout << "modo: " << modoTexto;

    moveTo(inicioCol + 16, 4);
    cout << "puntaje: " << juego.puntaje << "/" << META_PUNTAJE;

    moveTo(inicioCol + 39, 4);
    cout << "vidas: " << juego.vidas;

    moveTo(inicioCol, inicioFila);
    cout << "+";
    for (int c = 0; c < COLUMNAS * 3; c++) {
        cout << "-";
    }
    cout << "+";
// se dibujan las celdas del tablero, la pieza actual y la linea de tiempo
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
                // la linea de tiempo se pinta encima de la celda
                if (valor == 0) {
                    cout << Color::MAGENTA << " | " << Color::RESET;
                } else {
                    cout << Color::MAGENTA << "|"
                         << colorBloque(valor) << simboloBloque(valor)
                         << Color::MAGENTA << "|" << Color::RESET;
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
// se dibuja el panel lateral con informacion y controles
    moveTo(panelCol, inicioFila + 1);
    cout << "elementos:";
    moveTo(panelCol, inicioFila + 3);
    cout << Color::CYAN << "[#]" << Color::RESET << " color 1";
    moveTo(panelCol, inicioFila + 4);
    cout << Color::YELLOW << "[O]" << Color::RESET << " color 2";
    moveTo(panelCol, inicioFila + 5);
    cout << Color::GREEN << "[*]" << Color::RESET << " especial";
    moveTo(panelCol, inicioFila + 6);
    cout << Color::MAGENTA << "|" << Color::RESET << "   linea de tiempo";

    moveTo(panelCol, inicioFila + 9);
    cout << "controles:";
    moveTo(panelCol, inicioFila + 10);
    cout << "flechas mover";
    moveTo(panelCol, inicioFila + 11);
    cout << "z y x para rotar";
    moveTo(panelCol, inicioFila + 12);
    cout << "p pausar";
    moveTo(panelCol, inicioFila + 13);
    cout << "q salir";

    moveTo(inicioCol, alto - 2);
    if (juego.pausado) {
        cout << Color::YELLOW << "juego pausado" << Color::RESET;
    } else {
        cout << "hilos activos: jugador, caida, linea, puntaje y especiales";
    }

    cout.flush();
}
// muestra la pantalla final con el resultado de la partida y opciones para reiniciar o volver al menu :D
bool mostrarPantallaFinal(const ResultadoPartida& resultado, ScoreManager& scores) {
    int ancho = WIDTH;
    int alto = HEIGHT;
    getTerminalSize(ancho, alto);

    if (pantallaMuyPequena(ancho, alto)) {
        ancho = WIDTH;
        alto = HEIGHT;
    }

    clear();
    drawBox(1, 1, ancho, alto);

    if (resultado.salio) {
        escribirCentradoColor(5, ancho, Color::YELLOW, "partida cancelada");
    } else if (resultado.gano) {
        escribirCentradoColor(5, ancho, Color::GREEN, "ganaste la partida");
    } else {
        escribirCentradoColor(5, ancho, Color::RED, "perdiste la partida");
    }

    string puntaje = "puntaje final: " + to_string(resultado.puntaje);
    escribirCentrado(8, ancho, puntaje);

    if (resultado.puntaje > 0) {
        moveTo(columnaCentrada(ancho, "ingresa tu nombre sin espacios: jugador"), 11);
        string nombre = pedirNombreJugador();
        scores.addScore(nombre, resultado.puntaje);
        scores.saveToFile();

        escribirCentrado(13, ancho, "puntaje guardado");
    }

    escribirCentrado(16, ancho, "1. reiniciar partida");
    escribirCentrado(17, ancho, "2. volver al menu");

    moveTo(columnaCentrada(ancho, "elige una opcion: "), 20);
    cout << "elige una opcion: ";

    string opcion;
    getline(cin, opcion);

    return opcion == "1";
}

string celdaComoTexto(int valor) {
    // colores simples para distinguir los bloques en consola
    if (valor == 1) return string(Color::CYAN) + "[#]" + Color::RESET;
    if (valor == 2) return string(Color::YELLOW) + "[O]" + Color::RESET;
    if (valor == 3) return string(Color::GREEN) + "[*]" + Color::RESET;
    return "   ";
}
// simbolitos
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

int columnaCentrada(int ancho, const string& texto) {
    int columna = (ancho - (int)texto.size()) / 2 + 1;

    if (columna < 1) {
        columna = 1;
    }

    return columna;
}

void escribirCentrado(int fila, int ancho, const string& texto) {
    moveTo(columnaCentrada(ancho, texto), fila);
    cout << texto;
}

void escribirCentradoColor(int fila, int ancho, const string& color, const string& texto) {
    moveTo(columnaCentrada(ancho, texto), fila);
    cout << color << texto << Color::RESET;
}

bool pantallaMuyPequena(int ancho, int alto) {
    return ancho < 64 || alto < 22;
}

void mostrarAvisoPantallaPequena(int ancho, int alto) {
    clear();

    if (ancho < 40 || alto < 8) {
        moveTo(1, 1);
        cout << "agranda la terminal";
        cout.flush();
        return;
    }

    drawBox(1, 1, ancho, alto);
    escribirCentradoColor(3, ancho, Color::YELLOW, "terminal muy pequena");
    escribirCentrado(5, ancho, "tamano minimo recomendado: 64 x 22");
    escribirCentrado(6, ancho, "agranda la ventana para ver el tablero");

    cout.flush();
}

string colorBloque(int valor) {
    if (valor == 1) return Color::CYAN;
    if (valor == 2) return Color::YELLOW;
    if (valor == 3) return Color::GREEN;
    return Color::WHITE;
}
