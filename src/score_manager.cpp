/**
 * @file score_manager.cpp
 * @brief Implementacion de ScoreManager
 */

#include "score_manager.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

ScoreManager::ScoreManager()
{
    if (pthread_mutex_init(&m_mutex, nullptr) != 0) {
        std::perror("pthread_mutex_init (ScoreManager)");
        std::exit(EXIT_FAILURE);
    }
    loadFromFile();
}

ScoreManager::~ScoreManager()
{
    saveToFile();
    pthread_mutex_destroy(&m_mutex);
}

void ScoreManager::addScore(const std::string& name, int score)
{
    pthread_mutex_lock(&m_mutex);

    m_scores.push_back({name, score});

    // Ordena de mayor a menor
    std::sort(m_scores.begin(), m_scores.end(),
              [](const ScoreEntry& a, const ScoreEntry& b) {
                  return a.score > b.score;
              });

    // Limita al top MAX_SCORES
    if (static_cast<int>(m_scores.size()) > MAX_SCORES)
        m_scores.resize(MAX_SCORES);

    pthread_mutex_unlock(&m_mutex);
}

std::vector<ScoreEntry> ScoreManager::getScores() const
{
    pthread_mutex_lock(&m_mutex);
    std::vector<ScoreEntry> copy = m_scores;
    pthread_mutex_unlock(&m_mutex);
    return copy;
}

void ScoreManager::saveToFile() const
{
    pthread_mutex_lock(&m_mutex);
    std::ofstream ofs(FILE_PATH);
    if (ofs.is_open()) {
        for (const auto& entry : m_scores)
            ofs << entry.score << ' ' << entry.name << '\n';
    }
    pthread_mutex_unlock(&m_mutex);
}

void ScoreManager::loadFromFile()
{
    std::ifstream ifs(FILE_PATH);
    if (!ifs.is_open()) return;

    pthread_mutex_lock(&m_mutex);
    m_scores.clear();
    int score;
    std::string name;
    while (ifs >> score >> name && static_cast<int>(m_scores.size()) < MAX_SCORES)
        m_scores.push_back({name, score});
    pthread_mutex_unlock(&m_mutex);
}