#include <gb/gb.h>
#include <stdio.h>
#include "Tiles.h"
#include "Font.h"

#define MAX_SNAKE_LENGTH 252
#define SNAKE_BUFFER (MAX_SNAKE_LENGTH + 1)  // Tamaño máximo + 1 para la posición actual

typedef enum {
    SELECT_CLASSIC,
    SELECT_REDUX
} SelectionState;

SelectionState currentSelection = SELECT_CLASSIC;

typedef enum {
    MODE_CLASSIC,
    MODE_REDUX
} GameMode;

GameMode currentMode = MODE_REDUX;  // Por defecto modo REDUX


typedef enum {
    START_SCREEN,
    PLAYING,
    GAME_OVER
} GameState;

GameState currentState = START_SCREEN;

typedef enum {
    RIGHT,
    LEFT,
    UP,
    DOWN
} Direction;

typedef struct {
    UINT8 x[SNAKE_BUFFER];
    UINT8 y[SNAKE_BUFFER];
} Snake;

typedef struct {
    UINT8 x;
    UINT8 y;
} Food;

Snake snake;
Food food;
Direction snakeDirection = RIGHT;
UINT8 snakeLength = 2;  // Comienza con una longitud de 2

UINT8 isPaused = 0;  // 0 = no está en pausa, 1 = está en pausa

UINT8 score = 0;
UINT8 highScore = 0;

UINT8 randomSeed = 0;
UINT8 isFirstMove = 1;

UINT8 jinglePlayed = 0; 

UINT8 foodCount = 0;  // Cuenta cuántas comidas ha comido la serpiente
UINT8 isSpecialFood = 0;  // Determina si la comida actual es especial
UINT8 specialFoodTimer = 0;  // Cuenta cuánto tiempo ha estado la comida especial en pantalla


UINT8 randomNumber(UINT8 min, UINT8 max) {
    randomSeed += DIV_REG;  // DIV_REG es un registro que siempre está incrementando
    return (randomSeed % (max - min + 1)) + min;
}

UINT8 getTileAt(UINT8 x, UINT8 y) {
    return get_bkg_tile_xy(x / 8, y / 8);
}

void loadTiles() {
    set_bkg_data(0, 2, Tiles); // Cargamos los 3 tiles en la memoria del Game Boy
}

void loadFont() {
    set_bkg_data(3, 110, Font); // Cargamos los tiles del abecedario y números en la memoria del Game Boy
}

void playBipSound() {
    // Configura el canal 1 (onda cuadrada)
    NR10_REG = 0x34; // Sweep: sube en tono
    NR11_REG = 0x81; // Longitud de onda: tono alto
    NR12_REG = 0xF1; // Envelope: inicio en volumen máximo, decrece suavemente
    NR13_REG = 0x60; // Frecuencia baja: valor aún más alto para un tono más agudo
    NR14_REG = 0x87; // Frecuencia alta: valor alto y comienza la reproducción
}

void playSelectSound() {
   // Configura el canal 1 (onda cuadrada)
    NR10_REG = 0x34;  // Sweep: sube en tono
    NR11_REG = 0x81;  // Longitud de onda: tono alto
    NR12_REG = 0xF1;  // Envelope: inicio en volumen máximo, decrece suavemente
    NR13_REG = 0x20;  // Frecuencia baja: valor más bajo para un tono más grave
    NR14_REG = 0x87;  // Frecuencia alta: valor más bajo y comienza la reproducción
}

void playStartSound() {
   // Configura el canal 1 (onda cuadrada)
    NR10_REG = 0x34;  // Sweep: sube en tono
    NR11_REG = 0x81;  // Longitud de onda: tono alto
    NR12_REG = 0xF1;  // Envelope: inicio en volumen máximo, decrece suavemente
    NR13_REG = 0x40;  // Frecuencia baja: valor más bajo para un tono más grave
    NR14_REG = 0x86;  // Frecuencia alta: valor más bajo y comienza la reproducción
}

void playGameOverJingle() {
    // Canal 1 - Primera nota (Fa alto)
    NR10_REG = 0x00; 
    NR11_REG = 0x81;
    NR12_REG = 0xF0;
    NR13_REG = 0x38;
    NR14_REG = 0x86;  
    delay(100);  // Espera

    // Canal 1 - Segunda nota (Mi alto)
    NR13_REG = 0x3C;
    NR14_REG = 0x86;  
    delay(100);  // Espera

    // Canal 1 - Tercera nota (Re alto)
    NR13_REG = 0x43;
    NR14_REG = 0x86;  
    delay(100);  // Espera

    // Canal 1 - Cuarta nota (Do alto)
    NR13_REG = 0x4B;
    NR14_REG = 0x86;  
    delay(100);  // Espera

    // Canal 1 - Quinta nota (Si medio)
    NR13_REG = 0x55;
    NR14_REG = 0x86;  
    delay(300);  // Espera

    // Apaga el canal de sonido después de que el jingle haya terminado
    NR12_REG = 0x00;
}

void displayStartScreen() {
    UINT8 S = 21;
    UINT8 N = 16;
    UINT8 A = 3;
    UINT8 K = 13;
    UINT8 E = 7;
    UINT8 P = 18;
    UINT8 R = 20;
    UINT8 T = 22;
    UINT8 W = 25;
    UINT8 I = 11;
    UINT8 B = 4;
    UINT8 Y = 27;
    UINT8 L = 14;
    UINT8 U = 23;
    UINT8 C = 5;
    UINT8 D = 6;
    UINT8 X = 26;
    UINT8 point = 85;
    UINT8 space = 0;
    UINT8 cpr = 41;
    UINT8 cero = 29;
    UINT8 dos = 31;
    UINT8 tres = 32;

    UINT8 letters[56] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 66, 0, 0, 0, 0,
        75, 77, 0, 57, 59, 0, 61, 63, 0, 69, 71, 0, 73, 68,
        76, 78, 0, 58, 60, 0, 62, 64, 0, 70, 72, 0, 74, 67,
        0, 0, 0, 0, 0, 0, 0, 79, 80, 81, 82, 83, 84, 65
    };

    UINT8 classicText[] = { C, L, A, S, S, I, C };
    
    UINT8 reduxText[] = { R, E, D, U, X };

    UINT8 cprText[] = { cpr, space, dos, cero, dos, tres };
    UINT8 devText[] = { W, R, I, T, T, E, N, space, B, Y, space, L, U, I, S, space, C, point};

    // Posiciona el texto "SNAKE" en el centro de la pantalla
    set_bkg_tiles(3, 3, 14, 4, letters);  // Ajusta las coordenadas si es necesario

    // Posiciona el menú debajo del logo 
    set_bkg_tiles(7, 11, 7, 1, classicText);
    set_bkg_tiles(7, 13, 5, 1, reduxText);

    set_bkg_tiles(7, 16, 6, 1, cprText);

    // Posiciona el texto "Written By Luis C" debajo del todo
    set_bkg_tiles(1, 17, 18, 1, devText);  // Ajusta las coordenadas si es necesario

    UINT8 cursorTile = 42;
    UINT8 blankTile = 0;  
    UINT8 cursorX = 5;  

    // Borra el cursor de ambas opciones
    set_bkg_tiles(cursorX, 11, 1, 1, &blankTile);  // Borra de CLASSIC
    set_bkg_tiles(cursorX, 13, 1, 1, &blankTile);  // Borra de REDUX

    // Dibuja el cursor en la opción seleccionada
    UINT8 cursorY = (currentSelection == SELECT_CLASSIC) ? 11 : 13;
    set_bkg_tiles(cursorX, cursorY, 1, 1, &cursorTile);
}

void waitForStart() {
    UINT8 key;
    do {
        key = joypad();
        if (key & J_SELECT) {
            // Reproduce el efecto de sonido
            playSelectSound();

            // Cambia la selección
            if (currentSelection == SELECT_CLASSIC) {
                currentSelection = SELECT_REDUX;
            } else {
                currentSelection = SELECT_CLASSIC;
            }
            displayStartScreen();  // Actualiza la pantalla para reflejar el cambio
            while(joypad() & J_SELECT);  // Espera a que el jugador suelte el botón "Select"
        }
        wait_vbl_done();
    } while(!(key & J_START));

    // Establece el modo de juego según la selección
    if (currentSelection == SELECT_CLASSIC) {
        currentMode = MODE_CLASSIC;
    } else {
        currentMode = MODE_REDUX;
    }

    // Espera a que el jugador suelte el botón "Start"
    do {
        key = joypad();
        wait_vbl_done();
        playStartSound();
    } while(key & J_START);
}

void decomposeScore(UINT8 score, UINT8* digits, UINT8* length) {
    UINT8 i = 0;
    do {
        digits[i] = score % 10;  // Obtiene el dígito menos significativo
        score /= 10;  // Elimina el dígito menos significativo
        i++;
    } while(score > 0);
    *length = i;  // Establece la longitud real del puntaje
}

void drawBorder() {
    UINT8 x, y;

    if (currentMode == MODE_CLASSIC) {
        // Dibuja los bordes verticales
        for(y = 3; y < 17; y++) {  // Comienza desde 3 para evitar sobrescribir el score
            UINT8 leftBorderTile = 103;  // Borde izquierdo
            UINT8 rightBorderTile = 104;  // Borde derecho
            set_bkg_tiles(0, y, 1, 1, &leftBorderTile);
            set_bkg_tiles(19, y, 1, 1, &rightBorderTile);
        }

        // Dibuja los bordes horizontales
        for(x = 1; x < 19; x++) {
            UINT8 topBorderTile = 106;  // Borde superior
            UINT8 bottomBorderTile = 105;  // Borde inferior
            set_bkg_tiles(x, 2, 1, 1, &topBorderTile);
            set_bkg_tiles(x, 17, 1, 1, &bottomBorderTile);
        }

        // Dibuja las esquinas
        UINT8 topLeftCornerTile = 107;  // Esquina superior izquierda
        UINT8 topRightCornerTile = 108;  // Esquina superior derecha
        UINT8 bottomLeftCornerTile = 109;  // Esquina inferior izquierda
        UINT8 bottomRightCornerTile = 110;  // Esquina inferior derecha

        set_bkg_tiles(0, 2, 1, 1, &topLeftCornerTile);
        set_bkg_tiles(19, 2, 1, 1, &topRightCornerTile);
        set_bkg_tiles(0, 17, 1, 1, &bottomLeftCornerTile);
        set_bkg_tiles(19, 17, 1, 1, &bottomRightCornerTile);
    } else {
        UINT8 blockTileIndex = 1; // Índice del tile de bloque

        for(y = 2; y < 17; y++) {
            set_bkg_tiles(0, y, 1, 1, &blockTileIndex);
            set_bkg_tiles(19, y, 1, 1, &blockTileIndex);
        }

        for(x = 0; x < 20; x++) {
            set_bkg_tiles(x, 2, 1, 1, &blockTileIndex);
            set_bkg_tiles(x, 17, 1, 1, &blockTileIndex);
        }
    }
}

void addRandomGrass() {
    UINT8 i, j, randomTile;
    UINT8 grassTileIndex = (currentMode == MODE_CLASSIC) ? 0 : 40; 

    for(i = 3; i < 17; i++) {  // Evitamos la primera fila (score) y las filas del borde
        for(j = 1; j < 19; j++) {  // Evitamos las columnas del borde
            randomTile = randomNumber(0, 5);  // Genera un número entre 0 y 5
            if(randomTile == 0) {
                set_bkg_tiles(j, i, 1, 1, &grassTileIndex);
            }
        }
    }
}

void initializeSnake() {
    for(UINT8 i = 0; i < SNAKE_BUFFER; i++) {
        snake.x[i] = 56 - i * 8;  // Separa cada posición por 8 píxeles
        snake.y[i] = 72;
    }

    // Dibuja la serpiente en el fondo usando tiles
    UINT8 snakeTile = 40;  // Índice del tile de la serpiente en Font.c
    set_bkg_tiles(snake.x[0] / 8, snake.y[0] / 8, 1, 1, &snakeTile);
    set_bkg_tiles(snake.x[1] / 8, snake.y[1] / 8, 1, 1, &snakeTile);
}

UINT8 isFoodOnSnake() {
    for(UINT8 i = 0; i < snakeLength; i++) {
        if(food.x == snake.x[i] && food.y == snake.y[i]) {
            return 1;  // La comida está en la posición de la serpiente
        }
    }
    return 0;  // La comida no está en la posición de la serpiente
}

void initializeFood() {
    do {
        food.x = randomNumber(2, 17) * 8;  // Asegurándonos de que la comida esté dentro de los bordes
        food.y = randomNumber(3, 15) * 8;
    } while (isFoodOnSnake());  // Repite hasta que la comida no esté en la posición de la serpiente

    UINT8 foodTile;
    if (foodCount >= 5) {  // Si ha comido 5 comidas, la siguiente es especial
        foodTile = (currentMode == MODE_CLASSIC) ? 102 : 86;  // Índice del tile de la comida especial
        isSpecialFood = 1;
        foodCount = 0;  // Reinicia el contador
        specialFoodTimer = 0;  // Inicializa el temporizador de la comida especial
    } else {
        foodTile = (currentMode == MODE_CLASSIC) ? 101 : 42;  // Índice del tile de la comida normal
        isSpecialFood = 0;
    }
    set_bkg_tiles(food.x / 8, food.y / 8, 1, 1, &foodTile);
}

UINT8 snakeSelfCollision() {
    for(UINT8 i = 1; i < snakeLength; i++) {  // Comienza desde 1 para evitar la cabeza
        if(snake.x[0] == snake.x[i] && snake.y[0] == snake.y[i]) {
            return 1;  // La cabeza ha chocado con el cuerpo
        }
    }
    return 0;  // No hay colisión
}

UINT8 wouldCollide() {
    UINT8 nextX = snake.x[0];
    UINT8 nextY = snake.y[0];

    switch(snakeDirection) {
        case RIGHT:
            nextX += 8;
            break;
        case LEFT:
            nextX -= 8;
            break;
        case UP:
            nextY -= 8;
            break;
        case DOWN:
            nextY += 8;
            break;
    }

    // Verifica si las coordenadas ajustadas chocarían con un borde
    if(nextX <= 0 || nextX >= 148 || nextY <= 16 || nextY >= 132) {
        return 1; // Choca con el borde
    }

    // Verifica si las coordenadas ajustadas chocarían con el cuerpo
    for(UINT8 i = 1; i < snakeLength; i++) {  // Comienza desde 1 para evitar la cabeza
        if(nextX == snake.x[i] && nextY == snake.y[i]) {
            return 1;  // Choca con el cuerpo
        }
    }

    return 0; // No choca
}

void moveSnake() {
    UINT8 lastTailX = snake.x[snakeLength - 1];
    UINT8 lastTailY = snake.y[snakeLength - 1];

    // Si el siguiente movimiento chocaría, simplemente regresa
    if (wouldCollide()) {
        currentState = GAME_OVER;
        return;
    }

    // Desplaza las posiciones en el buffer
    for(UINT8 i = SNAKE_BUFFER - 1; i > 0; i--) {
        snake.x[i] = snake.x[i - 1];
        snake.y[i] = snake.y[i - 1];
    }

    // Mover la cabeza en la dirección actual
    switch(snakeDirection) {
        case RIGHT:
            snake.x[0] += 8;
            break;
        case LEFT:
            snake.x[0] -= 8;
            break;
        case UP:
            snake.y[0] -= 8;
            break;
        case DOWN:
            snake.y[0] += 8;
            break;
    }

    // Actualizar las posiciones de los tiles
    for(UINT8 i = 0; i < snakeLength; i++) {
        UINT8 tileToUse;
        if (i == 0) {
            // Lógica para la cabeza
            switch(snakeDirection) {
                case RIGHT:
                    tileToUse = (currentMode == MODE_CLASSIC) ? 88 : 44;
                    break;
                case LEFT:
                    tileToUse = (currentMode == MODE_CLASSIC) ? 90 : 46;
                    break;
                case UP:
                    tileToUse = (currentMode == MODE_CLASSIC) ? 87 : 43;
                    break;
                case DOWN:
                    tileToUse = (currentMode == MODE_CLASSIC) ? 89 : 45;
                    break;
            }
        } else if (i == snakeLength - 1) {
            // Lógica para la cola
            if (snake.x[i] > snake.x[i - 1]) {
                tileToUse = (currentMode == MODE_CLASSIC) ? 99 : 55;  // Derecha
            } else if (snake.x[i] < snake.x[i - 1]) {
                tileToUse = (currentMode == MODE_CLASSIC) ? 100 : 56;  // Izquierda
            } else if (snake.y[i] > snake.y[i - 1]) {
                tileToUse = (currentMode == MODE_CLASSIC) ? 97 : 53;  // Abajo
            } else {
                tileToUse = (currentMode == MODE_CLASSIC) ? 98 : 54;  // Arriba
            }
        } else {  // Cuerpo
            // Lógica para las curvas y segmentos rectos
            if (snake.x[i] > snake.x[i - 1]) {
                if (snake.y[i] > snake.y[i + 1]) {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 94 : 50;  // Curva hacia abajo a la izquierda
                } else if (snake.y[i] < snake.y[i + 1]) {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 96 : 52;  // Curva hacia arriba a la izquierda
                } else {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 92 : 48;  // Horizontal
                }
            } else if (snake.x[i] < snake.x[i - 1]) {
                if (snake.y[i] > snake.y[i + 1]) {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 93 : 49;  // Curva hacia abajo a la derecha
                } else if (snake.y[i] < snake.y[i + 1]) {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 95 : 51;  // Curva hacia arriba a la derecha
                } else {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 92 : 48;  // Horizontal
                }
            } else if (snake.y[i] > snake.y[i - 1]) {
                if (snake.x[i] > snake.x[i + 1]) {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 94 : 50;  // Curva hacia abajo a la izquierda
                } else if (snake.x[i] < snake.x[i + 1]) {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 93 : 49;  // Curva hacia abajo a la derecha
                } else {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 91 : 47;  // Vertical
                }
            } else {
                if (snake.x[i] > snake.x[i + 1]) {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 96 : 52;  // Curva hacia arriba a la izquierda
                } else if (snake.x[i] < snake.x[i + 1]) {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 95 : 51;  // Curva hacia arriba a la derecha
                } else {
                    tileToUse = (currentMode == MODE_CLASSIC) ? 91 : 47;  // Vertical
                }
            }
        }
        set_bkg_tiles(snake.x[i] / 8, snake.y[i] / 8, 1, 1, &tileToUse);
    }

   // Verificar si la serpiente ha comido la comida
    if(snake.x[0] == food.x && snake.y[0] == food.y) {
        playBipSound();
        snakeLength++;

        if (isSpecialFood) {
            score += 3;  // Incrementa el puntaje en 3 puntos si es comida especial
            isSpecialFood = 0;  // Reinicia la bandera de comida especial
        } else {
            score++;  // Incrementa el puntaje en 1 punto si es comida normal
            foodCount++;  // Incrementa el contador de comida
        }

        displayScore();  // Actualiza el puntaje en pantalla
        initializeFood();
    } else if (isSpecialFood) {
        specialFoodTimer++;
        if (specialFoodTimer >= 20) {  // Aproximadamente 3 segundos (dependiendo de tu delay)
            // Borra el tile de la comida especial
            UINT8 blankTile = 0;  // Índice del tile blanco en Font.c
            set_bkg_tiles(food.x / 8, food.y / 8, 1, 1, &blankTile);

            isSpecialFood = 0;  // Reinicia la bandera de comida especial
            initializeFood();
        }
    }

    // Independientemente de si la serpiente ha comido o no, borra la última posición de la cola
    UINT8 randomTile = randomNumber(0, 1);
    if (randomTile == 0) {  // Si el número aleatorio es 0, coloca pasto
        UINT8 grassTile = (currentMode == MODE_CLASSIC) ? 0 : 40;  // Índice del tile de pasto en Font.c
        set_bkg_tiles(lastTailX / 8, lastTailY / 8, 1, 1, &grassTile);
    } else {
        UINT8 blankTile = 0;  // Índice del tile blanco en Font.c
        set_bkg_tiles(lastTailX / 8, lastTailY / 8, 1, 1, &blankTile);
    }

    // Verificar colisión con el propio cuerpo
    if(snakeSelfCollision()) {
        currentState = GAME_OVER;
        return;
    }
}

void handleInput() {
    UINT8 key = joypad();

    // Lógica de pausa
    if(key & J_START) {
        if(isPaused) {
            isPaused = 0;  // Si está en pausa, reanuda el juego
        } else {
            isPaused = 1;  // Si no está en pausa, pausa el juego
        }
        while(joypad() & J_START);  // Espera a que el jugador suelte el botón "Start"
        return;  // Sal del método para no procesar otros inputs mientras se pausa
    }

    // Resto de la lógica de manejo de entrada
    if((key & J_RIGHT) && snakeDirection != LEFT) {
        snakeDirection = RIGHT;
    } else if((key & J_LEFT) && snakeDirection != RIGHT) {
        snakeDirection = LEFT;
    } else if((key & J_UP) && snakeDirection != DOWN) {
        snakeDirection = UP;
    } else if((key & J_DOWN) && snakeDirection != UP) {
        snakeDirection = DOWN;
    }
}

UINT8 checkCollision() {
    for(UINT8 i = 0; i < snakeLength; i++) {
        if(snake.x[i] <= 0 || snake.x[i] >= 148 || snake.y[i] <= 16 || snake.y[i] >= 132) {
            return 1; // Ha ocurrido una colisión
        }
    }
    return 0; // No ha ocurrido una colisión
}

void displayScore() {
    UINT8 S = 21;
    UINT8 C = 5;
    UINT8 O = 17;
    UINT8 R = 20;
    UINT8 E = 7;
    UINT8 D = 6;

    UINT8 scoreLabel[5] = { S, C, O, R, E};  // "SCORE:"
    UINT8 recordLabel[6] = { R, E, C, O, R, D};  // "RECORD:"
    UINT8 digits[4];  // Reserva espacio para 4 dígitos
    UINT8 length;
    UINT8 displayDigits[4];  // Para almacenar los dígitos en el orden correcto

    decomposeScore(score, digits, &length);

    // Posiciona el texto "SCORE:" en la primera fila, al inicio
    set_bkg_tiles(0, 0, 5, 1, scoreLabel);  

    // Ahora, muestra el puntaje numérico debajo de "SCORE:"
    for(UINT8 i = 0; i < length; i++) {
        displayDigits[i] = 29 + digits[length - 1 - i];  // 29 es el índice del número 0
    }
    set_bkg_tiles(0, 1, length, 1, displayDigits);  // Posiciona el puntaje en la segunda fila, al inicio

    // Posiciona el texto "RECORD:" justificado a la derecha
    UINT8 recordLabelStartX = 20 - 6;  // 20 tiles en total - 6 tiles de "RECORD:"
    set_bkg_tiles(recordLabelStartX, 0, 6, 1, recordLabel);  

    // Muestra el récord numérico debajo de "RECORD:"
    decomposeScore(highScore, digits, &length);
    for(UINT8 i = 0; i < length; i++) {
        displayDigits[i] = 29 + digits[length - 1 - i];  // 29 es el índice del número 0
    }
    UINT8 recordValueStartX = 20 - length;  // Justifica el valor del récord a la derecha
    set_bkg_tiles(recordValueStartX, 1, length, 1, displayDigits);  // Posiciona el récord en la segunda fila, justificado a la derecha
}

void displayGameOver() {
    UINT8 space = 0;
    UINT8 G = 9;
    UINT8 A = 3;
    UINT8 M = 15;
    UINT8 E = 7;
    UINT8 O = 17;
    UINT8 V = 24;
    UINT8 R = 20;

    UINT8 gameOverText[] = { G, A, M, E, space, space, O, V, E, R };
    playGameOverJingle();
    // Posiciona el texto "Game Over" en la primera fila, al inicio
    set_bkg_tiles(5, 9, 10, 1, gameOverText);  // Ajusta las coordenadas si es necesario
}

void resetGame() {
    snakeDirection = RIGHT;
    isPaused = 0;
    snakeLength = 2;
    isSpecialFood = 0;
    foodCount = 0;
    score = 0;
    isFirstMove = 1;
    jinglePlayed = 0;

    cls();
    drawBorder();
    addRandomGrass();
    displayScore();
    initializeSnake();
    initializeFood();
}

void main(void) {
    NR52_REG = 0x80; // Habilita el sonido
    NR50_REG = 0x77; // Establece el volumen al máximo para ambos altavoces
    NR51_REG = 0xFF; // Habilita todos los canales de sonido para ambos altavoces

    DISPLAY_ON;
    SHOW_BKG;

    loadTiles();
    loadFont();  // Carga los tiles del abecedario

    while(1) {

        switch(currentState) {
            case START_SCREEN:
                cls();
                displayStartScreen();  // Muestra la pantalla de inicio
                waitForStart();  // Espera a que el jugador presione "Start"
                currentState = PLAYING;
                resetGame();
                break;

            case PLAYING:
                handleInput();

                if(!isPaused) {  // Solo actualiza la lógica del juego si no está en pausa
                    moveSnake();

                    // Verifica colisiones con los bordes
                    if(checkCollision()) {
                        currentState = GAME_OVER;
                    }

                    // Verifica colisiones con el propio cuerpo
                    if(snakeSelfCollision()) {
                        currentState = GAME_OVER;
                    }
                }

                delay(100); // Rapidez de la serpiente (ajustar si es necesario)
                wait_vbl_done();
                break;

            case GAME_OVER:
                if(score > highScore) {  // Actualizar el record
                    highScore = score;
                    isSpecialFood = 0;
                    foodCount = 0;
                }
                displayGameOver();
                delay(3000);
                currentState = START_SCREEN;
                break;
        }
    }
}