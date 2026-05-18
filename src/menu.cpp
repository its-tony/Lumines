/**
 * @file menu.cpp
 * @brief Implementacion del sistema de menu para Lumines
 *
 * Patron de concurrencia usado:
 *   - Productor/consumidor simple: el hilo de animacion produce
 *     frames (incrementa m_animFrame) y el hilo principal los
 *     consume para decidir si redibujar.
 *   - Exclusion mutua con pthread_mutex_t para proteger los
 *     campos compartidos m_animFrame y m_running.
 */

#include "menu.hpp"
#include "display.hpp"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <time.h>       // nanosleep
#include <algorithm>
#include <string>
#include <vector>

using namespace Display;

/* ═══════════════════════════════════════════════════════════════════════
 *  Arte ASCII del titulo
 * ═══════════════════════════════════════════════════════════════════════ */

static const char* TITLE_ART[] = {
    " _      _   _  __  __  _  _  ____  ____",
    "| |    | | | ||  \\/  || || ||  __||  __|",
    "| |    | | | || |\\/| || || || |_  | |_  ",
    "| |___ | |_| || |  | || || ||  _| |  _| ",
    "|_____| \\___/ |_|  |_||____||_|   |_|   "
};
static constexpr int TITLE_ROWS = 5;

/* Paleta de colores para la animacion del titulo */
static const char* TITLE_COLORS[] = {
    Color::CYAN, Color::MAGENTA, Color::YELLOW, Color::GREEN, Color::BLUE
};
static constexpr int N_COLORS = 5;

/* Etiquetas de las opciones del menu principal */
static const char* MENU_LABELS[] = {
    "  [ 1 ]  Iniciar Partida   ",
    "  [ 2 ]  Instrucciones     ",
    "  [ 3 ]  Mejores Puntajes  ",
    "  [ 4 ]  Salir             "
};

/* ═══════════════════════════════════════════════════════════════════════
 *  Helper: sleep en microsegundos usando nanosleep (POSIX.1-2008)
 * ═══════════════════════════════════════════════════════════════════════ */

static void sleepUs(long us)
{
    struct timespec ts;
    ts.tv_sec  = us / 1'000'000L;
    ts.tv_nsec = (us % 1'000'000L) * 1000L;
    nanosleep(&ts, nullptr);
}

/* ═══════════════════════════════════════════════════════════════════════
 *  Constructor / Destructor
 * ═══════════════════════════════════════════════════════════════════════ */

Menu::Menu(ScoreManager& scores)
    : m_running(true)
    , m_animFrame(0)
    , m_scores(scores)
{
    // Inicializa el mutex con atributos por defecto
    if (pthread_mutex_init(&m_mutex, nullptr) != 0) {
        std::perror("pthread_mutex_init (Menu)");
        std::exit(EXIT_FAILURE);
    }

    // Crea el hilo de animacion.
    // animThreadEntry es static => puede pasarse como funcion C a pthreads.
    // Se pasa 'this' como argumento para que el hilo acceda a los miembros.
    if (pthread_create(&m_animThread, nullptr,
                       Menu::animThreadEntry, this) != 0) {
        std::perror("pthread_create (animThread)");
        pthread_mutex_destroy(&m_mutex);
        std::exit(EXIT_FAILURE);
    }
}

Menu::~Menu()
{
    // 1. Senaliza al hilo que debe terminar
    pthread_mutex_lock(&m_mutex);
    m_running = false;
    pthread_mutex_unlock(&m_mutex);

    // 2. Espera a que el hilo termine (join obligatorio para evitar leak)
    pthread_join(m_animThread, nullptr);

    // 3. Libera el mutex
    pthread_mutex_destroy(&m_mutex);
}

/* ═══════════════════════════════════════════════════════════════════════
 *  Hilo de animacion
 * ═══════════════════════════════════════════════════════════════════════ */

/**
 * Funcion de entrada estatica requerida por pthread_create.
 * Simplemente delega en la instancia real de Menu.
 */
void* Menu::animThreadEntry(void* arg)
{
    Menu* self = static_cast<Menu*>(arg);
    self->animLoop();
    return nullptr;
}

/**
 * Logica del hilo de animacion.
 *
 * Cada ANIM_FRAME_US microsegundos:
 *   1. Adquiere el mutex
 *   2. Lee m_running
 *   3. Si sigue activo, incrementa m_animFrame
 *   4. Libera el mutex
 *
 * El hilo principal lee m_animFrame (tambien bajo mutex) para saber
 * cuando redibujar el titulo con un color diferente.
 */
void Menu::animLoop()
{
    while (true) {
        sleepUs(ANIM_FRAME_US);

        pthread_mutex_lock(&m_mutex);
        bool alive = m_running;
        if (alive)
            m_animFrame = (m_animFrame + 1) % N_COLORS;
        pthread_mutex_unlock(&m_mutex);

        if (!alive) break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════
 *  API publica
 * ═══════════════════════════════════════════════════════════════════════ */

MenuResult Menu::run()
{
    MenuResult result;

    // Ciclo del menu hasta que el usuario salga
    bool inMenu = true;
    while (inMenu) {
        result = showMainMenu();

        switch (result.option) {
            case MenuOption::START:
                result.mode = showModeSelection();
                inMenu = false;
                break;
            case MenuOption::INSTRUCTIONS:
                showInstructions();
                break;
            case MenuOption::SCORES:
                showScores();
                break;
            case MenuOption::EXIT:
            default:
                inMenu = false;
                break;
        }
    }
    return result;
}

/* ═══════════════════════════════════════════════════════════════════════
 *  Pantalla: Menu principal
 * ═══════════════════════════════════════════════════════════════════════ */

MenuResult Menu::showMainMenu()
{
    MenuResult result;
    int selected  = 0;
    int prevFrame = -1;

    hideCursor();
    setRawMode();

    while (true) {
        // Lectura thread-safe del frame actual
        pthread_mutex_lock(&m_mutex);
        int frame = m_animFrame;
        pthread_mutex_unlock(&m_mutex);

        // Solo redibuja cuando el frame cambia (evita parpadeo)
        if (frame != prevFrame) {
            clear();
            drawFrame();
            drawTitle();
            drawOptions(selected);
            drawFooter();
            prevFrame = frame;
        }

        int ch = getChNonBlocking();
        if (ch == -1) {
            sleepUs(ANIM_FRAME_US / 4);
            continue;
        }

        if (ch == '\033') {
            // Secuencia de escape para flechas del teclado
            int ch2 = getChNonBlocking();
            if (ch2 == '[') {
                int ch3 = getChNonBlocking();
                if      (ch3 == 'A') selected = (selected - 1 + MENU_OPT_COUNT) % MENU_OPT_COUNT;
                else if (ch3 == 'B') selected = (selected + 1) % MENU_OPT_COUNT;
            }
        } else if (ch == 'w' || ch == 'W') {
            selected = (selected - 1 + MENU_OPT_COUNT) % MENU_OPT_COUNT;
        } else if (ch == 's' || ch == 'S') {
            selected = (selected + 1) % MENU_OPT_COUNT;
        } else if (ch >= '1' && ch <= '4') {
            selected = ch - '1';
        } else if (ch == '\n' || ch == '\r') {
            result.option = static_cast<MenuOption>(selected);
            break;
        }

        // Siempre actualiza el resaltado sin limpiar toda la pantalla
        drawOptions(selected);
    }

    restoreMode();
    showCursor();
    return result;
}

/* ═══════════════════════════════════════════════════════════════════════
 *  Pantalla: Seleccion de modo
 * ═══════════════════════════════════════════════════════════════════════ */

GameMode Menu::showModeSelection()
{
    const char* labels[2] = {
        "  [ 1 ]  Modo LENTO  -  Caida moderada, linea suave    ",
        "  [ 2 ]  Modo RAPIDO -  Caida veloz,   linea agresiva  "
    };

    int selected = 0;
    hideCursor();
    setRawMode();

    while (true) {
        clear();
        drawBox(1, 1, WIDTH, HEIGHT);

        printColorCentered(4,
            std::string(Color::BOLD) + Color::YELLOW,
            "=== SELECCIONA EL MODO DE JUEGO ===");

        hLine(2, 5, '-', WIDTH - 2);

        printColorCentered(8,
            Color::WHITE,
            "El modo afecta la velocidad de caida de bloques");
        printColorCentered(9,
            Color::WHITE,
            "y la rapidez con la que la linea de tiempo barre el tablero.");

        for (int i = 0; i < 2; ++i) {
            int row = 12 + i * 3;
            if (i == selected)
                printColorCentered(row,
                    std::string(Color::BG_YELLOW) + Color::BLACK + Color::BOLD,
                    labels[i]);
            else
                printColorCentered(row, Color::WHITE, labels[i]);
        }

        printColorCentered(HEIGHT - 2,
            std::string(Color::DIM) + Color::WHITE,
            "[ W/S o Flechas: navegar ]  [ Enter: confirmar ]");

        int ch = getCh();

        if (ch == 'w' || ch == 'W')
            selected = (selected - 1 + 2) % 2;
        else if (ch == 's' || ch == 'S')
            selected = (selected + 1) % 2;
        else if (ch == '1') selected = 0;
        else if (ch == '2') selected = 1;
        else if (ch == '\n' || ch == '\r') break;
        else if (ch == '\033') {
            int c2 = getChNonBlocking();
            if (c2 == '[') {
                int c3 = getChNonBlocking();
                if      (c3 == 'A') selected = (selected - 1 + 2) % 2;
                else if (c3 == 'B') selected = (selected + 1) % 2;
            }
        }
    }

    restoreMode();
    showCursor();
    return (selected == 0) ? GameMode::SLOW : GameMode::FAST;
}

/* ═══════════════════════════════════════════════════════════════════════
 *  Pantalla: Instrucciones
 * ═══════════════════════════════════════════════════════════════════════ */

void Menu::showInstructions()
{
    clear();
    hideCursor();
    setRawMode();
    drawBox(1, 1, WIDTH, HEIGHT);

    printColorCentered(3,
        std::string(Color::BOLD) + Color::CYAN,
        "=== INSTRUCCIONES DE LUMINES ===");
    hLine(2, 4, '-', WIDTH - 2);

    // Objetivo
    printColorCentered(6,
        std::string(Color::BOLD) + Color::YELLOW, "OBJETIVO");
    printColorCentered(7, Color::WHITE,
        "Forma cuadrados 2x2 del mismo simbolo. La linea de tiempo");
    printColorCentered(8, Color::WHITE,
        "los eliminara y ganara puntos. Alcanza 50 pts para ganar.");
    printColorCentered(9, Color::WHITE,
        "Pierdes si los bloques llegan a la parte superior o agotaslos 3 vidas.");

    hLine(2, 11, '-', WIDTH - 2);

    // Controles
    printColorCentered(12,
        std::string(Color::BOLD) + Color::YELLOW, "CONTROLES");

    struct { const char* key; const char* action; } controls[] = {
        { "  Flecha Izq / A  ", "Mover bloque a la izquierda  " },
        { "  Flecha Der / D  ", "Mover bloque a la derecha    " },
        { "  Flecha Abj / S  ", "Acelerar caida del bloque    " },
        { "  Espacio / W     ", "Rotar el bloque 90 grados    " },
        { "  P               ", "Pausar / Reanudar el juego   " },
        { "  Q               ", "Salir al menu principal      " },
    };

    for (int i = 0; i < 6; ++i) {
        char line[80];
        std::snprintf(line, sizeof(line), "%s  ->  %s",
                      controls[i].key, controls[i].action);
        printColorCentered(14 + i, Color::WHITE, line);
    }

    hLine(2, 21, '-', WIDTH - 2);

    // Elementos visuales
    printColorCentered(22, Color::WHITE,
        "[#] Bloque tipo A    [O] Bloque tipo B    [*] Bloque especial");

    printColorCentered(HEIGHT - 1,
        std::string(Color::DIM) + Color::WHITE,
        "[ Presiona cualquier tecla para volver ]");

    getCh();
    restoreMode();
    showCursor();
}

/* ═══════════════════════════════════════════════════════════════════════
 *  Pantalla: Tabla de puntajes
 * ═══════════════════════════════════════════════════════════════════════ */

void Menu::showScores()
{
    clear();
    hideCursor();
    setRawMode();
    drawBox(1, 1, WIDTH, HEIGHT);

    printColorCentered(3,
        std::string(Color::BOLD) + Color::YELLOW,
        "=== MEJORES PUNTAJES ===");
    hLine(2, 4, '-', WIDTH - 2);

    // Encabezado de tabla
    printColorCentered(6,
        std::string(Color::BOLD) + Color::CYAN,
        "  #    Nombre                          Puntaje");
    hLine(12, 7, '-', WIDTH - 24);

    // Obtiene copia thread-safe de los puntajes
    std::vector<ScoreEntry> scores = m_scores.getScores();

    if (scores.empty()) {
        printColorCentered(11,
            std::string(Color::DIM) + Color::WHITE,
            "-- Aun no hay puntajes registrados --");
    } else {
        // Medallas para top 3
        const char* medals[] = { Color::YELLOW, Color::WHITE, Color::RED };

        for (int i = 0; i < static_cast<int>(scores.size()); ++i) {
            char line[80];
            std::snprintf(line, sizeof(line),
                          "  %-3d  %-30s  %6d pts",
                          i + 1,
                          scores[i].name.c_str(),
                          scores[i].score);
            const char* color = (i < 3) ? medals[i] : Color::WHITE;
            printColorCentered(8 + i, color, line);
        }
    }

    hLine(2, HEIGHT - 3, '-', WIDTH - 2);
    printColorCentered(HEIGHT - 2,
        std::string(Color::DIM) + Color::WHITE,
        "[ Presiona cualquier tecla para volver ]");

    getCh();
    restoreMode();
    showCursor();
}

/* ═══════════════════════════════════════════════════════════════════════
 *  Helpers de dibujo (privados)
 * ═══════════════════════════════════════════════════════════════════════ */

void Menu::drawFrame() const
{
    drawBox(1, 1, WIDTH, HEIGHT);
    hLine(2, TITLE_ROWS + 3, '-', WIDTH - 2);
    hLine(2, HEIGHT - 3, '-', WIDTH - 2);
}

void Menu::drawTitle() const
{
    // Lectura thread-safe del frame para el color
    pthread_mutex_lock(&m_mutex);
    int frame = m_animFrame;
    pthread_mutex_unlock(&m_mutex);

    const char* color = TITLE_COLORS[frame % N_COLORS];
    const int startRow = 2;

    for (int i = 0; i < TITLE_ROWS; ++i)
        printColorCentered(startRow + i, color, TITLE_ART[i]);
}

void Menu::drawOptions(int selected) const
{
    for (int i = 0; i < MENU_OPT_COUNT; ++i) {
        int row = MENU_START_ROW + i * 2;
        if (i == selected)
            printColorCentered(row,
                std::string(Color::BG_CYAN) + Color::BLACK + Color::BOLD,
                MENU_LABELS[i]);
        else
            printColorCentered(row, Color::WHITE, MENU_LABELS[i]);
    }
}

void Menu::drawFooter() const
{
    printColorCentered(HEIGHT - 2,
        std::string(Color::DIM) + Color::WHITE,
        "[ W/S o Flechas: navegar ]  [ Enter / 1-4: confirmar ]");
}