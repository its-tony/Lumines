#pragma once

#include "game_types.hpp"
#include "score_manager.hpp"

#include <string>

void dibujarJuego(const EstadoJuego& juego, GameMode mode);
bool mostrarPantallaFinal(const ResultadoPartida& resultado, ScoreManager& scores);
std::string celdaComoTexto(int valor);
char simboloBloque(int valor);
std::string pedirNombreJugador();
