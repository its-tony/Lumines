# Lumines - Fase 3

Proyecto 2 de Programacion de Microprocesadores.

Este programa es una version en consola del juego Lumines. El tablero, los bloques y la linea de tiempo se muestran con ASCII-Art. En esta fase final se agrego programacion concurrente con Pthreads para manejar partes del juego en hilos separados :D

```bash
solo funciona en linux, no en Windows :)
```

## Compilacion en WSL

```bash
g++ src/main.cpp src/menu.cpp src/display.cpp src/score_manager.cpp src/game.cpp src/game_logic.cpp src/game_threads.cpp src/game_screen.cpp -o lumines -pthread
```

## Ejecucion

```bash
./lumines 
```

## Controles

- flecha izquierda: mover la pieza a la izquierda
- flecha derecha: mover la pieza a la derecha
- flecha abajo: acelerar la caida
- z: rotar en sentido antihorario
- x: rotar en sentido horario
- p: pausar o continuar
- q: salir al menu

## Elementos visuales

- `[#]`: bloque color 1
- `[O]`: bloque color 2
- `[*]`: bloque especial
- `|`: linea de tiempo
- `+`, `-`, `|`: bordes del tablero
