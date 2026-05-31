#pragma once

#include "game_types.hpp"

void prepararNuevaPieza(DatosHilos& datos);
bool puedeMover(const EstadoJuego& juego, const Pieza& pieza, int nuevaFila, int nuevaColumna);
void moverPieza(EstadoJuego& juego, int cambioColumna);
void bajarPieza(DatosHilos& datos);
void rotarPieza(EstadoJuego& juego, bool horario);
void fijarPieza(EstadoJuego& juego);
void revisarLineaDeTiempo(EstadoJuego& juego);
void aplicarBloqueEspecial(EstadoJuego& juego, int fila, int columna);
