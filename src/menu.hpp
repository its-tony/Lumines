#pragma once

/**
 * @file menu.hpp
 * @brief Sistema de menu para Lumines — Fase 2
 *
 * La clase Menu gestiona todas las pantallas de navegacion:
 *   - Menu principal con animacion de titulo
 *   - Seleccion de modo de juego
 *   - Pantalla de instrucciones
 *   - Tabla de mejores puntajes
 *
 * === Concurrencia ===
 *
 * Se utiliza un hilo POSIX dedicado (m_animThread) para animar el
 * titulo del menu de forma independiente al hilo principal de UI.
 *
 *   Hilo principal          |  Hilo de animacion (m_animThread)
 *   ----------------------- | ----------------------------------
 *   Lee m_animFrame         |  Escribe m_animFrame
 *   Lee/escribe m_running   |  Lee m_running
 *
 * Ambos campos son protegidos por m_mutex (pthread_mutex_t) para
 * evitar condiciones de carrera. El ciclo de vida es:
 *
 *   constructor -> pthread_mutex_init + pthread_create
 *   run()       -> ciclo de menu
 *   destructor  -> senaliza m_running=false -> pthread_join -> mutex_destroy
 */

#include "score_manager.hpp"
#include <string>
#include <pthread.h>

/* ── Tipos publicos ──────────────────────────────────────────────────── */

/** Opciones del menu principal */
enum class MenuOption {
    START        = 0,
    INSTRUCTIONS = 1,
    SCORES       = 2,
    EXIT         = 3,
    COUNT        = 4   ///< Centinela — numero total de opciones
};

/** Modos de juego disponibles */
enum class GameMode {
    SLOW = 1,   ///< Caida moderada, linea de tiempo suave
    FAST = 2    ///< Caida rapida, linea de tiempo agresiva
};

/** Resultado devuelto por Menu::run() */
struct MenuResult {
    MenuOption option = MenuOption::EXIT;
    GameMode   mode   = GameMode::SLOW;
};

/* ── Clase Menu ──────────────────────────────────────────────────────── */

class Menu {
public:
    /**
     * Constructor: inicializa el mutex y arranca el hilo de animacion.
     * @param scores  Referencia al gestor de puntajes compartido.
     */
    explicit Menu(ScoreManager& scores);

    /**
     * Destructor: senaliza al hilo de animacion que termine,
     * hace join y destruye el mutex.
     */
    ~Menu();

    // No copiar ni mover (recursos no copiables: mutex, pthread_t)
    Menu(const Menu&)            = delete;
    Menu& operator=(const Menu&) = delete;

    /**
     * Ejecuta el ciclo completo del menu.
     * Bloquea hasta que el usuario elige una opcion.
     * @return MenuResult con la opcion y el modo elegidos.
     */
    MenuResult run();

private:
    /* ── Pantallas ───────────────────────────────────────────────── */
    MenuResult   showMainMenu();
    GameMode     showModeSelection();
    void         showInstructions();
    void         showScores();

    /* ── Helpers de dibujo ───────────────────────────────────────── */
    void drawFrame()                 const;
    void drawTitle()                 const;
    void drawOptions(int selected)   const;
    void drawFooter()                const;

    /* ── Hilo de animacion ───────────────────────────────────────── */

    /**
     * Funcion de entrada del hilo de animacion.
     * Firma void*(void*) requerida por pthread_create.
     * @param arg  Puntero a la instancia de Menu (this).
     */
    static void* animThreadEntry(void* arg);

    /** Logica interna del hilo de animacion */
    void animLoop();

    /* ── Estado compartido (protegido por m_mutex) ───────────────── */
    mutable pthread_mutex_t  m_mutex;   ///< mutable: metodos const pueden adquirirlo
    pthread_t        m_animThread;      ///< Handle del hilo de animacion
    volatile bool    m_running;         ///< Flag de vida del hilo
    volatile int     m_animFrame;       ///< Frame actual de la animacion

    /* ── Recursos no compartidos ─────────────────────────────────── */
    ScoreManager&    m_scores;          ///< Gestor de puntajes (externo)

    /* ── Constantes de diseno ────────────────────────────────────── */
    static constexpr int   ANIM_FRAME_US  = 80'000;  ///< us entre frames
    static constexpr int   MENU_OPT_COUNT = static_cast<int>(MenuOption::COUNT);
    static constexpr int   MENU_START_ROW = 16;
};