# Lumines - Fase 2

Proyecto 2 de Programacion de Microprocesadores.  

Este programa es una version en consola del juego Lumines. En esta fase se trabaja el entorno grafico usando ASCII-Art, con menu principal, instrucciones, puntajes destacados, seleccion de modo y una pantalla inicial del juego.

## Compilacion en WSL

```bash
g++ src/main.cpp src/menu.cpp src/display.cpp src/score_manager.cpp src/game.cpp -o lumines -pthread

Controles

flechas o A/D/S: mover la pieza
Z: rotar en sentido antihorario
X o espacio: rotar en sentido horario
P: pausar o continuar
Q: salir al menu

Elementos visuales
[#]: bloque color 1
[O]: bloque color 2
[*]: bloque especial
|: linea de tiempo
+, -, |: bordes del tablero


Archivos principales
src/main.cpp: inicia el programa
src/menu.cpp: menu, instrucciones y puntajes
src/display.cpp: funciones de consola
src/score_manager.cpp: manejo de puntajes
src/game.cpp: tablero y demo del juego