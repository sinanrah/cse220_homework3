#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "hw3.h" 

#define DEBUG(...) fprintf(stderr, "[          ] [ DEBUG ] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, " -- %s()\n", __func__)
#define MAX 2048
#define DEFAULT_STACK_CAPACITY 10
#define VALID_WORD_MAX_SIZE 30

int num_valid_words = 0;
char **words_array = NULL;
// char words_buffer[VALID_WORD_MAX_SIZE]; // ASK IF THIS IS OK, array gets updated every time so its just 1 array so it should be ok


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

// GameStack* initialize_stack() {
//     GameStack* stack = malloc(sizeof(GameStack));
//     stack->capacity = 10;
//     stack->size = 0;
//     stack->array = malloc(sizeof(GameState*) * stack->capacity);
//     return stack;
// }

// check_first_word

int get_valid_word_count() {
    FILE* file = fopen("tests/words.txt", "r");
    char buffer[VALID_WORD_MAX_SIZE];
    while (fgets(buffer, VALID_WORD_MAX_SIZE, file)) {
        num_valid_words++;
    }
    fclose(file);
    return num_valid_words;
}

void create_valid_word_array() {
    FILE* file = fopen("tests/words.txt", "r");
    char buffer[VALID_WORD_MAX_SIZE];
    words_array = (char**)malloc(sizeof(char *) * num_valid_words);
    if (!words_array) {
        printf("Problem mallocing words_array\n");
        fclose(file);
        return;
    }

    for (int i = 0; i < num_valid_words; i++) {
        fgets(buffer, VALID_WORD_MAX_SIZE, file);
        size_t array_length = strlen(buffer);
        if (buffer[array_length - 1] == '\n') {
            buffer[array_length - 1] = '\0'; 
            array_length--; 
        }
        
        words_array[i] = (char*)malloc(sizeof(char) * (array_length + 1));
        if (!words_array[i]) {
            printf("Problem mallocing words_array[%d]\n", i);
            fclose(file);
            continue;            
        }
        for (size_t j = 0; j < array_length; j++) {
            words_array[i][j] = toupper(buffer[j]);
        }
        words_array[i][array_length] = '\0';
    }   
    fclose(file);
}

void free_word_array() {
    if (!words_array) {return;}
    for (int i = 0; i < num_valid_words; i++) {
        free(words_array[i]);
    }
    free(words_array);
}

int check_valid(const char* word) {
    for (int i = 0; i < num_valid_words; i++) {
        if (strcmp(words_array[i], word) == 0) {
            return 1; 
        }
    }
    return 0; 
}

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

void save(GameState *game, int row, int col, char direction, const char *tiles) {
    int array_size = strlen(tiles);
    Save *save;
    save = malloc(sizeof(Save));
    if (!save) {
        printf("Problem mallocing save state.\n");
        return;
    }
    save->row = row;
    save->col = col;
    save->rows = game->rows;
    save->cols = game->cols;
    save->direction = direction;
    save->resized = 0;
    // check if this save also includes a resize (for use later in place_tiles)
    if (direction == 'H') {
        if (game->cols < (col + array_size)) {
            save->resized = 1;
            array_size = game->cols - col;
        }
    }
    if (direction == 'V') {
        if (game->rows < (row + array_size)) {
            save->resized = 1;
            array_size = game->rows - row;
        }
    }
    // allocate memory for tiles and heights 1d arrays
    save->tiles = malloc(sizeof(char) * array_size);
    if (!save->tiles) {
        printf("Malloc save->tiles failed.\n");
        return;
    }
    save->heights = malloc(sizeof(int) * array_size);
    if (!save->heights) {
        printf("Malloc save->heights failed.\n");
        return;
    }
    // initialize tiles and heights depending on direction
    if (direction == 'H') {
        for (int i = 0; i < array_size; i++) {
            save->tiles[i] = game->board[row][col + i];
            save->heights[i] = game->heights[row][col + i];
        }
    } else {
        for (int i = 0; i < array_size; i++) {
            save->tiles[i] = game->board[row + i][col];
            save->heights[i] = game->heights[row + i][col];
        }        
    }
    save->length = array_size;
    push(game->save_stack, save);
}

void resize_vertical(GameState *game, int rows) {
    if (!game) {return;}
    // Change rows to represent new num of rows, realloc to add space for the appropriate amount of char pointers
    game->rows += rows;
    game->board = realloc(game->board, sizeof(char*) * game->rows);
    if (!game->board) {
        printf("Problem reallocing game->board\n");
        return;
    }
    if (rows > 0) {
        // allocate memory for the chars in the columns of the added rows
        for (int i = game->rows - rows; i < game->rows; i++) {
            game->board[i] = malloc(game->cols * sizeof(char));
            if (!game->board[i]) {
                printf("Problem reallocing game->board columns\n");
                return;
            }
            // initialize each new row to '.'
            for (int j = 0; j < game->cols; j++) {
                game->board[i][j] = '.';
            }
        }
    }

    // resize heights, first realloc size for int* (rows) then for each added row, malloc size for cols * int
    game->heights = realloc(game->heights, sizeof(int*) * game->rows);
    if (!game->heights) {
        printf("Problem reallocing game->heights\n");
        return;
    }
    if (rows > 0) {
        for (int i = game->rows - rows; i < game->rows; i++) {
            game->heights[i] = malloc(game->cols * sizeof(int));
            if (!game->heights[i]) {
                printf("Problem reallocing game->heights col\n");
                return;
            }
            // initialize each new row to 0
            for (int j = 0; j < game->cols; j++) {
                game->heights[i][j] = 0;        
            }
        }
    }


}

void resize_horizontal(GameState *game, int cols) {
    if (!game) {return;}
    // Change cols to new amt of cols, realloc new cols for each row for chars
    game->cols += cols;
    for (int i = 0; i < game->rows; i++) {
        game->board[i] = realloc(game->board[i], sizeof(char) * game->cols);
        if (!game->board[i]) {
            printf("Problem reallocing game->board[%d]\n", i);
            return;
        }
        if (cols > 0) {
            // initialize the new cols of each row to '.'
            for (int j = game->cols - cols; j < game->cols; j++) {
                game->board[i][j] = '.';
            }
        }
    }
    if (cols > 0) {
        // realloc new cols for each row for heights
        for (int i = 0; i < game->rows; i++) {
            game->heights[i] = realloc(game->heights[i], sizeof(int) * game->cols);
            if (!game->heights[i]) {
                printf("Problem reallocing game->heights[%d]\n", i);
                return;
            }
            // initialize the new cols of each row to 0
            for (int j = game->cols - cols; j < game->cols; j++) {
                game->heights[i][j] = 0;
            }
        }
    }
}

GameState* initialize_game_state(const char *filename) {
    FILE *file;
    file = fopen(filename, "r");
    if(!file) {
        printf("Unable to open file.");
        return NULL;
    }
    if (num_valid_words == 0) {
        get_valid_word_count();
        create_valid_word_array();
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
    if (!game->save_stack) {
        printf("Problem mallocing game->save_stack\n");
        return game;
    }
    game->board = malloc(rows * sizeof(char*));
    if (!game->board) {
        printf("Problem mallocing game->board\n");
        return game;
    }
    for (int i = 0; i < rows; i++) {
        game->board[i] = malloc(cols * sizeof(char));
        if (!game->board[i]) {
            printf("Problem mallocing game->board[%d]\n", i);
            return game;
        }
    }
    // height array
    game->heights = malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        game->heights[i] = malloc(cols * sizeof(int));
        if (!game->heights[i]) {
            printf("Problem mallocing game->heights[%d]\n", i);
            return game;
        }
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
    if (!game->save_stack->saves) {
        printf("Problem mallocing save stack");
        return game;
    }
    return game;
}
// SIMPLIFY ALL IF STATEMENTS TO CONDITIONAL OPERATORS, RMB DELETE THIS COMMENT WHEN DONE

void find_word_bounds(GameState *game, int row, int col, char direction, int *start, int *end) {
    int i = (direction == 'H') ? col : row;
    while (i >= 0 && game->board[direction == 'H' ? row : i][direction == 'H' ? i : col] != '.') {
        --i;
    }
    *start = i + 1;

    i = (direction == 'H') ? col : row;
    int limit = (direction == 'H') ? game->cols : game->rows;
    while (i < limit && game->board[direction == 'H' ? row : i][direction == 'H' ? i : col] != '.') {
        ++i;
    }
    *end = i - 1;
}

int check_word_validity(GameState *game, int row, int col, char direction) {
    int start, end;
    find_word_bounds(game, row, col, direction, &start, &end);

    int word_length = (direction == 'H') ? end - start + 1 : end - start + 1;
    char word[word_length + 1];

    for (int i = 0; i < word_length; ++i) {
        word[i] = game->board[direction == 'H' ? row : start + i][direction == 'H' ? start + i : col];
    }
    word[word_length] = '\0';

    return check_valid(word);
}

int check_bonus_words(GameState *game, int row, int col, char direction, const char *tiles) {
    int length = strlen(tiles);
    for (int i = 0; i < length; ++i) {
        if (tiles[i] != ' ') {
            int bonus_row = (direction == 'H') ? row : row + i;
            int bonus_col = (direction == 'H') ? col + i : col;
            if (!check_word_validity(game, bonus_row, bonus_col, direction == 'H' ? 'V' : 'H')) {
                return 0;
            }
        }
    }
    return 1;
}

int check_empty_board(GameState *game) {
    for (int row = 0; row < game->rows; row++) {
        for (int col = 0; col < game->cols; col++) {
            if (game->board[row][col] != '.' && game->board[row][col] != ' ') {
                return 0;
            }
        }
    }
    return 1;
}

GameState* place_tiles(GameState *game, int row, int col, char direction, const char *tiles, int *num_tiles_placed) {
    // Check bounds
    if (row > game->rows || col > game->cols || row < 0 || col < 0) return game;
    if (direction != 'H' && direction != 'V') return game;
    if (!game || !tiles) {return NULL;}
    int length = strlen(tiles);
    if (check_empty_board(game) && length < 2) {
        *num_tiles_placed = 0;
        return game;
    }
    int placed = 0;
    // save, resize if necessary
    save(game, row, col, direction, tiles);
    if (peek(game->save_stack)->resized) {
        if (direction == 'H') {
            int cols_to_add = col + length - game->cols;
            resize_horizontal(game, cols_to_add);
        }
        if (direction == 'V') {
            int rows_to_add = row + length - game->rows;
            resize_vertical(game, rows_to_add);
        }
    }
    for (int i = 0; i < length; i++) {
        int current_row = direction == 'H' ? row : row + i;
        int current_col = direction == 'H' ? col + i : col;

        if (tiles[i] != ' ') { 
            if (game->heights[current_row][current_col] >= 5) { 
                undo_place_tiles(game);
                *num_tiles_placed = 0;
                return game;
            }
            game->board[current_row][current_col] = toupper(tiles[i]);
            game->heights[current_row][current_col]++;
            placed++;
        } else {
            if (game->board[current_row][current_col] == '.' || game->board[current_row][current_col] == ' ') {
                undo_place_tiles(game);
                *num_tiles_placed = 0;
                return game;
            }
        }
    } 
    if (!check_word_validity(game, row, col, direction)) {
        undo_place_tiles(game);
        *num_tiles_placed = 0;
        return game;
    } 

    if (!check_bonus_words(game, row, col, direction, tiles)) {
        undo_place_tiles(game);
        *num_tiles_placed = 0;
        return game;
    }


    *num_tiles_placed = placed;
    return game;
}

GameState* undo_place_tiles(GameState *game) {
    Save* save = pop(game->save_stack);
    if (!save) {return game;}
    // check if last place_tiles resized, then resize (shrink)
    if (save->resized) {
        if (save->direction == 'V') {
            int rows_to_subtract = 0 - (game->rows - save->rows);
            for (int i = save->rows; i < game->rows; i++) {
                free(game->board[i]);
                free(game->heights[i]);
            }
            resize_vertical(game, rows_to_subtract);
        } else {
            int cols_to_subtract = 0 - (game->cols - save->cols);
            resize_horizontal(game, cols_to_subtract);            
        }
    }
    int length = save->length;
    // copy tiles and heights
    if (save->direction == 'V') {
        for (int i = 0; i < length; i++) {
            game->board[save->row + i][save->col] = save->tiles[i];
            game->heights[save->row + i][save->col] = save->heights[i];
        }
    } else {
        for (int i = 0; i < length; i++) {
            game->board[save->row][save->col + i] = save->tiles[i];
            game->heights[save->row][save->col + i] = save->heights[i];
        }
    }
    free(save->tiles);
    free(save->heights);
    free(save);
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
    free_word_array();
    num_valid_words = 0;

    // for (int i = 0; i < game->rows; i++) {
    //     fprintf(file, "%s\n", game->board[i]);
    // }
    // for (int i = 0; i < game->rows; i++) {
    //     for (int j = 0; j < game->cols; j++) {
    //         fprintf(file, "%d", game->heights[i][j]);
    //     }
    //     fprintf(file, "\n");
    }

