#include <stdlib.h>

typedef struct Save {
    int row;
    int col;
    char *tiles;
    int *heights; 
} Save;

typedef struct SaveStack {
    int capacity;
    int size;
    Save **saves;
} SaveStack;

typedef struct GameState
{
    // define your struct here
    int rows;
    int cols;
    char **board;         
    int **heights;
    SaveStack *save_stack;
} GameState;



// typedef struct GameStack
// {
//     int capacity;
//     int size;
//     GameState **array;
// } GameStack;

// void print_game_state(GameState* gs, int rows); // COMMENT OUT WHEN DONE
GameState* initialize_game_state(const char *filename);
GameState* place_tiles(GameState *game, int row, int col, char direction, const char *tiles, int *num_tiles_placed);
GameState* undo_place_tiles(GameState *game);
void free_game_state(GameState *game);
void save_game_state(GameState *game, const char *filename);
