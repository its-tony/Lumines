#pragma once

/**
 * @file display.hpp
 * @brief Utilidades de visualizacion en consola para Lumines
 *
 * Abstrae operaciones de terminal: secuencias ANSI, posicionamiento
 * de cursor, modo raw de teclado y lectura no bloqueante.
 *
 * Plataforma: Linux/Unix (termios + secuencias ANSI).
 *
 * Autores: Antony Portillo, Sergio Lopez, Daniel Vasquez, Jordi Cardona
 * Curso  : CC3086 - Programacion de Microprocesadores, UVG 2026
 */

#include <string>

namespace Display {

/* ── Dimensiones estandar de consola ─────────────────────────────────── */
constexpr int WIDTH  = 80;
constexpr int HEIGHT = 24;

/* ── Codigos de color ANSI ───────────────────────────────────────────── */
namespace Color {
    constexpr const char* RESET   = "\033[0m";
    constexpr const char* BOLD    = "\033[1m";
    constexpr const char* DIM     = "\033[2m";

    constexpr const char* BLACK   = "\033[30m";
    constexpr const char* RED     = "\033[31m";
    constexpr const char* GREEN   = "\033[32m";
    constexpr const char* YELLOW  = "\033[33m";
    constexpr const char* BLUE    = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN    = "\033[36m";
    constexpr const char* WHITE   = "\033[37m";

    constexpr const char* BG_BLACK   = "\033[40m";
    constexpr const char* BG_RED     = "\033[41m";
    constexpr const char* BG_GREEN   = "\033[42m";
    constexpr const char* BG_YELLOW  = "\033[43m";
    constexpr const char* BG_BLUE    = "\033[44m";
    constexpr const char* BG_MAGENTA = "\033[45m";
    constexpr const char* BG_CYAN    = "\033[46m";
    constexpr const char* BG_WHITE   = "\033[47m";
} // namespace Color

/* ── Funciones de control de terminal ────────────────────────────────── */

/** Limpia la pantalla completa */
void clear();

/** Mueve el cursor a (col, row) — 1-indexed */
void moveTo(int col, int row);

/** Oculta el cursor del terminal */
void hideCursor();

/** Muestra el cursor del terminal */
void showCursor();

/** Configura la terminal en modo raw (sin eco, sin buffer de linea) */
void setRawMode();

/** Restaura la configuracion original de la terminal */
void restoreMode();

/* ── Funciones de dibujo ─────────────────────────────────────────────── */

/** Imprime texto centrado en la fila dada */
void printCentered(int row, const std::string& text);

/**
 * Imprime texto centrado con color ANSI.
 * @param row    Fila en pantalla (1-indexed)
 * @param color  Secuencia de escape ANSI (usar Display::Color::*)
 * @param text   Texto a mostrar
 */
void printColorCentered(int row,
                        const std::string& color,
                        const std::string& text);

/** Dibuja una linea horizontal de 'ch' desde (col, row) con longitud width */
void hLine(int col, int row, char ch, int width);

/** Dibuja un recuadro ASCII con esquinas '+', lados '-' y '|' */
void drawBox(int col, int row, int width, int height);

/* ── Lectura de teclado ──────────────────────────────────────────────── */

/**
 * Lee una tecla sin bloquear.
 * @return Codigo ASCII de la tecla, o -1 si no hay entrada disponible.
 */
int getChNonBlocking();

/**
 * Lee una tecla bloqueando hasta recibir entrada.
 * @return Codigo ASCII de la tecla presionada.
 */
int getCh();

/** Limpia teclas pendientes en el buffer de entrada */
void flushInput();

} // namespace Display
