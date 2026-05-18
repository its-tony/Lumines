/**
 * @file main.cpp
 * @brief Punto de entrada principal — Lumines Fase 2
 *
 * Flujo de ejecucion:
 *   1. Carga puntajes previos (ScoreManager)
 *   2. Instancia Menu (arranca hilo de animacion internamente)
 *   3. Ejecuta ciclo del menu hasta que el usuario elija
 *   4. Despacha la accion elegida (jugar, instrucciones, puntajes, salir)
 *   5. Al destruirse Menu se hace join del hilo y se limpia el mutex
 *
 * Autores: Antony Portillo, Sergio Lopez, Daniel Vasquez, Jordi Cardona
 * Curso  : CC3086 - Programacion de Microprocesadores, UVG 2026
 */

#include "menu.hpp"
#include "score_manager.hpp"
#include "display.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

using namespace Display;

/* ── Placeholder de juego (se reemplaza en Fase 3) ───────────────────── */
static void launchGame(GameMode mode, ScoreManager& scores)
{
    clear();
    hideCursor();
    drawBox(1, 1, WIDTH, HEIGHT);

    const char* modeStr = (mode == GameMode::SLOW) ? "LENTO" : "RAPIDO";
    char msg[80];
    std::snprintf(msg, sizeof(msg),
                  "Iniciando juego en MODO %s...", modeStr);

    printColorCentered(9,  std::string(Color::BOLD) + Color::GREEN, msg);
    printColorCentered(11, Color::WHITE,
        "[ El modulo de juego se integrara en Fase 3 ]");
    printColorCentered(13, Color::WHITE,
        "Se registrara un puntaje de prueba al salir.");
    printColorCentered(HEIGHT - 2,
        std::string(Color::DIM) + Color::WHITE,
        "[ Presiona cualquier tecla para volver al menu ]");

    showCursor();
    setRawMode();
    getCh();
    restoreMode();

    // Puntaje de prueba para demostrar la tabla de puntajes
    scores.addScore("JUGADOR_1", 35);
    scores.saveToFile();
}

/* ── main ────────────────────────────────────────────────────────────── */
int main()
{
    ScoreManager scores;   // Carga puntajes desde disco en el constructor

    bool appRunning = true;
    while (appRunning) {
        // Menu se construye cada iteracion del loop externo para
        // reiniciar la animacion limpiamente al volver del juego.
        Menu menu(scores);
        MenuResult result = menu.run();
        // Al salir del scope, ~Menu() hace join del hilo de animacion.

        switch (result.option) {
            case MenuOption::START:
                launchGame(result.mode, scores);
                break;

            case MenuOption::EXIT:
            default:
                appRunning = false;
                break;

            // INSTRUCTIONS y SCORES ya son manejados internamente por Menu::run()
            // Si run() los retorna es porque el usuario eligio START o EXIT.
            case MenuOption::INSTRUCTIONS:
            case MenuOption::SCORES:
                break;
        }
    }

    clear();
    showCursor();
    std::printf("Gracias por jugar Lumines. Hasta luego!\n");

    return EXIT_SUCCESS;
}