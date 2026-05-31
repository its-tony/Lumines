/**
 * @file display.cpp
 * @brief Implementacion de utilidades de visualizacion en consola
 */

#include "display.hpp"

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

namespace Display {

//Estado inicial del terminal para restaurar al finalizar el programa
static struct termios s_origTermios;
static bool           s_rawActive = false;

// control de la terminal

void clear()
{
    std::printf("\033[2J\033[H");
    std::fflush(stdout);
}

void moveTo(int col, int row)
{
    std::printf("\033[%d;%dH", row, col);
    std::fflush(stdout);
}

void hideCursor()
{
    std::printf("\033[?25l");
    std::fflush(stdout);
}

void showCursor()
{
    std::printf("\033[?25h");
    std::fflush(stdout);
}

void setRawMode()
{
    if (s_rawActive) return;
    tcgetattr(STDIN_FILENO, &s_origTermios);

    struct termios raw = s_origTermios;
    raw.c_lflag &= static_cast<unsigned int>(~(ICANON | ECHO));
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    s_rawActive = true;
}

void restoreMode()
{
    if (!s_rawActive) return;
    tcsetattr(STDIN_FILENO, TCSANOW, &s_origTermios);
    s_rawActive = false;
}

void getTerminalSize(int& width, int& height)
{
    // si no se puede leer el tamano se usan valores normales
    width = WIDTH;
    height = HEIGHT;

    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_col > 0) width = ws.ws_col;
        if (ws.ws_row > 0) height = ws.ws_row;
    }
}

// dibujar en pantalla

void printCentered(int row, const std::string& text)
{
    int len = static_cast<int>(text.size());
    int col = (WIDTH - len) / 2 + 1;
    if (col < 1) col = 1;
    moveTo(col, row);
    std::printf("%s", text.c_str());
    std::fflush(stdout);
}

void printColorCentered(int row,
                        const std::string& color,
                        const std::string& text)
{
    int len = static_cast<int>(text.size());
    int col = (WIDTH - len) / 2 + 1;
    if (col < 1) col = 1;
    moveTo(col, row);
    std::printf("%s%s%s", color.c_str(), text.c_str(), Color::RESET);
    std::fflush(stdout);
}

void hLine(int col, int row, char ch, int width)
{
    moveTo(col, row);
    for (int i = 0; i < width; ++i)
        std::putchar(ch);
    std::fflush(stdout);
}

void drawBox(int col, int row, int width, int height)
{
    // Fila superior
    moveTo(col, row);
    std::putchar('+');
    for (int i = 0; i < width - 2; ++i) std::putchar('-');
    std::putchar('+');

    // Laterales
    for (int r = 1; r < height - 1; ++r) {
        moveTo(col, row + r);
        std::putchar('|');
        moveTo(col + width - 1, row + r);
        std::putchar('|');
    }

    // Fila inferior
    moveTo(col, row + height - 1);
    std::putchar('+');
    for (int i = 0; i < width - 2; ++i) std::putchar('-');
    std::putchar('+');

    std::fflush(stdout);
}

// teclado

int getChNonBlocking()
{
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    int ch = std::getchar();
    fcntl(STDIN_FILENO, F_SETFL, flags);
    return ch;
}

int getCh()
{
    return std::getchar();
}

void flushInput()
{
    while (getChNonBlocking() != -1) {
        // limpia pulsaciones que quedaron pendientes
    }
}

} 
