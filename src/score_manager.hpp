#pragma once

/**
 * @file score_manager.hpp
 * @brief Gestion de puntajes para Lumines
 *
 * Encapsula la tabla de mejores puntajes con persistencia en disco.
 * Operaciones thread-safe mediante pthread_mutex.
 */

#include <string>
#include <vector>
#include <pthread.h>

/** Representa una entrada en la tabla de puntajes */
struct ScoreEntry {
    std::string name;
    int         score;

    // Permite ordenar de mayor a menor
    bool operator>(const ScoreEntry& other) const {
        return score > other.score;
    }
};

class ScoreManager {
public:
    static constexpr int MAX_SCORES    = 10;
    static constexpr const char* FILE_PATH = "lumines_scores.txt";

    ScoreManager();
    ~ScoreManager();

    // No copiar ni mover (tiene mutex interno)
    ScoreManager(const ScoreManager&)            = delete;
    ScoreManager& operator=(const ScoreManager&) = delete;

    /**
     * Agrega un puntaje. Mantiene la lista ordenada (mayor a menor)
     * y limitada a MAX_SCORES entradas. Thread-safe.
     */
    void addScore(const std::string& name, int score);

    /**
     * Retorna una copia de la tabla actual. Thread-safe.
     */
    std::vector<ScoreEntry> getScores() const;

    /** Persiste la tabla en disco */
    void saveToFile() const;

    /** Carga la tabla desde disco (si existe) */
    void loadFromFile();

private:
    std::vector<ScoreEntry> m_scores;
    mutable pthread_mutex_t m_mutex;   ///< Protege m_scores
};