#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "hw3.h" 

#define DEBUG(...) fprintf(stderr, "[          ] [ DEBUG ] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, " -- %s()\n", __func__)
#define MAX 2048
#define DEFAULT_STACK_CAPACITY 10

// JUST TO CHECK, COMMENT OUT WHEN DONE
// void print_game_state(GameState* game, int rows) {
//     printf("BOARD:\n");
//     for (int i = 0; i < rows; i++) {
//         printf("%s\n", game->board[i]);
//     }
//     for (int i = 0; i < rows; i++) {
//         for (int j = 0; j < game->cols; j++) {
//             printf("%d", game->heights[i][j]);
//         }
//         printf("\n");
//     }
// }

// void free_one_game_state(GameState* game) {
//     if (game != NULL) {
//         for (int i = 0; i < game->rows; i++) {
//             free(game->board[i]);
//         }
//         free(game->board);

//         for (int i = 0; i < game->rows; i++) {
//             free(game->heights[i]);
//         }
//         free(game->heights);
//         free(game);
//         }
// }

// GameStack* initialize_stack() {
//     GameStack* stack = malloc(sizeof(GameStack));
//     stack->capacity = 10;
//     stack->size = 0;
//     stack->array = malloc(sizeof(GameState*) * stack->capacity);

//     return stack;
// }

void enlarge_stack(SaveStack* stack) {
    int new_capacity = stack->capacity * 2;
    Save **temp_array = realloc(stack->saves, sizeof(Save*) * new_capacity);
    if (!temp_array) {
        printf("Realloc failed.\n");
        return;
    }
    stack->saves = temp_array;
    stack->capacity = new_capacity;
}

void push(SaveStack* stack, Save *save) {
    if (!stack || !save) {
        return;
    }
    if (stack->size == stack->capacity) {
        enlarge_stack(stack);
    }
    stack->saves[stack->size++] = save; 
}

Save* peek(SaveStack* stack) {
    if (!stack || stack->size == 0) {return NULL;}
    return stack->saves[stack->size - 1];
}

Save* pop(SaveStack* stack) {
    if (!stack || stack->size <= 0) {return NULL;}
    return stack->saves[stack->size-- - 1];
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
    GameState *game = malloc(sizeof(GameState));
    if (!game) {return NULL;}
    game->rows = rows;
    game->cols = cols;
    game->save_stack = malloc(sizeof(SaveStack));
    game->board = malloc(rows * sizeof(char*));
    for (int i = 0; i < rows; i++) {
        game->board[i] = malloc(cols * sizeof(char));
    }
    // height array
    game->heights = malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        game->heights[i] = malloc(cols * sizeof(int));
    }
    // now initialize the array according to the file, have to go back to beginning of file using fseek (ask if this is OK)
    fseek(file, 0, SEEK_SET);
    for (int i = 0; i < rows; i++) {
        fgets(buffer, MAX, file);
        for (int j = 0; j < cols; j++) {
            game->board[i][j] = buffer[j];
        }
    }
    // initialize heights
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (game->board[i][j] == '.') {
                game->heights[i][j] = 0;
            } else {
            game->heights[i][j] = 1;
            }
        }
    }
    fclose(file);

    //initialize save stakc
    game->save_stack->capacity = DEFAULT_STACK_CAPACITY;
    game->save_stack->size = 0;
    game->save_stack->saves = malloc(sizeof(Save*) * game->save_stack->capacity);
    return game;
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

        if (game->save_stack != NULL) {
            for (int i = 0; i < game->save_stack->size; i++) {
                Save *save = game->save_stack->saves[i];
                if (save != NULL) {
                    free(save->tiles);
                    free(save->heights);
                    free(save);
                }
            }
            free(game->save_stack->saves);
            free(game->save_stack);
        }

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

