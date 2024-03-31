#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "hw3.h" 

#define DEBUG(...) fprintf(stderr, "[          ] [ DEBUG ] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, " -- %s()\n", __func__)
#define MAX 2048

// JUST TO CHECK, COMMENT OUT WHEN DONE
void print_game_state(GameState* gs, int rows) {
    printf("BOARD:\n");
    for (int i = 0; i < rows; i++) {
        printf("%s\n", gs->board[i]);
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < gs->cols; j++) {
            printf("%d", gs->heights[i][j]);
        }
        printf("\n");
    }
}

GameState* initialize_game_state(const char *filename) {
    FILE *file;
    file = fopen(filename, "r");
    if(!file) {
        printf("Unable to open file.");
        return NULL;
    }
    // first find the dimensions of the current board to allocate memory for the gamestate
    int rows = 0, cols = 0;
    char buffer[MAX];
    while(fgets(buffer, MAX, file)) {
        cols = strlen(buffer) - 1;
        rows++;
    }
    // printf("Rows: %d, Cols: %d\n", rows, cols); //check

    // start allocaing memory for the gamestate
    GameState *gs = malloc(sizeof(GameState));
    gs->rows = rows;
    gs->cols = cols;
    gs->board = malloc(rows * sizeof(char*));
    for (int i = 0; i < rows; i++) {
        gs->board[i] = malloc(cols * sizeof(char));
    }
    // height array
    gs->heights = malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        gs->heights[i] = malloc(cols * sizeof(int));
    }
    // now initialize the array according to the file, have to go back to beginning of file using fseek (ask if this is OK)
    fseek(file, 0, SEEK_SET);
    for (int i = 0; i < rows; i++) {
        fgets(buffer, MAX, file);
        for (int j = 0; j < cols; j++) {
            gs->board[i][j] = buffer[j];
        }
    }
    // initialize heights to 1
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            gs->heights[i][j] = 1;
        }
    }
    fclose(file);
    return gs;
}

GameState* place_tiles(GameState *game, int row, int col, char direction, const char *tiles, int *num_tiles_placed) {
    // Check bounds
    if (row > game->rows || col > game->cols || row < 0 || col < 0) return game;
    if (direction != 'H' || direction != 'V') return game;

    // Check valid word
    int valid_words;
    FILE *file = fopen("/workspaces/cse220_homework3/tests/words.txt", "r");
    char buffer[30]; // ASK IF THIS IS OK
    while (fgets(buffer, 30, file)) {
        valid_words++;
    }



    (void) *game;
    (void) row;
    (void) col;
    (void) direction;
    (void) *tiles;
    (void) num_tiles_placed;



    return game;
}

GameState* undo_place_tiles(GameState *game) {
    (void)game;
    return game;
}

void free_game_state(GameState *game) {
    if (game != NULL) {
        for (int i = 0; i < game->rows; i++) {
            free(game->board[i]);
        }
        free(game->board);

        for (int i = 0; i < game->rows; i++) {
            free(game->heights[i]);
        }
        free(game->heights);
        free(game);

    } 
}

void save_game_state(GameState *game, const char *filename) {
    if (!game) {
        printf("GameState is null.\n");
        return;
    }
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Unable to open file.\n");
        return;
    }

    // board, use fputc to write char by char
    for (int i = 0; i < game->rows; i++) {
        for (int j = 0; j < game->cols; j++) {
            fputc(game->board[i][j], file);
        }
        fputc('\n', file);
    }

    // heights
    for (int i = 0; i < game->rows; i++) {
        for (int j = 0; j < game->cols; j++) {
            fprintf(file, "%d", game->heights[i][j]);
        }
        fputc('\n', file);
    }

    fclose(file);

    // for (int i = 0; i < game->rows; i++) {
    //     fprintf(file, "%s\n", game->board[i]);
    // }
    // for (int i = 0; i < game->rows; i++) {
    //     for (int j = 0; j < game->cols; j++) {
    //         fprintf(file, "%d", game->heights[i][j]);
    //     }
    //     fprintf(file, "\n");
    }

