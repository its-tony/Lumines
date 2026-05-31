// universidad del valle de guatemala
// cc3086 - programacion de microprocesadores
// proyecto 2 - lumines en consola
//
// integrantes:
// antony portillo 25615
// sergio lopez 25848
// daniel vasquez 25582
// jordi cardona 251142

#include "display.hpp"
#include "game.hpp"
#include "menu.hpp"
#include "score_manager.hpp"

#include <cstdlib>
#include <cstdio>

using namespace Display;

int main() {
    ScoreManager scores;
    bool appRunning = true;

    while (appRunning) {
        MenuResult result;

        {
            Menu menu(scores);
            result = menu.run();
        }

        if (result.option == MenuOption::START) {
            runGame(result.mode, scores);
        } else if (result.option == MenuOption::EXIT) {
            appRunning = false;
        }
    }

    clear();
    showCursor();
    printf("gracias por jugar lumines\n");

    return EXIT_SUCCESS;
}
