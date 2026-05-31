#pragma once

#include "game_types.hpp"

void* hiloJugador(void* arg);
void* hiloCaida(void* arg);
void* hiloLineaTiempo(void* arg);
void* hiloPuntaje(void* arg);
void* hiloBloquesEspeciales(void* arg);

void avisarCambio(DatosHilos& datos);
bool juegoTerminado(DatosHilos& datos);
bool esperarConSalida(DatosHilos& datos, int milisegundos);
int leerTeclaJuego();
